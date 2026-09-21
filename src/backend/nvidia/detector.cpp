#include "detector.h"

#include "system/capabilityprobe.h"
#include "system/commandrunner.h"
#include "system/sessionutil.h"
#include "system/systeminfoprovider.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QMutex>
#include <QRegularExpression>
#include <QTextStream>
#include <QtGlobal>

#include <optional>

namespace {

QString normalizedRpmDriverVersion(const QString &packageVersion) {
  QString version = packageVersion.trimmed();
  const int epochIndex = version.indexOf(QLatin1Char(':'));
  if (epochIndex >= 0) {
    version = version.mid(epochIndex + 1);
  }

  const int releaseIndex = version.indexOf(QLatin1Char('-'));
  if (releaseIndex > 0) {
    version = version.left(releaseIndex);
  }

  return version.trimmed();
}

QString overriddenSystemPath(const char *environmentVariable,
                             const QString &fallback) {
  const QString overridePath =
      qEnvironmentVariable(environmentVariable).trimmed();
  return overridePath.isEmpty() ? fallback : overridePath;
}

std::optional<bool> runningNvidiaModuleIsOpen() {
  QFile openRm(overriddenSystemPath(
      "RO_CONTROL_NVIDIA_OPENRM_PATH",
      QStringLiteral("/sys/module/nvidia/parameters/NVreg_OpenRm")));
  if (openRm.open(QIODevice::ReadOnly | QIODevice::Text)) {
    const QString value = QString::fromUtf8(openRm.readAll()).trimmed();
    if (value == QStringLiteral("1")) {
      return true;
    }
    if (value == QStringLiteral("0")) {
      return false;
    }
  }

  QFile procVersion(
      overriddenSystemPath("RO_CONTROL_NVIDIA_PROC_VERSION_PATH",
                           QStringLiteral("/proc/driver/nvidia/version")));
  if (!procVersion.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return std::nullopt;
  }

  const QString content = QString::fromUtf8(procVersion.readAll());
  if (content.contains(QStringLiteral("Open Kernel Module"),
                       Qt::CaseInsensitive)) {
    return true;
  }
  if (content.contains(QStringLiteral("NVRM version"), Qt::CaseInsensitive) ||
      content.contains(QStringLiteral("NVIDIA UNIX"), Qt::CaseInsensitive)) {
    return false;
  }
  return std::nullopt;
}

struct InstalledPackagesSnapshot {
  QSet<QString> installedPackages;
  QString detectedVersion;
};

InstalledPackagesSnapshot queryInstalledDriverPackages() {
  static InstalledPackagesSnapshot cached;
  static QElapsedTimer cacheTimer;
  static bool cacheValid = false;
  static QMutex cacheMutex;
  static QString cachedRpmPath;

  QMutexLocker locker(&cacheMutex);
  const QString rpmPath =
      CommandRunner::resolveProgramPath(QStringLiteral("rpm"));
  if (cacheValid && cachedRpmPath == rpmPath && cacheTimer.isValid() &&
      cacheTimer.elapsed() < 5000) {
    return cached;
  }

  cached.installedPackages.clear();
  cached.detectedVersion.clear();

  if (rpmPath.isEmpty()) {
    cacheValid = false;
    cachedRpmPath.clear();
    return cached;
  }

  CommandRunner runner;
  CommandRunner::RunOptions opts;
  opts.timeoutMs = 1200;
  const auto result = runner.run(
      QStringLiteral("rpm"),
      {QStringLiteral("-q"), QStringLiteral("--qf"),
       QStringLiteral("%{NAME}|%{EPOCH}:%{VERSION}-%{RELEASE}\n"),
       QStringLiteral("akmod-nvidia"), QStringLiteral("akmod-nvidia-open"),
       QStringLiteral("xorg-x11-drv-nvidia"),
       QStringLiteral("xorg-x11-drv-nvidia-open")},
      opts);

  if (result.success() || !result.stdout.isEmpty()) {
    const QStringList lines =
        result.stdout.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
      if (line.contains(QStringLiteral("is not installed"))) {
        continue;
      }
      const QStringList parts = line.split(QLatin1Char('|'));
      if (parts.size() >= 2) {
        const QString pkgName = parts.at(0).trimmed();
        const QString rawVer = parts.at(1).trimmed();
        cached.installedPackages.insert(pkgName);
        if (cached.detectedVersion.isEmpty()) {
          cached.detectedVersion = normalizedRpmDriverVersion(rawVer);
        }
      }
    }
  }

  cacheTimer.start();
  cachedRpmPath = rpmPath;
  cacheValid = true;
  return cached;
}

} // namespace

NvidiaDetector::NvidiaDetector(QObject *parent) : QObject(parent) {}

NvidiaDetector::GpuInfo NvidiaDetector::detect() const {
  GpuInfo info;

  info.displayAdapterName = detectDisplayAdapterName();
  info.name = detectGpuName();
  info.found = !info.name.isEmpty();
  info.driverVersion = detectDriverVersion();
  info.driverLoaded = isModuleLoaded(QStringLiteral("nvidia"));

  const auto pkgSnapshot = queryInstalledDriverPackages();
  if (info.driverVersion.isEmpty()) {
    info.driverVersion = pkgSnapshot.detectedVersion;
  }
  info.driverPackageInstalled = !pkgSnapshot.installedPackages.isEmpty();
  info.openSourceDriverInstalled =
      pkgSnapshot.installedPackages.contains(
          QStringLiteral("akmod-nvidia-open")) ||
      pkgSnapshot.installedPackages.contains(
          QStringLiteral("xorg-x11-drv-nvidia-open"));
  if (!info.openSourceDriverInstalled && info.driverLoaded) {
    const std::optional<bool> runningOpen = runningNvidiaModuleIsOpen();
    if (runningOpen.has_value()) {
      info.openSourceDriverInstalled = runningOpen.value();
    }
  }

  if (pkgSnapshot.installedPackages.contains(QStringLiteral("akmod-nvidia"))) {
    info.closedSourceDriverInstalled = true;
  } else if (info.driverLoaded) {
    const std::optional<bool> runningOpen = runningNvidiaModuleIsOpen();
    if (runningOpen.has_value()) {
      info.closedSourceDriverInstalled = !runningOpen.value();
    } else {
      info.closedSourceDriverInstalled = !info.openSourceDriverInstalled;
    }
  }

  info.openKernelModulesInstalled = info.openSourceDriverInstalled;
  info.secureBootEnabled = detectSecureBoot(&info.secureBootKnown);
  info.sessionType = SessionUtil::detectSessionType();

  return info;
}

bool NvidiaDetector::hasNvidiaGpu() const { return !detectGpuName().isEmpty(); }

QString NvidiaDetector::gpuName() const {
  return SystemInfoProvider::localizeGpuName(m_info.name);
}

QString NvidiaDetector::displayAdapterName() const {
  return SystemInfoProvider::localizeGpuName(m_info.displayAdapterName);
}

QString NvidiaDetector::localizeGpuName(const QString &rawName) {
  return SystemInfoProvider::localizeGpuName(rawName);
}

bool NvidiaDetector::isDriverInstalled() const {
  return !installedDriverVersion().isEmpty() || detectDriverPackageInstalled();
}

QString NvidiaDetector::installedDriverVersion() const {
  const QString activeVersion = detectDriverVersion();
  if (!activeVersion.isEmpty()) {
    return activeVersion;
  }
  return detectDriverPackageVersion();
}

QString NvidiaDetector::activeDriver() const {
  if (m_info.driverLoaded) {
    if (m_info.openKernelModulesInstalled) {
      return tr("NVIDIA Open Kernel Modules");
    }
    return tr("NVIDIA Driver");
  }
  if (m_info.driverPackageInstalled)
    return tr("Installed, Restart Required");
  return tr("Not Installed");
}

QString NvidiaDetector::installedDriverSource() const {
  if (m_info.closedSourceDriverInstalled && m_info.openSourceDriverInstalled) {
    return QStringLiteral("mixed");
  }
  if (m_info.closedSourceDriverInstalled) {
    return QStringLiteral("closed-source");
  }
  if (m_info.openSourceDriverInstalled) {
    return QStringLiteral("open-source");
  }
  return QStringLiteral("none");
}

QString NvidiaDetector::installedDriverSourceLabel() const {
  const QString source = installedDriverSource();
  if (source == QStringLiteral("closed-source")) {
    return tr("NVIDIA Proprietary Kernel Module detected");
  }
  if (source == QStringLiteral("open-source")) {
    return tr("NVIDIA Open Kernel Modules detected");
  }
  if (source == QStringLiteral("mixed")) {
    return tr("Mixed driver state detected");
  }
  return tr("No driver source detected");
}

QString NvidiaDetector::verificationReport() const {
  const QString gpuText = m_info.found ? m_info.name
                                       : (m_info.displayAdapterName.isEmpty()
                                              ? tr("Unavailable")
                                              : m_info.displayAdapterName);
  const QString versionText =
      m_info.driverVersion.isEmpty() ? tr("Unavailable") : m_info.driverVersion;

  return tr("GPU: %1\nDriver Version: %2\nSecure Boot: %3\nSession: %4\n"
            "Active Stack: %5")
      .arg(gpuText, versionText,
           m_info.secureBootKnown
               ? (m_info.secureBootEnabled ? tr("Enabled") : tr("Disabled"))
               : tr("Unknown"),
           m_info.sessionType.isEmpty() ? tr("Unknown") : m_info.sessionType,
           activeDriver());
}

void NvidiaDetector::refresh() {
  const GpuInfo newInfo = detect();
  if (m_info == newInfo) {
    return;
  }
  m_info = newInfo;
  emit infoChanged();
}

void NvidiaDetector::setDetectionResult(const GpuInfo &info) {
  if (m_info == info) {
    return;
  }
  m_info = info;
  emit infoChanged();
}

QString NvidiaDetector::cleanGpuName(const QString &rawName,
                                     const QString &vendor) {
  QString name = rawName.trimmed();
  if (name.isEmpty()) {
    return {};
  }

  // 1. If lspci bracket format like "TU106 [GeForce RTX 2060 SUPER]" or
  // "[GeForce RTX 3080]"
  static const QRegularExpression bracketRegex(
      QStringLiteral("\\[([^\\]]+)\\]"));
  const auto match = bracketRegex.match(name);
  if (match.hasMatch()) {
    name = match.captured(1).trimmed();
  }

  // 2. Strip revision suffixes like (rev a1), (rev 01), [rev a1], -ra1, etc.
  static const QRegularExpression revRegex(
      QStringLiteral(
          "\\s*\\(rev\\s+[a-f0-9]+\\)|\\s*\\[rev\\s+[a-f0-9]+\\]|\\s+-r[a-"
          "f0-9]+"),
      QRegularExpression::CaseInsensitiveOption);
  name.remove(revRegex);

  // 3. Clean up redundant vendor prefixes
  name.remove(QStringLiteral("NVIDIA Corporation "), Qt::CaseInsensitive);
  name.remove(QStringLiteral("NVIDIA Corp "), Qt::CaseInsensitive);
  name.remove(QStringLiteral("Intel Corporation "), Qt::CaseInsensitive);
  name.remove(QStringLiteral("Advanced Micro Devices, Inc. "),
              Qt::CaseInsensitive);
  name.remove(QStringLiteral("AMD/ATI "), Qt::CaseInsensitive);
  name = name.trimmed();

  // 4. Properly prefix with canonical vendor name
  const bool isNvidia =
      vendor.contains(QStringLiteral("NVIDIA"), Qt::CaseInsensitive) ||
      rawName.contains(QStringLiteral("NVIDIA"), Qt::CaseInsensitive) ||
      name.startsWith(QStringLiteral("GeForce"), Qt::CaseInsensitive) ||
      name.startsWith(QStringLiteral("RTX"), Qt::CaseInsensitive) ||
      name.startsWith(QStringLiteral("GTX"), Qt::CaseInsensitive) ||
      name.startsWith(QStringLiteral("Quadro"), Qt::CaseInsensitive) ||
      name.startsWith(QStringLiteral("Tesla"), Qt::CaseInsensitive) ||
      name.startsWith(QStringLiteral("Titan"), Qt::CaseInsensitive) ||
      name.startsWith(QStringLiteral("A100"), Qt::CaseInsensitive) ||
      name.startsWith(QStringLiteral("H100"), Qt::CaseInsensitive) ||
      name.startsWith(QStringLiteral("B200"), Qt::CaseInsensitive) ||
      name.startsWith(QStringLiteral("L40"), Qt::CaseInsensitive);

  if (isNvidia) {
    if (!name.startsWith(QStringLiteral("NVIDIA"), Qt::CaseInsensitive)) {
      name.prepend(QStringLiteral("NVIDIA "));
    }
  } else if (vendor.contains(QStringLiteral("Intel"), Qt::CaseInsensitive) ||
             rawName.contains(QStringLiteral("Intel"), Qt::CaseInsensitive)) {
    if (!name.startsWith(QStringLiteral("Intel"), Qt::CaseInsensitive)) {
      name.prepend(QStringLiteral("Intel "));
    }
  } else if (vendor.contains(QStringLiteral("AMD"), Qt::CaseInsensitive) ||
             vendor.contains(QStringLiteral("ATI"), Qt::CaseInsensitive) ||
             rawName.contains(QStringLiteral("Radeon"), Qt::CaseInsensitive)) {
    if (!name.startsWith(QStringLiteral("AMD"), Qt::CaseInsensitive)) {
      name.prepend(QStringLiteral("AMD "));
    }
  }

  // Normalize duplicate spaces
  static const QRegularExpression spacesRegex(QStringLiteral("\\s+"));
  name = name.replace(spacesRegex, QStringLiteral(" ")).trimmed();

  return name;
}

QString NvidiaDetector::detectGpuNameFromProc() {
  const QDir nvidiaGpusDir(QStringLiteral("/proc/driver/nvidia/gpus"));
  if (nvidiaGpusDir.exists()) {
    const QStringList gpuDirs =
        nvidiaGpusDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &dir : gpuDirs) {
      const QString infoPath =
          nvidiaGpusDir.filePath(dir) + QStringLiteral("/information");
      QFile file(infoPath);
      if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
          const QString line = in.readLine().trimmed();
          if (line.startsWith(QStringLiteral("Model:"), Qt::CaseInsensitive)) {
            const QString model = line.mid(6).trimmed();
            if (!model.isEmpty()) {
              return cleanGpuName(model, QStringLiteral("NVIDIA"));
            }
          }
        }
      }
    }
  }
  return {};
}

QString NvidiaDetector::detectDisplayAdapterName() const {
  if (!CapabilityProbe::isToolAvailable(QStringLiteral("lspci"))) {
    return {};
  }

  CommandRunner runner;

  const auto result =
      runner.run(QStringLiteral("lspci"), {QStringLiteral("-mm")});

  if (!result.success())
    return {};

  const QStringList lines = result.stdout.split(QLatin1Char('\n'));
  for (const QString &line : lines) {
    if (line.contains(QStringLiteral("VGA"), Qt::CaseInsensitive) ||
        line.contains(QStringLiteral("3D controller"), Qt::CaseInsensitive) ||
        line.contains(QStringLiteral("Display controller"),
                      Qt::CaseInsensitive)) {
      static const QRegularExpression re(QStringLiteral("\"([^\"]+)\""));
      auto it = re.globalMatch(line);
      QStringList parts;
      while (it.hasNext())
        parts << it.next().captured(1);

      if (parts.size() >= 3) {
        const QString vendor = parts.size() >= 2 ? parts[1] : QString();
        return cleanGpuName(parts[2], vendor);
      }
    }
  }

  return {};
}

QString NvidiaDetector::detectGpuName() const {
  // 1. Check direct Linux /proc/driver/nvidia/gpus/*/information (instant &
  // kernel-backed)
  const QString procName = detectGpuNameFromProc();
  if (!procName.isEmpty()) {
    return procName;
  }

  CommandRunner runner;

  // 2. Query nvidia-smi tool if available
  if (CapabilityProbe::isToolAvailable(QStringLiteral("nvidia-smi"))) {
    const auto result = runner.run(QStringLiteral("nvidia-smi"),
                                   {QStringLiteral("--query-gpu=name"),
                                    QStringLiteral("--format=csv,noheader")});
    if (result.success() && !result.stdout.trimmed().isEmpty()) {
      const QString name =
          result.stdout.split(QLatin1Char('\n')).value(0).trimmed();
      if (!name.isEmpty()) {
        return cleanGpuName(name, QStringLiteral("NVIDIA"));
      }
    }
  }

  // 3. Fallback to lspci
  if (CapabilityProbe::isToolAvailable(QStringLiteral("lspci"))) {
    const auto result =
        runner.run(QStringLiteral("lspci"), {QStringLiteral("-mm")});
    if (result.success()) {
      const QStringList lines = result.stdout.split(QLatin1Char('\n'));
      for (const QString &line : lines) {
        if (line.contains(QStringLiteral("NVIDIA"), Qt::CaseInsensitive) &&
            (line.contains(QStringLiteral("VGA"), Qt::CaseInsensitive) ||
             line.contains(QStringLiteral("3D controller"),
                           Qt::CaseInsensitive) ||
             line.contains(QStringLiteral("Display controller"),
                           Qt::CaseInsensitive))) {
          static const QRegularExpression re(QStringLiteral("\"([^\"]+)\""));
          auto it = re.globalMatch(line);
          QStringList parts;
          while (it.hasNext())
            parts << it.next().captured(1);

          if (parts.size() >= 3) {
            const QString vendor =
                parts.size() >= 2 ? parts[1] : QStringLiteral("NVIDIA");
            return cleanGpuName(parts[2], vendor);
          }
        }
      }
    }
  }

  return {};
}

QString NvidiaDetector::detectDriverVersion() const {
  // 1. Direct sysfs module version
  QFile sysModuleVersion(QStringLiteral("/sys/module/nvidia/version"));
  if (sysModuleVersion.open(QIODevice::ReadOnly | QIODevice::Text)) {
    const QString ver = QString::fromUtf8(sysModuleVersion.readAll()).trimmed();
    if (!ver.isEmpty()) {
      return ver;
    }
  }

  // 2. Direct proc driver nvidia version
  QFile procVersion(QStringLiteral("/proc/driver/nvidia/version"));
  if (procVersion.open(QIODevice::ReadOnly | QIODevice::Text)) {
    const QString content = QString::fromUtf8(procVersion.readAll());
    static const QRegularExpression re(
        QStringLiteral(R"(NVRM\s+version:.*?(\d+\.\d+(?:\.\d+)?))"),
        QRegularExpression::CaseInsensitiveOption);
    const auto match = re.match(content);
    if (match.hasMatch()) {
      return match.captured(1).trimmed();
    }
  }

  CommandRunner runner;

  if (CapabilityProbe::isToolAvailable(QStringLiteral("nvidia-smi"))) {
    const auto result =
        runner.run(QStringLiteral("nvidia-smi"),
                   {QStringLiteral("--query-gpu=driver_version"),
                    QStringLiteral("--format=csv,noheader")});

    if (result.success() && !result.stdout.trimmed().isEmpty())
      return result.stdout.trimmed();
  }

  if (!CapabilityProbe::isToolAvailable(QStringLiteral("modinfo"))) {
    return {};
  }

  const auto modinfo =
      runner.run(QStringLiteral("modinfo"), {QStringLiteral("nvidia")});

  if (modinfo.success()) {
    static const QRegularExpression re(QStringLiteral("^version:\\s+(.+)$"),
                                       QRegularExpression::MultilineOption);
    const auto match = re.match(modinfo.stdout);
    if (match.hasMatch())
      return match.captured(1).trimmed();
  }

  return {};
}

QString NvidiaDetector::detectDriverPackageVersion() const {
  const auto snapshot = queryInstalledDriverPackages();
  return snapshot.detectedVersion;
}

bool NvidiaDetector::detectDriverPackageInstalled() const {
  const auto snapshot = queryInstalledDriverPackages();
  return !snapshot.installedPackages.isEmpty();
}

bool NvidiaDetector::detectClosedSourceDriverInstalled() const {
  const auto snapshot = queryInstalledDriverPackages();
  // xorg-x11-drv-nvidia is shared by both kernel-module variants on Fedora.
  // Only the akmod package (or the running module) identifies the source.
  if (snapshot.installedPackages.contains(QStringLiteral("akmod-nvidia"))) {
    return true;
  }

  if (isModuleLoaded(QStringLiteral("nvidia"))) {
    const std::optional<bool> runningOpen = runningNvidiaModuleIsOpen();
    if (runningOpen.has_value()) {
      return !runningOpen.value();
    }
  }

  // Nvidia module loaded but no package markers found — check that no
  // open-source package is present before claiming closed-source.
  const bool openPackageInstalled =
      snapshot.installedPackages.contains(
          QStringLiteral("akmod-nvidia-open")) ||
      snapshot.installedPackages.contains(
          QStringLiteral("xorg-x11-drv-nvidia-open"));
  return isModuleLoaded(QStringLiteral("nvidia")) && !openPackageInstalled;
}

bool NvidiaDetector::detectOpenSourceDriverInstalled() const {
  const auto snapshot = queryInstalledDriverPackages();
  if (snapshot.installedPackages.contains(
          QStringLiteral("akmod-nvidia-open")) ||
      snapshot.installedPackages.contains(
          QStringLiteral("xorg-x11-drv-nvidia-open"))) {
    return true;
  }

  if (isModuleLoaded(QStringLiteral("nvidia"))) {
    const std::optional<bool> runningOpen = runningNvidiaModuleIsOpen();
    if (runningOpen.has_value()) {
      return runningOpen.value();
    }
  }

  return false;
}

bool NvidiaDetector::isPackageInstalled(const QString &packageName) const {
  if (!CapabilityProbe::isToolAvailable(QStringLiteral("rpm"))) {
    return false;
  }

  CommandRunner runner;
  const auto result =
      runner.run(QStringLiteral("rpm"), {QStringLiteral("-q"), packageName});
  return result.success();
}

bool NvidiaDetector::isModuleLoaded(const QString &moduleName) const {
  QFile modules(overriddenSystemPath("RO_CONTROL_PROC_MODULES_PATH",
                                     QStringLiteral("/proc/modules")));
  if (!modules.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  QTextStream stream(&modules);
  while (!stream.atEnd()) {
    const QString line = stream.readLine();
    if (line.startsWith(moduleName + QLatin1Char(' ')))
      return true;
  }

  return false;
}

bool NvidiaDetector::detectSecureBoot(bool *known) const {
  bool enabled = false;
  bool efivarsKnown = false;
  if (detectSecureBootFromEfivars(&enabled, &efivarsKnown)) {
    if (known != nullptr) {
      *known = efivarsKnown;
    }
    return enabled;
  }

  CommandRunner runner;

  if (CapabilityProbe::isToolAvailable(QStringLiteral("mokutil"))) {
    const auto result =
        runner.run(QStringLiteral("mokutil"), {QStringLiteral("--sb-state")});
    // mokutil outputs: "SecureBoot enabled\n" or "SecureBoot disabled\n"
    // After toLower(): "secureboot enabled" or "secureboot disabled"
    const QString combined =
        (result.stdout + QLatin1Char(' ') + result.stderr).toLower().trimmed();

    if (combined.contains(QStringLiteral("secureboot enabled"))) {
      if (known != nullptr) {
        *known = true;
      }
      return true;
    }
    if (combined.contains(QStringLiteral("secureboot disabled"))) {
      if (known != nullptr) {
        *known = true;
      }
      return false;
    }
    // mokutil present but output unrecognized — do not mark as known
  }

  if (CapabilityProbe::isToolAvailable(QStringLiteral("bootctl"))) {
    const auto result =
        runner.run(QStringLiteral("bootctl"), {QStringLiteral("status")});
    if (result.success()) {
      static const QRegularExpression sbRegex(
          QStringLiteral(R"(Secure\s*Boot:\s*(enabled|disabled))"),
          QRegularExpression::CaseInsensitiveOption);
      const auto match = sbRegex.match(result.stdout);
      if (match.hasMatch()) {
        if (known != nullptr) {
          *known = true;
        }
        return match.captured(1).compare(QStringLiteral("enabled"),
                                         Qt::CaseInsensitive) == 0;
      }
    }
  }

  if (known != nullptr) {
    *known = false;
  }

  return false;
}

bool NvidiaDetector::detectSecureBootFromEfivars(bool *enabled,
                                                 bool *known) const {
  if (enabled == nullptr || known == nullptr) {
    return false;
  }

  const QString overridePath =
      qEnvironmentVariable("RO_CONTROL_SECURE_BOOT_EFIVAR_PATH").trimmed();
  QString secureBootPath = overridePath;
  if (secureBootPath.isEmpty()) {
    QDir efivarsDir(QStringLiteral("/sys/firmware/efi/efivars"));
    const QStringList entries = efivarsDir.entryList(
        {QStringLiteral("SecureBoot-*")}, QDir::Files, QDir::Name);
    if (!entries.isEmpty()) {
      secureBootPath = efivarsDir.filePath(entries.constFirst());
    }
  }

  if (!secureBootPath.isEmpty()) {
    QFile file(secureBootPath);
    if (file.open(QIODevice::ReadOnly)) {
      const QByteArray raw = file.readAll();
      if (raw.size() >= 5) {
        *enabled = raw.at(4) != 0;
        *known = true;
        return true;
      }
    }
  }

  // Legacy sysfs efivars (/sys/firmware/efi/vars/SecureBoot/data)
  QFile legacyFile(QStringLiteral("/sys/firmware/efi/vars/SecureBoot/data"));
  if (legacyFile.open(QIODevice::ReadOnly)) {
    const QByteArray raw = legacyFile.readAll();
    if (!raw.isEmpty()) {
      *enabled = raw.at(0) != 0;
      *known = true;
      return true;
    }
  }

  return false;
}
