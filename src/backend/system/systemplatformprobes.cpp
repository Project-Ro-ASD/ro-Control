#include "systemplatformprobes.h"

#include "capabilityprobe.h"
#include "commandrunner.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QRegularExpression>
#include <QSysInfo>
#include <QTextStream>

#if defined(Q_OS_UNIX)
#include <sys/utsname.h>
#endif

namespace {

QString readTextFile(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }
  return QString::fromUtf8(file.readAll()).trimmed();
}

QString valueFromOsRelease(const QString &key) {
  QFile file(QStringLiteral("/etc/os-release"));
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }

  QTextStream stream(&file);
  while (!stream.atEnd()) {
    const QString line = stream.readLine().trimmed();
    if (!line.startsWith(key + QLatin1Char('='))) {
      continue;
    }

    QString value = line.mid(key.size() + 1).trimmed();
    if (value.startsWith(QLatin1Char('"')) &&
        value.endsWith(QLatin1Char('"')) && value.size() >= 2) {
      value = value.mid(1, value.size() - 2);
    }
    return value;
  }

  return {};
}

#if defined(Q_OS_LINUX)
QString virtualizationLabel(const QString &virtualization) {
  const QString normalized = virtualization.trimmed().toLower();
  if (normalized.isEmpty() || normalized == QStringLiteral("none")) {
    return {};
  }
  if (normalized == QStringLiteral("kvm")) {
    return QStringLiteral("KVM");
  }
  if (normalized == QStringLiteral("qemu")) {
    return QStringLiteral("QEMU");
  }
  if (normalized == QStringLiteral("vmware")) {
    return QStringLiteral("VMware");
  }
  if (normalized == QStringLiteral("oracle") ||
      normalized == QStringLiteral("virtualbox")) {
    return QStringLiteral("VirtualBox");
  }
  if (normalized == QStringLiteral("microsoft") ||
      normalized == QStringLiteral("hyperv")) {
    return QStringLiteral("Hyper-V");
  }

  QString label = virtualization.trimmed();
  if (label.isEmpty()) {
    return {};
  }
  label[0] = label[0].toUpper();
  return label;
}
#endif

QString simplifiedDesktopName(const QString &desktop) {
  const QString trimmed = desktop.trimmed();
  if (trimmed.compare(QStringLiteral("KDE"), Qt::CaseInsensitive) == 0 ||
      trimmed.compare(QStringLiteral("KDE Plasma"), Qt::CaseInsensitive) == 0 ||
      trimmed.compare(QStringLiteral("Plasma"), Qt::CaseInsensitive) == 0) {
    return QStringLiteral("KDE Plasma");
  }

  if (trimmed.compare(QStringLiteral("GNOME"), Qt::CaseInsensitive) == 0) {
    return QStringLiteral("GNOME");
  }

  return trimmed;
}

QString formatMemoryBytes(qint64 bytes) {
  if (bytes <= 0) {
    return {};
  }
  const double mib = static_cast<double>(bytes) / (1024.0 * 1024.0);
  if (mib >= 1024.0) {
    return QStringLiteral("%1 GB (%2 MiB)")
        .arg(mib / 1024.0, 0, 'f', 1)
        .arg(static_cast<qint64>(mib));
  }
  return QStringLiteral("%1 MiB").arg(static_cast<qint64>(mib));
}

bool isIntegratedGpuDescription(const QString &vendor, const QString &model) {
  const QString normalizedVendor = vendor.toLower();
  const QString normalizedModel = model.toLower();
  if (normalizedVendor.contains(QStringLiteral("intel"))) {
    return !normalizedModel.contains(QStringLiteral("arc"));
  }
  return (normalizedVendor.contains(QStringLiteral("amd")) ||
          normalizedVendor.contains(QStringLiteral("ati"))) &&
         (normalizedModel.contains(QStringLiteral("radeon graphics")) ||
          normalizedModel.contains(QStringLiteral("vega")) ||
          normalizedModel.contains(QStringLiteral("integrated")));
}

QString normalizedGpuDisplayName(const QString &vendor, const QString &model) {
  QString name = model.trimmed();
  if (name.isEmpty()) {
    return {};
  }
  if (vendor.contains(QStringLiteral("Intel"), Qt::CaseInsensitive) &&
      !name.startsWith(QStringLiteral("Intel"), Qt::CaseInsensitive)) {
    name.prepend(QStringLiteral("Intel "));
  } else if ((vendor.contains(QStringLiteral("AMD"), Qt::CaseInsensitive) ||
              vendor.contains(QStringLiteral("ATI"), Qt::CaseInsensitive)) &&
             !name.startsWith(QStringLiteral("AMD"), Qt::CaseInsensitive)) {
    name.prepend(QStringLiteral("AMD "));
  }
  return name;
}

} // namespace

PlatformStaticInfo SystemPlatformProbes::probeStaticInfo() {
  PlatformStaticInfo info;
  info.osName = detectOsName();
  info.desktopEnvironment = detectDesktopEnvironment();
  info.kernelVersion = detectKernelVersion();
  info.virtualizationType = detectVirtualizationType();
  info.cpuModel = detectCpuModel();
  info.motherboardModel = detectMotherboardModel(info.virtualizationType);
  info.biosVersion = detectBiosVersion();
  info.cudaVersion = detectCudaVersion();
  info.graphicsApiSummary = detectGraphicsApiSummary();
  info.deviceType = detectDeviceType(info.virtualizationType);
  info.integratedGpuName = detectIntegratedGpuName();
  info.integratedGpuMemory = detectIntegratedGpuMemory(info.integratedGpuName);
  return info;
}

QString SystemPlatformProbes::detectOsName() {
  const QString prettyName = valueFromOsRelease(QStringLiteral("PRETTY_NAME"));
  if (!prettyName.isEmpty()) {
    return prettyName;
  }

  const QString productName = QSysInfo::prettyProductName();
  if (!productName.isEmpty()) {
    return productName;
  }

  return QSysInfo::productType();
}

QString SystemPlatformProbes::detectDesktopEnvironment() {
  QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP").trimmed();
  if (desktop.isEmpty()) {
    desktop = qEnvironmentVariable("DESKTOP_SESSION").trimmed();
  }

  if (desktop.isEmpty()) {
    return {};
  }

  desktop.replace(QLatin1Char(':'), QLatin1String(" / "));
  const QStringList parts = desktop.split(QLatin1Char('/'), Qt::SkipEmptyParts);
  QStringList normalizedParts;
  for (const QString &part : parts) {
    const QString trimmed = simplifiedDesktopName(part);
    if (trimmed.isEmpty()) {
      continue;
    }

    if (!normalizedParts.contains(trimmed, Qt::CaseInsensitive)) {
      normalizedParts << trimmed;
    }
  }

  return normalizedParts.join(QStringLiteral(" / "));
}

QString SystemPlatformProbes::detectKernelVersion() {
#if defined(Q_OS_UNIX)
  utsname name{};
  if (uname(&name) == 0) {
    return QString::fromLocal8Bit(name.release);
  }
#endif
  return QSysInfo::kernelVersion();
}

QString SystemPlatformProbes::detectCpuModel() {
#if defined(Q_OS_LINUX)
  // Fast path: read /proc/cpuinfo directly from memory without process fork
  QFile cpuInfo(QStringLiteral("/proc/cpuinfo"));
  if (cpuInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream stream(&cpuInfo);
    while (!stream.atEnd()) {
      const QString line = stream.readLine();
      if (line.startsWith(QStringLiteral("model name"), Qt::CaseInsensitive) ||
          line.startsWith(QStringLiteral("Hardware"), Qt::CaseInsensitive) ||
          line.startsWith(QStringLiteral("Processor"), Qt::CaseInsensitive)) {
        const int separatorIndex = line.indexOf(QLatin1Char(':'));
        if (separatorIndex >= 0) {
          const QString value = line.mid(separatorIndex + 1).trimmed();
          if (!value.isEmpty() &&
              value.compare(QSysInfo::currentCpuArchitecture(),
                            Qt::CaseInsensitive) != 0) {
            return value;
          }
        }
      }
    }
  }

  // Fallback to lscpu only if /proc/cpuinfo had no readable model name
  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 1000;
  const auto lscpuResult = runner.run(QStringLiteral("lscpu"), {}, options);
  if (lscpuResult.success()) {
    const QStringList lines =
        lscpuResult.stdout.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
      if (line.startsWith(QStringLiteral("Model name:")) ||
          line.startsWith(QStringLiteral("Hardware:")) ||
          line.startsWith(QStringLiteral("Processor:"))) {
        const int separatorIndex = line.indexOf(QLatin1Char(':'));
        if (separatorIndex >= 0) {
          const QString value = line.mid(separatorIndex + 1).trimmed();
          if (!value.isEmpty() &&
              value.compare(QSysInfo::currentCpuArchitecture(),
                            Qt::CaseInsensitive) != 0) {
            return value;
          }
        }
      }
    }
  }

#elif defined(Q_OS_MACOS)
  CommandRunner runner;
  const auto result = runner.run(
      QStringLiteral("sysctl"),
      {QStringLiteral("-n"), QStringLiteral("machdep.cpu.brand_string")});
  if (result.success()) {
    const QString value = result.stdout.trimmed();
    if (!value.isEmpty()) {
      return value;
    }
  }
#endif

  const QString architecture = QSysInfo::currentCpuArchitecture();
  const QString virtualizationType = detectVirtualizationType();
  if (!virtualizationType.isEmpty()) {
    return architecture.isEmpty()
               ? QStringLiteral("%1 Virtual CPU").arg(virtualizationType)
               : QStringLiteral("%1 Virtual CPU (%2)")
                     .arg(virtualizationType, architecture);
  }
  return architecture.isEmpty() ? QStringLiteral("Unknown CPU")
                                : QStringLiteral("CPU (%1)").arg(architecture);
}

QString SystemPlatformProbes::detectMotherboardModel(
    const QString &virtualizationType) {
#if defined(Q_OS_LINUX)
  const QString vendor =
      readTextFile(QStringLiteral("/sys/class/dmi/id/board_vendor")).trimmed();
  const QString name =
      readTextFile(QStringLiteral("/sys/class/dmi/id/board_name")).trimmed();
  if (!name.isEmpty()) {
    if (!vendor.isEmpty() && !name.startsWith(vendor, Qt::CaseInsensitive)) {
      return vendor + QLatin1Char(' ') + name;
    }
    return name;
  }

  const QString sysVendor =
      readTextFile(QStringLiteral("/sys/class/dmi/id/sys_vendor")).trimmed();
  const QString prodName =
      readTextFile(QStringLiteral("/sys/class/dmi/id/product_name")).trimmed();
  if (!prodName.isEmpty()) {
    if (!sysVendor.isEmpty() &&
        !prodName.startsWith(sysVendor, Qt::CaseInsensitive)) {
      return sysVendor + QLatin1Char(' ') + prodName;
    }
    return prodName;
  }
#endif
  const QString virt = !virtualizationType.isEmpty()
                           ? virtualizationType
                           : detectVirtualizationType();
  if (!virt.isEmpty()) {
    return QStringLiteral("%1 Virtual Motherboard").arg(virt);
  }
  return {};
}

QString SystemPlatformProbes::detectBiosVersion() {
#if defined(Q_OS_LINUX)
  const QString version =
      readTextFile(QStringLiteral("/sys/class/dmi/id/bios_version")).trimmed();
  const QString date =
      readTextFile(QStringLiteral("/sys/class/dmi/id/bios_date")).trimmed();
  if (!version.isEmpty()) {
    return !date.isEmpty() ? QStringLiteral("%1 (%2)").arg(version, date)
                           : version;
  }
#endif
  return {};
}

QString SystemPlatformProbes::detectCudaVersion() {
#if defined(Q_OS_LINUX)
  if (CapabilityProbe::isToolAvailable(QStringLiteral("nvidia-smi"))) {
    CommandRunner runner;
    CommandRunner::RunOptions options;
    options.timeoutMs = 1500;
    const auto smiResult =
        runner.run(QStringLiteral("nvidia-smi"), {}, options);
    if (smiResult.success()) {
      static const QRegularExpression cudaRegex(
          QStringLiteral(R"(CUDA Version:\s*([0-9]+\.[0-9]+))"));
      const auto match = cudaRegex.match(smiResult.stdout);
      if (match.hasMatch()) {
        return QStringLiteral("CUDA %1").arg(match.captured(1).trimmed());
      }
    }
  }

  QFile verJson(QStringLiteral("/usr/local/cuda/version.json"));
  if (verJson.open(QIODevice::ReadOnly | QIODevice::Text)) {
    const QString content = QString::fromUtf8(verJson.readAll());
    static const QRegularExpression jsonRegex(QStringLiteral(
        R"("cuda"\s*:\s*\{\s*"version"\s*:\s*"([0-9]+\.[0-9]+))"));
    const auto m = jsonRegex.match(content);
    if (m.hasMatch()) {
      return QStringLiteral("CUDA %1").arg(m.captured(1).trimmed());
    }
  }
#endif
  return {};
}

QString SystemPlatformProbes::detectGraphicsApiSummary() {
  QStringList capabilities;
  const QString cuda = detectCudaVersion();
  if (!cuda.isEmpty()) {
    capabilities << cuda;
  }

#if defined(Q_OS_LINUX)
  // Fast path: inspect Vulkan ICD manifests without spawning vulkaninfo
  static const QStringList icdDirs = {QStringLiteral("/usr/share/vulkan/icd.d"),
                                      QStringLiteral("/etc/vulkan/icd.d")};
  bool hasVulkan = false;
  for (const QString &dirPath : icdDirs) {
    QDir dir(dirPath);
    if (dir.exists() &&
        !dir.entryList({QStringLiteral("*.json")}, QDir::Files).isEmpty()) {
      hasVulkan = true;
      break;
    }
  }

  if (!hasVulkan &&
      CapabilityProbe::isToolAvailable(QStringLiteral("vulkaninfo"))) {
    CommandRunner runner;
    CommandRunner::RunOptions options;
    options.timeoutMs = 500;
    const auto vulkan = runner.run(QStringLiteral("vulkaninfo"),
                                   {QStringLiteral("--summary")}, options);
    if (vulkan.success()) {
      hasVulkan = true;
    }
  }

  if (hasVulkan) {
    capabilities << QStringLiteral("Vulkan");
  }
#endif
  return capabilities.join(QStringLiteral(" • "));
}

QString SystemPlatformProbes::detectVirtualizationType() {
#if defined(Q_OS_LINUX)
  // Fast path: Check DMI tables for well-known hypervisor signatures
  const QString sysVendor =
      readTextFile(QStringLiteral("/sys/class/dmi/id/sys_vendor")).toLower();
  const QString prodName =
      readTextFile(QStringLiteral("/sys/class/dmi/id/product_name")).toLower();
  const QString biosVendor =
      readTextFile(QStringLiteral("/sys/class/dmi/id/bios_vendor")).toLower();

  auto matchDmi = [](const QString &s) -> QString {
    if (s.contains(QStringLiteral("qemu")))
      return QStringLiteral("QEMU");
    if (s.contains(QStringLiteral("kvm")))
      return QStringLiteral("KVM");
    if (s.contains(QStringLiteral("vmware")))
      return QStringLiteral("VMware");
    if (s.contains(QStringLiteral("virtualbox")) ||
        s.contains(QStringLiteral("innotek")))
      return QStringLiteral("VirtualBox");
    if (s.contains(QStringLiteral("hyper-v")) ||
        s.contains(QStringLiteral("microsoft corporation virtual machine")))
      return QStringLiteral("Hyper-V");
    if (s.contains(QStringLiteral("bochs")))
      return QStringLiteral("Bochs");
    if (s.contains(QStringLiteral("xen")))
      return QStringLiteral("Xen");
    return {};
  };

  QString virt = matchDmi(sysVendor);
  if (virt.isEmpty())
    virt = matchDmi(prodName);
  if (virt.isEmpty())
    virt = matchDmi(biosVendor);
  if (!virt.isEmpty()) {
    return virt;
  }

  const QString hypervisor =
      readTextFile(QStringLiteral("/sys/hypervisor/type"));
  if (!hypervisor.isEmpty()) {
    return virtualizationLabel(hypervisor);
  }

  const QString osRelVirt =
      valueFromOsRelease(QStringLiteral("VIRTUALIZATION"));
  if (!osRelVirt.isEmpty()) {
    return virtualizationLabel(osRelVirt);
  }

  // Fallback to systemd-detect-virt with a tight timeout
  if (CapabilityProbe::isToolAvailable(QStringLiteral("systemd-detect-virt"))) {
    CommandRunner runner;
    CommandRunner::RunOptions options;
    options.timeoutMs = 500;
    const auto virtResult =
        runner.run(QStringLiteral("systemd-detect-virt"), {}, options);
    if (virtResult.success()) {
      const QString output = virtResult.stdout.trimmed();
      if (!output.isEmpty() && output != QStringLiteral("none")) {
        return virtualizationLabel(output);
      }
    }
  }
#endif
  return {};
}

QString
SystemPlatformProbes::detectDeviceType(const QString &virtualizationType) {
  const QString virt = !virtualizationType.isEmpty()
                           ? virtualizationType
                           : detectVirtualizationType();
  if (!virt.isEmpty()) {
    if (virt.compare(QStringLiteral("QEMU"), Qt::CaseInsensitive) == 0 ||
        virt.compare(QStringLiteral("KVM"), Qt::CaseInsensitive) == 0) {
      return QStringLiteral("QEMU");
    }
    return virt;
  }

#if defined(Q_OS_LINUX)
  const QString chassisType =
      readTextFile(QStringLiteral("/sys/class/dmi/id/chassis_type"));
  bool ok = false;
  const int chassis = chassisType.toInt(&ok);
  if (ok) {
    static const QList<int> laptopChassisTypes = {8, 9, 10, 14, 30, 31, 32};
    if (laptopChassisTypes.contains(chassis)) {
      return QStringLiteral("Laptop");
    }

    static const QList<int> desktopChassisTypes = {
        3, 4, 5, 6, 7, 15, 16, 35, 36,
    };
    if (desktopChassisTypes.contains(chassis)) {
      return QStringLiteral("Desktop");
    }
  }

  const QString chassisName =
      readTextFile(QStringLiteral("/sys/class/dmi/id/chassis_vendor")) +
      QLatin1Char(' ') +
      readTextFile(QStringLiteral("/sys/class/dmi/id/product_name"));
  if (chassisName.contains(QStringLiteral("laptop"), Qt::CaseInsensitive) ||
      chassisName.contains(QStringLiteral("notebook"), Qt::CaseInsensitive)) {
    return QStringLiteral("Laptop");
  }
#endif

  return {};
}

QString SystemPlatformProbes::detectIntegratedGpuName() {
#if defined(Q_OS_LINUX)
  // Fast path: inspect /sys/bus/pci/devices directly. If no Intel (0x8086) or
  // AMD (0x1002) display controller exists, skip running lspci entirely.
  const QDir pciBus(QStringLiteral("/sys/bus/pci/devices"));
  bool hasCandidateIntegrated = false;
  const auto entries =
      pciBus.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const auto &dev : entries) {
    const QString classStr = readTextFile(dev.absoluteFilePath() + "/class");
    if (classStr.startsWith(QStringLiteral("0x03"), Qt::CaseInsensitive)) {
      const QString vendor = readTextFile(dev.absoluteFilePath() + "/vendor");
      if (vendor == QStringLiteral("0x8086") ||
          vendor == QStringLiteral("0x1002")) {
        hasCandidateIntegrated = true;
        break;
      }
    }
  }

  if (!hasCandidateIntegrated) {
    return {};
  }

  if (!CapabilityProbe::isToolAvailable(QStringLiteral("lspci"))) {
    return {};
  }

  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 1200;
  const auto result =
      runner.run(QStringLiteral("lspci"), {QStringLiteral("-mm")}, options);
  if (!result.success()) {
    return {};
  }

  static const QRegularExpression fieldPattern(QStringLiteral("\"([^\"]+)\""));
  for (const QString &line :
       result.stdout.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
    if (!line.contains(QStringLiteral("VGA"), Qt::CaseInsensitive) &&
        !line.contains(QStringLiteral("3D controller"), Qt::CaseInsensitive) &&
        !line.contains(QStringLiteral("Display controller"),
                       Qt::CaseInsensitive)) {
      continue;
    }
    auto matches = fieldPattern.globalMatch(line);
    QStringList fields;
    while (matches.hasNext()) {
      fields << matches.next().captured(1);
    }
    if (fields.size() >= 3 &&
        isIntegratedGpuDescription(fields.at(1), fields.at(2))) {
      return normalizedGpuDisplayName(fields.at(1), fields.at(2));
    }
  }
#endif
  return {};
}

QString SystemPlatformProbes::detectIntegratedGpuMemory(
    const QString &integratedGpuName) {
#if defined(Q_OS_LINUX)
  if (integratedGpuName.isEmpty()) {
    return {};
  }
  const QDir drmRoot(QStringLiteral("/sys/class/drm"));
  const QFileInfoList cards =
      drmRoot.entryInfoList({QStringLiteral("card[0-9]*")},
                            QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const QFileInfo &card : cards) {
    const QString devicePath =
        card.absoluteFilePath() + QStringLiteral("/device");
    const QString vendor = readTextFile(devicePath + QStringLiteral("/vendor"));
    if (vendor != QStringLiteral("0x8086") &&
        vendor != QStringLiteral("0x1002")) {
      continue;
    }
    bool ok = false;
    const qint64 bytes =
        readTextFile(devicePath + QStringLiteral("/mem_info_vram_total"))
            .toLongLong(&ok);
    if (ok && bytes > 0) {
      return formatMemoryBytes(bytes);
    }
  }
#else
  Q_UNUSED(integratedGpuName);
#endif
  return {};
}

bool SystemPlatformProbes::onBattery(QString *sourceLabel) {
  const QString overrideOnline =
      qEnvironmentVariable("RO_CONTROL_POWER_SUPPLY_ONLINE").trimmed();
  if (!overrideOnline.isEmpty()) {
    const bool onBattery = overrideOnline == QStringLiteral("0");
    if (sourceLabel) {
      *sourceLabel =
          onBattery ? QStringLiteral("Battery") : QStringLiteral("AC Power");
    }
    return onBattery;
  }
#if defined(Q_OS_LINUX)
  QDir powerDir(QStringLiteral("/sys/class/power_supply"));
  bool hasBattery = false;
  bool discharging = false;
  bool acOnline = false;
  for (const QFileInfo &entry :
       powerDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
    const QString type = readTextFile(entry.absoluteFilePath() + "/type");
    if (type.compare(QStringLiteral("Battery"), Qt::CaseInsensitive) == 0) {
      hasBattery = true;
      discharging =
          discharging || readTextFile(entry.absoluteFilePath() + "/status")
                                 .compare(QStringLiteral("Discharging"),
                                          Qt::CaseInsensitive) == 0;
    } else if (type.compare(QStringLiteral("Mains"), Qt::CaseInsensitive) ==
               0) {
      acOnline =
          acOnline || readTextFile(entry.absoluteFilePath() + "/online") == "1";
    }
  }
  const bool onBattery = hasBattery && (discharging || !acOnline);
  if (sourceLabel) {
    *sourceLabel = hasBattery ? (onBattery ? QStringLiteral("Battery")
                                           : QStringLiteral("AC Power"))
                              : QStringLiteral("AC / Desktop");
  }
  return onBattery;
#else
  if (sourceLabel) {
    *sourceLabel = QStringLiteral("AC Power");
  }
  return false;
#endif
}
