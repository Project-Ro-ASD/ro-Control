#include "systeminfoprovider.h"

#include "commandrunner.h"
#include "diagnosticreportformatter.h"
#include "diagnosticreportpreferences.h"
#include "systemplatformprobes.h"

#include <QClipboard>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

QString SystemInfoProvider::localizeGpuName(const QString &rawName) {
  if (rawName.trimmed().isEmpty()) {
    return {};
  }
  QString result = rawName.trimmed();

  // Fast check: if none of the candidate keywords exist in the string,
  // skip all expensive regular expression evaluations.
  static const auto containsAnyKeyword = [](const QString &s) {
    return s.contains(QLatin1String("Integrated"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("Dahili"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("Integriert"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("Integrado"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("Graphics"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("Grafik"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("Gráficos"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("Graficos"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("Display"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("Ekran"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("Processor"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("İşlemci"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("Prozessor"), Qt::CaseInsensitive) ||
           s.contains(QLatin1String("Procesador"), Qt::CaseInsensitive);
  };

  if (!containsAnyKeyword(result)) {
    return result;
  }

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

  if (result.contains(QLatin1String("  "))) {
    static const QRegularExpression spacesRegex(QStringLiteral("\\s+"));
    result.replace(spacesRegex, QStringLiteral(" "));
  }
  return result.trimmed();
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

  const auto info = SystemPlatformProbes::probeStaticInfo();
  m_osName = info.osName;
  m_desktopEnvironment = info.desktopEnvironment;
  m_kernelVersion = info.kernelVersion;
  m_cpuModel = info.cpuModel;
  m_motherboardModel = info.motherboardModel;
  m_biosVersion = info.biosVersion;
  m_cudaVersion = info.cudaVersion;
  m_graphicsApiSummary = info.graphicsApiSummary;
  m_virtualizationType = info.virtualizationType;
  m_deviceType = info.deviceType;
  m_integratedGpuName = info.integratedGpuName;
  m_integratedGpuMemory = info.integratedGpuMemory;
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
  const auto preferences = DiagnosticReportPreferenceStore::load();
  m_diagnosticReportFormat = preferences.format;
  m_diagnosticReportDestination = preferences.destination;
}

void SystemInfoProvider::saveDiagnosticReportPreferences() const {
  DiagnosticReportPreferenceStore::save(
      {.format = m_diagnosticReportFormat,
       .destination = m_diagnosticReportDestination});
}

void SystemInfoProvider::setDiagnosticReportFormat(const QString &format) {
  const QString next =
      DiagnosticReportPreferenceStore::normalizedFormat(format);
  if (m_diagnosticReportFormat == next)
    return;
  m_diagnosticReportFormat = next;
  saveDiagnosticReportPreferences();
  emit diagnosticReportPreferencesChanged();
}

void SystemInfoProvider::setDiagnosticReportDestination(
    const QString &destination) {
  const QString next =
      DiagnosticReportPreferenceStore::normalizedDestination(destination);
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
  return DiagnosticReportFormatter::format(
      {.osName = m_osName,
       .kernelVersion = m_kernelVersion,
       .desktopEnvironment = m_desktopEnvironment,
       .cpuModel = m_cpuModel,
       .motherboardModel = m_motherboardModel,
       .biosVersion = m_biosVersion,
       .graphicsApiSummary = m_graphicsApiSummary,
       .integratedGpuName = m_integratedGpuName,
       .integratedGpuMemory = m_integratedGpuMemory,
       .gpuName = gpuName,
       .driverVersion = driverVer,
       .vram = vramStr,
       .ram = ramStr,
       .pcie = pcieStr,
       .secureBoot = secureBoot},
      format);
}

QString SystemInfoProvider::detectOsName() const {
  return SystemPlatformProbes::detectOsName();
}

QString SystemInfoProvider::detectKernelVersion() const {
  return SystemPlatformProbes::detectKernelVersion();
}

QString SystemInfoProvider::detectCpuModel() const {
  return SystemPlatformProbes::detectCpuModel();
}

QString SystemInfoProvider::detectMotherboardModel() const {
  return SystemPlatformProbes::detectMotherboardModel(m_virtualizationType);
}

QString SystemInfoProvider::detectBiosVersion() const {
  return SystemPlatformProbes::detectBiosVersion();
}

QString SystemInfoProvider::detectCudaVersion() const {
  return SystemPlatformProbes::detectCudaVersion();
}

QString SystemInfoProvider::detectGraphicsApiSummary() const {
  return SystemPlatformProbes::detectGraphicsApiSummary();
}

QString SystemInfoProvider::detectVirtualizationType() const {
  return SystemPlatformProbes::detectVirtualizationType();
}

QString SystemInfoProvider::detectDeviceType() const {
  return SystemPlatformProbes::detectDeviceType(m_virtualizationType);
}

QString SystemInfoProvider::detectDesktopEnvironment() const {
  return SystemPlatformProbes::detectDesktopEnvironment();
}

QString SystemInfoProvider::detectIntegratedGpuName() const {
  return SystemPlatformProbes::detectIntegratedGpuName();
}

QString SystemInfoProvider::detectIntegratedGpuMemory() const {
  return SystemPlatformProbes::detectIntegratedGpuMemory(m_integratedGpuName);
}

bool SystemInfoProvider::detectOnBattery(QString *sourceLabel) const {
  return SystemPlatformProbes::onBattery(sourceLabel);
}
