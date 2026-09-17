#include "systeminfoprovider.h"

#include "commandrunner.h"

#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QSysInfo>
#include <QTextStream>

#if defined(Q_OS_UNIX)
#include <sys/utsname.h>
#endif

namespace {

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
  if (normalized == QStringLiteral("oracle")) {
    return QStringLiteral("VirtualBox");
  }
  if (normalized == QStringLiteral("microsoft")) {
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

[[maybe_unused]] QString valueFromFile(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }

  return QString::fromUtf8(file.readAll()).trimmed();
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

QString SystemInfoProvider::localizeGpuName(const QString &rawName) {
  if (rawName.trimmed().isEmpty()) {
    return {};
  }
  QString result = rawName.trimmed();

  // Multi-word phrases first to prevent partial replacements
  static const QRegularExpression reIntegratedGpuCtrl(
      QStringLiteral("(?i)(Integrated Graphics Controller|Dahili Grafik "
                     "Denetleyicisi|Integrierter "
                     "Grafikcontroller|Controlador de gr[áa]ficos integrado)"));
  result.replace(reIntegratedGpuCtrl,
                 QCoreApplication::translate("GpuNames",
                                             "Integrated Graphics Controller"));

  static const QRegularExpression reIntegratedDispCtrl(QStringLiteral(
      "(?i)(Integrated Display Controller|Dahili Ekran "
      "Denetleyicisi|Integrierter Display-Controller|Controlador "
      "de pantalla integrado)"));
  result.replace(
      reIntegratedDispCtrl,
      QCoreApplication::translate("GpuNames", "Integrated Display Controller"));

  static const QRegularExpression reIntegratedGpu(
      QStringLiteral("(?i)(Integrated Graphics|Dahili Grafik|Integrierte "
                     "Grafik|Gr[áa]ficos integrados)"));
  result.replace(reIntegratedGpu, QCoreApplication::translate(
                                      "GpuNames", "Integrated Graphics"));

  static const QRegularExpression reGpuCtrl(
      QStringLiteral("(?i)(Graphics Controller|Grafik "
                     "Denetleyicisi|Grafikcontroller|Controlador de "
                     "gr[áa]ficos)"));
  result.replace(reGpuCtrl, QCoreApplication::translate("GpuNames",
                                                        "Graphics Controller"));

  static const QRegularExpression reDispCtrl(
      QStringLiteral("(?i)(Display Controller|Ekran "
                     "Denetleyicisi|Display-Controller|Controlador de "
                     "pantalla)"));
  result.replace(reDispCtrl,
                 QCoreApplication::translate("GpuNames", "Display Controller"));

  static const QRegularExpression reCoreProc(
      QStringLiteral("(?i)(Core Processor|Çekirdek "
                     "İşlemci|Core-Prozessor|Procesador Core)"));
  result.replace(reCoreProc,
                 QCoreApplication::translate("GpuNames", "Core Processor"));

  static const QRegularExpression reIntegrated(
      QStringLiteral("(?i)\\b(Integrated|Dahili|Integriert|Integrado)\\b"));
  result.replace(reIntegrated,
                 QCoreApplication::translate("GpuNames", "Integrated"));

  static const QRegularExpression spacesRegex(QStringLiteral("\\s+"));
  return result.replace(spacesRegex, QStringLiteral(" ")).trimmed();
}

SystemInfoProvider::SystemInfoProvider(QObject *parent) : QObject(parent) {
  loadDiagnosticReportPreferences();
  initializeStaticInfo();
  refresh();
}

void SystemInfoProvider::initializeStaticInfo() {
  if (m_staticHardwareLoaded) {
    return;
  }

  m_osName = detectOsName();
  m_desktopEnvironment = detectDesktopEnvironment();
  m_kernelVersion = detectKernelVersion();
  m_cpuModel = detectCpuModel();
  m_motherboardModel = detectMotherboardModel();
  m_biosVersion = detectBiosVersion();
  m_cudaVersion = detectCudaVersion();
  m_graphicsApiSummary = detectGraphicsApiSummary();
  m_virtualizationType = detectVirtualizationType();
  m_deviceType = detectDeviceType();
  m_integratedGpuName = detectIntegratedGpuName();
  m_integratedGpuMemory = detectIntegratedGpuMemory();
  m_staticHardwareLoaded = true;
}

void SystemInfoProvider::refresh() {
  if (!m_staticHardwareLoaded) {
    initializeStaticInfo();
  }

  QString nextPowerSource;
  const bool nextOnBattery = detectOnBattery(&nextPowerSource);

  if (m_onBattery == nextOnBattery && m_powerSource == nextPowerSource) {
    return;
  }

  m_onBattery = nextOnBattery;
  m_powerSource = nextPowerSource;
  emit infoChanged();
}

void SystemInfoProvider::rescanHardware() {
  m_staticHardwareLoaded = false;
  initializeStaticInfo();
  QString nextPowerSource;
  m_onBattery = detectOnBattery(&nextPowerSource);
  m_powerSource = nextPowerSource;
  emit infoChanged();
}

bool SystemInfoProvider::requestRestart() {
#if defined(Q_OS_LINUX)
  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 30000;
  const auto result =
      runner.runAsRoot(QStringLiteral("systemctl"), {QStringLiteral("reboot")});
  if (result.success()) {
    return true;
  }

  const auto rebootResult = runner.runAsRoot(QStringLiteral("reboot"), {});
  if (rebootResult.success()) {
    return true;
  }

  return QProcess::startDetached(
      QStandardPaths::findExecutable(QStringLiteral("systemctl")),
      {QStringLiteral("--no-ask-password"), QStringLiteral("reboot")});
#endif
  return false;
}

bool SystemInfoProvider::requestRebootToFirmware() {
#if defined(Q_OS_LINUX)
  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 30000;
  const auto result = runner.runAsRoot(
      QStringLiteral("systemctl"),
      {QStringLiteral("reboot"), QStringLiteral("--firmware-setup")});
  if (result.success()) {
    return true;
  }

  return QProcess::startDetached(
      QStandardPaths::findExecutable(QStringLiteral("systemctl")),
      {QStringLiteral("--no-ask-password"), QStringLiteral("reboot"),
       QStringLiteral("--firmware-setup")});
#endif
  return false;
}

bool SystemInfoProvider::copyToClipboard(const QString &text) {
  if (auto *cb = QGuiApplication::clipboard()) {
    cb->setText(text);
    return true;
  }
  return false;
}

void SystemInfoProvider::loadDiagnosticReportPreferences() {
  QSettings settings;
  settings.beginGroup(QStringLiteral("DiagnosticReport"));
  const QString format =
      settings.value(QStringLiteral("format"), QStringLiteral("markdown"))
          .toString()
          .trimmed()
          .toLower();
  const QString destination =
      settings.value(QStringLiteral("destination"), QStringLiteral("preview"))
          .toString()
          .trimmed()
          .toLower();
  settings.endGroup();
  m_diagnosticReportFormat =
      (format == QStringLiteral("plain") || format == QStringLiteral("json"))
          ? format
          : QStringLiteral("markdown");
  m_diagnosticReportDestination = destination == QStringLiteral("clipboard")
                                      ? QStringLiteral("clipboard")
                                      : QStringLiteral("preview");
}

void SystemInfoProvider::saveDiagnosticReportPreferences() const {
  QSettings settings;
  settings.beginGroup(QStringLiteral("DiagnosticReport"));
  settings.setValue(QStringLiteral("format"), m_diagnosticReportFormat);
  settings.setValue(QStringLiteral("destination"),
                    m_diagnosticReportDestination);
  settings.endGroup();
}

void SystemInfoProvider::setDiagnosticReportFormat(const QString &format) {
  const QString normalized = format.trimmed().toLower();
  const QString next = (normalized == QStringLiteral("plain") ||
                        normalized == QStringLiteral("json"))
                           ? normalized
                           : QStringLiteral("markdown");
  if (m_diagnosticReportFormat == next)
    return;
  m_diagnosticReportFormat = next;
  saveDiagnosticReportPreferences();
  emit diagnosticReportPreferencesChanged();
}

void SystemInfoProvider::setDiagnosticReportDestination(
    const QString &destination) {
  const QString next =
      destination.trimmed().toLower() == QStringLiteral("clipboard")
          ? QStringLiteral("clipboard")
          : QStringLiteral("preview");
  if (m_diagnosticReportDestination == next)
    return;
  m_diagnosticReportDestination = next;
  saveDiagnosticReportPreferences();
  emit diagnosticReportPreferencesChanged();
}

QString SystemInfoProvider::generateSystemReport(
    const QString &gpuName, const QString &driverVer, const QString &vramStr,
    const QString &ramStr, const QString &pcieStr, const QString &secureBoot,
    const QString &format) {
  const QString outputFormat = format.trimmed().toLower();
  if (outputFormat == QStringLiteral("json")) {
    QJsonObject report;
    report.insert(QStringLiteral("report"),
                  QStringLiteral("ro-Control System Diagnostic Report"));
    report.insert(QStringLiteral("operatingSystem"), m_osName);
    report.insert(QStringLiteral("linuxKernel"), m_kernelVersion);
    report.insert(QStringLiteral("desktopEnvironment"), m_desktopEnvironment);
    report.insert(QStringLiteral("processor"), m_cpuModel);
    if (!m_motherboardModel.isEmpty())
      report.insert(QStringLiteral("motherboard"), m_motherboardModel);
    if (!m_biosVersion.isEmpty())
      report.insert(QStringLiteral("uefiBios"), m_biosVersion);
    if (!gpuName.isEmpty())
      report.insert(QStringLiteral("graphicsCard"), gpuName);
    if (!driverVer.isEmpty())
      report.insert(QStringLiteral("nvidiaDriver"), driverVer);
    if (!vramStr.isEmpty())
      report.insert(QStringLiteral("videoMemory"), vramStr);
    if (!m_integratedGpuName.isEmpty() && !m_integratedGpuMemory.isEmpty()) {
      report.insert(QStringLiteral("integratedGraphics"), m_integratedGpuName);
      report.insert(QStringLiteral("integratedGraphicsMemory"),
                    m_integratedGpuMemory);
    }
    if (!ramStr.isEmpty())
      report.insert(QStringLiteral("systemMemory"), ramStr);
    if (!pcieStr.isEmpty())
      report.insert(QStringLiteral("pcieLink"), pcieStr);
    if (!secureBoot.isEmpty())
      report.insert(QStringLiteral("platformSecurity"), secureBoot);
    if (!m_graphicsApiSummary.isEmpty())
      report.insert(QStringLiteral("computeGraphics"), m_graphicsApiSummary);
    return QString::fromUtf8(
        QJsonDocument(report).toJson(QJsonDocument::Indented));
  }

  QString report;
  QTextStream out(&report);
  const bool markdown = outputFormat != QStringLiteral("plain");
  const auto field = [markdown](const QString &label, const QString &value) {
    return markdown ? QStringLiteral("- **%1:** %2\n").arg(label, value)
                    : QStringLiteral("%1: %2\n").arg(label, value);
  };
  out << (markdown ? QStringLiteral("# ro-Control System Diagnostic Report\n\n")
                   : QStringLiteral("ro-Control System Diagnostic Report\n\n"));
  out << field(QStringLiteral("Operating System"), m_osName);
  out << field(QStringLiteral("Linux Kernel"), m_kernelVersion);
  out << field(QStringLiteral("Desktop Environment"), m_desktopEnvironment);
  out << field(QStringLiteral("Processor (CPU)"), m_cpuModel);
  if (!m_motherboardModel.isEmpty()) {
    out << field(QStringLiteral("Motherboard"), m_motherboardModel);
  }
  if (!m_biosVersion.isEmpty()) {
    out << field(QStringLiteral("UEFI / BIOS"), m_biosVersion);
  }
  if (!gpuName.isEmpty()) {
    out << field(QStringLiteral("Graphics Card (GPU)"), gpuName);
  }
  if (!driverVer.isEmpty()) {
    out << field(QStringLiteral("NVIDIA Driver"), driverVer);
  }
  if (!vramStr.isEmpty()) {
    out << field(QStringLiteral("Video Memory (VRAM)"), vramStr);
  }
  if (!m_integratedGpuName.isEmpty() && !m_integratedGpuMemory.isEmpty()) {
    out << field(QStringLiteral("Integrated Graphics"), m_integratedGpuName);
    out << field(QStringLiteral("Integrated Graphics Memory"),
                 m_integratedGpuMemory);
  }
  if (!ramStr.isEmpty()) {
    out << field(QStringLiteral("System Memory (RAM)"), ramStr);
  }
  if (!pcieStr.isEmpty()) {
    out << field(QStringLiteral("PCIe Link"), pcieStr);
  }
  if (!secureBoot.isEmpty()) {
    out << field(QStringLiteral("Platform Security"), secureBoot);
  }
  if (!m_graphicsApiSummary.isEmpty()) {
    out << field(QStringLiteral("Compute & Graphics"), m_graphicsApiSummary);
  }
  return report;
}

QString SystemInfoProvider::detectOsName() const {
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

QString SystemInfoProvider::detectKernelVersion() const {
#if defined(Q_OS_UNIX)
  utsname name{};
  if (uname(&name) == 0) {
    return QString::fromLocal8Bit(name.release);
  }
#endif
  return QSysInfo::kernelVersion();
}

QString SystemInfoProvider::detectCpuModel() const {
#if defined(Q_OS_LINUX)
  CommandRunner runner;
  const auto lscpuResult = runner.run(QStringLiteral("lscpu"));
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

  QFile cpuInfo(QStringLiteral("/proc/cpuinfo"));
  if (cpuInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream stream(&cpuInfo);
    while (!stream.atEnd()) {
      const QString line = stream.readLine();
      if (line.startsWith(QStringLiteral("model name")) ||
          line.startsWith(QStringLiteral("Hardware")) ||
          line.startsWith(QStringLiteral("Processor"))) {
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

QString SystemInfoProvider::detectMotherboardModel() const {
#if defined(Q_OS_LINUX)
  const QString vendor =
      valueFromFile(QStringLiteral("/sys/class/dmi/id/board_vendor")).trimmed();
  const QString name =
      valueFromFile(QStringLiteral("/sys/class/dmi/id/board_name")).trimmed();
  if (!name.isEmpty()) {
    if (!vendor.isEmpty() && !name.startsWith(vendor, Qt::CaseInsensitive)) {
      return vendor + QLatin1Char(' ') + name;
    }
    return name;
  }

  const QString sysVendor =
      valueFromFile(QStringLiteral("/sys/class/dmi/id/sys_vendor")).trimmed();
  const QString prodName =
      valueFromFile(QStringLiteral("/sys/class/dmi/id/product_name")).trimmed();
  if (!prodName.isEmpty()) {
    if (!sysVendor.isEmpty() &&
        !prodName.startsWith(sysVendor, Qt::CaseInsensitive)) {
      return sysVendor + QLatin1Char(' ') + prodName;
    }
    return prodName;
  }
#endif
  const QString virt = !m_virtualizationType.isEmpty()
                           ? m_virtualizationType
                           : detectVirtualizationType();
  if (!virt.isEmpty()) {
    return QStringLiteral("%1 Virtual Motherboard").arg(virt);
  }
  return {};
}

QString SystemInfoProvider::detectBiosVersion() const {
#if defined(Q_OS_LINUX)
  const QString version =
      valueFromFile(QStringLiteral("/sys/class/dmi/id/bios_version")).trimmed();
  const QString date =
      valueFromFile(QStringLiteral("/sys/class/dmi/id/bios_date")).trimmed();
  if (!version.isEmpty()) {
    return !date.isEmpty() ? QStringLiteral("%1 (%2)").arg(version, date)
                           : version;
  }
#endif
  return {};
}

QString SystemInfoProvider::detectCudaVersion() const {
#if defined(Q_OS_LINUX)
  CommandRunner runner;
  const auto smiResult = runner.run(QStringLiteral("nvidia-smi"));
  if (smiResult.success()) {
    static const QRegularExpression cudaRegex(
        QStringLiteral(R"(CUDA Version:\s*([0-9]+\.[0-9]+))"));
    const auto match = cudaRegex.match(smiResult.stdout);
    if (match.hasMatch()) {
      return QStringLiteral("CUDA %1").arg(match.captured(1).trimmed());
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

QString SystemInfoProvider::detectGraphicsApiSummary() const {
  QStringList capabilities;
  const QString cuda = detectCudaVersion();
  if (!cuda.isEmpty()) {
    capabilities << cuda;
  }

#if defined(Q_OS_LINUX)
  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 1500;
  const auto vulkan = runner.run(QStringLiteral("vulkaninfo"),
                                 {QStringLiteral("--summary")}, options);
  if (vulkan.success()) {
    capabilities << QStringLiteral("Vulkan");
  }
#endif
  return capabilities.join(QStringLiteral(" • "));
}

QString SystemInfoProvider::detectVirtualizationType() const {
#if defined(Q_OS_LINUX)
  CommandRunner runner;
  const auto virtResult = runner.run(QStringLiteral("systemd-detect-virt"));
  if (virtResult.success()) {
    const QString output = virtResult.stdout.trimmed();
    if (!output.isEmpty() && output != QStringLiteral("none")) {
      return virtualizationLabel(output);
    }
  }

  return virtualizationLabel(
      valueFromOsRelease(QStringLiteral("VIRTUALIZATION")));
#else
  return {};
#endif
}

QString SystemInfoProvider::detectDeviceType() const {
  const QString virtualizationType = !m_virtualizationType.isEmpty()
                                         ? m_virtualizationType
                                         : detectVirtualizationType();
  if (!virtualizationType.isEmpty()) {
    if (virtualizationType.compare(QStringLiteral("QEMU"),
                                   Qt::CaseInsensitive) == 0 ||
        virtualizationType.compare(QStringLiteral("KVM"),
                                   Qt::CaseInsensitive) == 0) {
      return QStringLiteral("QEMU");
    }
    return virtualizationType;
  }

#if defined(Q_OS_LINUX)
  const QString chassisType =
      valueFromFile(QStringLiteral("/sys/class/dmi/id/chassis_type"));
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
      valueFromFile(QStringLiteral("/sys/class/dmi/id/chassis_vendor")) +
      QLatin1Char(' ') +
      valueFromFile(QStringLiteral("/sys/class/dmi/id/product_name"));
  if (chassisName.contains(QStringLiteral("laptop"), Qt::CaseInsensitive) ||
      chassisName.contains(QStringLiteral("notebook"), Qt::CaseInsensitive)) {
    return QStringLiteral("Laptop");
  }
#endif

  return {};
}

QString SystemInfoProvider::detectDesktopEnvironment() const {
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

QString SystemInfoProvider::detectIntegratedGpuName() const {
#if defined(Q_OS_LINUX)
  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 1500;
  const auto result =
      runner.run(QStringLiteral("lspci"), {QStringLiteral("-mm")}, options);
  if (!result.success())
    return {};

  static const QRegularExpression fieldPattern(QStringLiteral("\"([^\"]+)\""));
  for (const QString &line :
       result.stdout.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
    if (!line.contains(QStringLiteral("VGA"), Qt::CaseInsensitive) &&
        !line.contains(QStringLiteral("3D controller"), Qt::CaseInsensitive) &&
        !line.contains(QStringLiteral("Display controller"),
                       Qt::CaseInsensitive))
      continue;
    auto matches = fieldPattern.globalMatch(line);
    QStringList fields;
    while (matches.hasNext())
      fields << matches.next().captured(1);
    if (fields.size() >= 3 &&
        isIntegratedGpuDescription(fields.at(1), fields.at(2)))
      return normalizedGpuDisplayName(fields.at(1), fields.at(2));
  }
#endif
  return {};
}

QString SystemInfoProvider::detectIntegratedGpuMemory() const {
#if defined(Q_OS_LINUX)
  if (m_integratedGpuName.isEmpty())
    return {};
  const QDir drmRoot(QStringLiteral("/sys/class/drm"));
  const QFileInfoList cards =
      drmRoot.entryInfoList({QStringLiteral("card[0-9]*")},
                            QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const QFileInfo &card : cards) {
    const QString devicePath =
        card.absoluteFilePath() + QStringLiteral("/device");
    const QString vendor =
        valueFromFile(devicePath + QStringLiteral("/vendor"));
    if (vendor != QStringLiteral("0x8086") &&
        vendor != QStringLiteral("0x1002"))
      continue;
    bool ok = false;
    const qint64 bytes =
        valueFromFile(devicePath + QStringLiteral("/mem_info_vram_total"))
            .toLongLong(&ok);
    if (ok && bytes > 0)
      return formatMemoryBytes(bytes);
  }
#endif
  return {};
}

bool SystemInfoProvider::detectOnBattery(QString *sourceLabel) const {
  const QString overrideOnline =
      qEnvironmentVariable("RO_CONTROL_POWER_SUPPLY_ONLINE").trimmed();
  if (!overrideOnline.isEmpty()) {
    const bool onBat = (overrideOnline == QStringLiteral("0"));
    if (sourceLabel) {
      *sourceLabel =
          onBat ? QStringLiteral("Battery") : QStringLiteral("AC Power");
    }
    return onBat;
  }

#if defined(Q_OS_LINUX)
  QDir powerDir(QStringLiteral("/sys/class/power_supply"));
  if (!powerDir.exists()) {
    if (sourceLabel) {
      *sourceLabel = QStringLiteral("AC / Desktop");
    }
    return false;
  }

  const QFileInfoList entries =
      powerDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
  bool hasBattery = false;
  bool isDischarging = false;
  bool isAcOnline = false;

  for (const QFileInfo &entry : entries) {
    const QString type =
        valueFromFile(entry.absoluteFilePath() + QStringLiteral("/type"))
            .trimmed();
    if (type.compare(QStringLiteral("Battery"), Qt::CaseInsensitive) == 0) {
      hasBattery = true;
      const QString status =
          valueFromFile(entry.absoluteFilePath() + QStringLiteral("/status"))
              .trimmed();
      if (status.compare(QStringLiteral("Discharging"), Qt::CaseInsensitive) ==
          0) {
        isDischarging = true;
      }
    } else if (type.compare(QStringLiteral("Mains"), Qt::CaseInsensitive) ==
               0) {
      const QString online =
          valueFromFile(entry.absoluteFilePath() + QStringLiteral("/online"))
              .trimmed();
      if (online == QStringLiteral("1")) {
        isAcOnline = true;
      }
    }
  }

  if (!hasBattery) {
    if (sourceLabel) {
      *sourceLabel = QStringLiteral("AC / Desktop");
    }
    return false;
  }

  const bool onBat = isDischarging || (!isAcOnline && hasBattery);
  if (sourceLabel) {
    *sourceLabel =
        onBat ? QStringLiteral("Battery") : QStringLiteral("AC Power");
  }
  return onBat;
#else
  if (sourceLabel) {
    *sourceLabel = QStringLiteral("AC Power");
  }
  return false;
#endif
}
