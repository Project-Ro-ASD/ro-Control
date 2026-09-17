#include "fancontroller.h"
#include "nvidia/detector.h"
#include "system/commandrunner.h"
#include "system/polkit.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <algorithm>
#include <cmath>

namespace {

QString fanSysfsRoot() {
  const QString overridePath =
      qEnvironmentVariable("RO_CONTROL_FAN_SYSFS_ROOT").trimmed();
  return overridePath.isEmpty() ? QStringLiteral("/sys/class/hwmon")
                                : overridePath;
}

bool writeTextFile(const QString &path, const QString &content) {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }
  return file.write(content.toUtf8()) != -1;
}

QString readTextFile(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }
  return QString::fromUtf8(file.readAll()).trimmed();
}

bool isGpuHwmon(const QString &name) {
  const QString lower = name.trimmed().toLower();
  return lower.contains(QStringLiteral("nvidia")) ||
         lower.contains(QStringLiteral("amdgpu")) ||
         lower.contains(QStringLiteral("radeon")) ||
         lower.contains(QStringLiteral("gpu"));
}

QString s_cachedCoretempInput;
QString s_cachedAcpitzInput;
QString s_cachedCpuFanRpmInput;
QString s_cachedSysFanRpmInput;
bool s_hwmonTopologyProbed = false;

struct ExtraFanChannel {
  bool gpuHwmon = false;
  QString basePath;
  QString inputName;
  QString label;
  QString id;
};

QList<ExtraFanChannel> s_extraFanChannels;
bool s_extraFanChannelsProbed = false;

} // namespace

FanController::FanController(QObject *parent) : QObject(parent) {
  m_customCurve = defaultCustomCurve();
  loadSettings();

  m_timer.setInterval(2000);
  m_timer.setTimerType(Qt::CoarseTimer);
  connect(&m_timer, &QTimer::timeout, this, &FanController::refresh);

  detectHardwareCapabilities();
  refresh();
  if (!m_hardwareSetupComplete) {
    runHardwareSetup();
  }
  start();
}

FanController::~FanController() {
  // Safe restoration: if manual or custom mode was applied, restore auto on
  // exit
  if (!m_lastAppliedModeWasAuto && m_controlSupported) {
    executeSetFanSpeed(0, true);
  }
}

bool FanController::supported() const { return m_supported; }

bool FanController::controlSupported() const { return m_controlSupported; }

FanController::ControlCapability FanController::capability() const {
  return m_capability;
}

QString FanController::capabilityString() const {
  return capabilityToString(m_capability);
}

QString FanController::hardwareType() const { return m_hardwareType; }

bool FanController::running() const { return m_timer.isActive(); }

int FanController::fanCount() const { return m_fanCount; }

int FanController::currentFanSpeedPercent() const {
  return m_currentFanSpeedPercent;
}

int FanController::currentRpm() const { return m_currentRpm; }

int FanController::targetFanSpeedPercent() const {
  return m_targetFanSpeedPercent;
}

int FanController::manualFanSpeedPercent() const {
  return m_manualFanSpeedPercent;
}

QString FanController::fanMode() const { return modeToString(m_mode); }

FanController::FanMode FanController::modeEnum() const { return m_mode; }

QStringList FanController::availableModes() const {
  return {QStringLiteral("auto"),     QStringLiteral("silent"),
          QStringLiteral("balanced"), QStringLiteral("performance"),
          QStringLiteral("manual"),   QStringLiteral("custom")};
}

bool FanController::safetyOverrideActive() const {
  return m_safetyOverrideActive;
}

int FanController::thermalThresholdC() const { return m_thermalThresholdC; }

QString FanController::statusMessage() const { return m_statusMessage; }

QVariantList FanController::customCurvePointsVariant() const {
  QVariantList list;
  list.reserve(m_customCurve.size());
  for (const auto &pt : m_customCurve) {
    QVariantMap map;
    map.insert(QStringLiteral("temp"), pt.temperatureC);
    map.insert(QStringLiteral("speed"), pt.fanSpeedPercent);
    list.append(map);
  }
  return list;
}

QVector<FanCurvePoint> FanController::customCurvePoints() const {
  return m_customCurve;
}

int FanController::gpuTemperatureC() const { return m_gpuTemperatureC; }

int FanController::cpuTemperatureC() const { return m_cpuTemperatureC; }

void FanController::updateCpuTemperature(int tempC) {
  if (tempC > 0 && tempC < 130 && m_cpuTemperatureC != tempC) {
    m_cpuTemperatureC = tempC;
    emit cpuTemperatureCChanged();
    updateSystemFansTelemetry();
  }
}

QVariantList FanController::systemFans() const { return m_systemFans; }

int FanController::systemFanCount() const { return m_systemFans.size(); }

int FanController::selectedFanIndex() const { return m_selectedFanIndex; }

QString FanController::selectedFanId() const { return m_selectedFanId; }

void FanController::setSelectedFanIndex(int index) {
  if (index >= 0 && index < m_systemFans.size() &&
      m_selectedFanIndex != index) {
    m_selectedFanIndex = index;
    const QVariantMap fan = m_systemFans.at(index).toMap();
    m_selectedFanId = fan.value(QStringLiteral("id")).toString();
    emit selectedFanIndexChanged();
    emit selectedFanIdChanged();
  }
}

void FanController::setSelectedFanId(const QString &id) {
  if (m_selectedFanId == id) {
    return;
  }
  m_selectedFanId = id;
  for (int i = 0; i < m_systemFans.size(); ++i) {
    if (m_systemFans.at(i).toMap().value(QStringLiteral("id")).toString() ==
        id) {
      if (m_selectedFanIndex != i) {
        m_selectedFanIndex = i;
        emit selectedFanIndexChanged();
      }
      break;
    }
  }
  emit selectedFanIdChanged();
}

void FanController::selectFan(int index) { setSelectedFanIndex(index); }

void FanController::selectFanById(const QString &id) { setSelectedFanId(id); }

bool FanController::smoothingEnabled() const { return m_smoothingEnabled; }

bool FanController::hardwareSetupComplete() const {
  return m_hardwareSetupComplete;
}

QVariantMap FanController::runHardwareSetup() {
  // A topology can change after docking, so a setup is always a fresh probe.
  s_cachedCoretempInput.clear();
  s_cachedAcpitzInput.clear();
  s_cachedCpuFanRpmInput.clear();
  s_cachedSysFanRpmInput.clear();
  s_extraFanChannels.clear();
  s_hwmonTopologyProbed = false;
  s_extraFanChannelsProbed = false;
  detectHardwareCapabilities(true);
  readCurrentFanTelemetry();
  const bool wasComplete = m_hardwareSetupComplete;
  m_hardwareSetupComplete = true;
  saveSettings();
  setStatusMessage(tr("Hardware fan scan complete: %1 channel(s) detected.")
                       .arg(m_systemFans.size()));
  if (!wasComplete) {
    emit hardwareSetupCompleteChanged();
  }

  QVariantMap result;
  result.insert(QStringLiteral("completed"), true);
  result.insert(QStringLiteral("channelCount"), m_systemFans.size());
  result.insert(QStringLiteral("telemetryAvailable"), m_supported);
  result.insert(QStringLiteral("controlSupported"), m_controlSupported);
  result.insert(QStringLiteral("capability"), capabilityString());
  result.insert(QStringLiteral("statusMessage"), m_statusMessage);
  return result;
}

int FanController::rampUpRatePercent() const { return m_rampUpRatePercent; }

int FanController::rampDownRatePercent() const { return m_rampDownRatePercent; }

int FanController::hysteresisTempC() const { return m_hysteresisTempC; }

void FanController::setSmoothingEnabled(bool enabled) {
  if (m_smoothingEnabled == enabled)
    return;
  m_smoothingEnabled = enabled;
  saveSettings();
  emit smoothingEnabledChanged();
}

void FanController::setRampUpRatePercent(int percent) {
  const int clamped = std::clamp(percent, 1, 100);
  if (m_rampUpRatePercent == clamped)
    return;
  m_rampUpRatePercent = clamped;
  saveSettings();
  emit rampRateChanged();
}

void FanController::setRampDownRatePercent(int percent) {
  const int clamped = std::clamp(percent, 1, 100);
  if (m_rampDownRatePercent == clamped)
    return;
  m_rampDownRatePercent = clamped;
  saveSettings();
  emit rampRateChanged();
}

void FanController::setHysteresisTempC(int degrees) {
  const int clamped = std::clamp(degrees, 0, 15);
  if (m_hysteresisTempC == clamped)
    return;
  m_hysteresisTempC = clamped;
  saveSettings();
  emit hysteresisTempCChanged();
}

bool FanController::coolbitsEnabled() const {
  const QString confPath =
      QStringLiteral("/etc/X11/xorg.conf.d/99-nvidia-coolbits.conf");
  if (QFile::exists(confPath)) {
    return true;
  }

  const QFileInfoList entries =
      QDir(QStringLiteral("/etc/X11/xorg.conf.d")).entryInfoList(QDir::Files);
  for (const auto &e : entries) {
    if (readTextFile(e.absoluteFilePath())
            .contains(QStringLiteral("Coolbits"), Qt::CaseInsensitive)) {
      return true;
    }
  }
  return false;
}

bool FanController::enableNvidiaCoolbits() {
  PolkitHelper polkit;
  if (!polkit.isPkexecAvailable()) {
    setStatusMessage(
        tr("Polkit (pkexec) is not available to configure Coolbits."));
    return false;
  }

  const QString configScript = QStringLiteral(
      "mkdir -p /etc/X11/xorg.conf.d && "
      "cat << 'EOF' > /etc/X11/xorg.conf.d/99-nvidia-coolbits.conf\n"
      "Section \"OutputClass\"\n"
      "    Identifier \"nvidia\"\n"
      "    MatchDriver \"nvidia-drm\"\n"
      "    Driver \"nvidia\"\n"
      "    Option \"Coolbits\" \"28\"\n"
      "EndSection\n\n"
      "Section \"Device\"\n"
      "    Identifier \"NvidiaCard\"\n"
      "    Driver \"nvidia\"\n"
      "    Option \"Coolbits\" \"28\"\n"
      "EndSection\n"
      "EOF\n");

  const auto result = polkit.runPrivileged(
      QStringLiteral("sh"), {QStringLiteral("-c"), configScript});
  if (result.success()) {
    setStatusMessage(tr("Coolbits enabled successfully! A session restart or "
                        "reboot is required to activate manual fan control."));
    emit coolbitsEnabledChanged();
    refresh();
    return true;
  }

  setStatusMessage(
      tr("Failed to enable Coolbits: %1")
          .arg(result.stderr.isEmpty() ? result.stdout : result.stderr));
  return false;
}

QString FanController::modeToString(FanMode mode) {
  switch (mode) {
  case FanMode::Silent:
    return QStringLiteral("silent");
  case FanMode::Balanced:
    return QStringLiteral("balanced");
  case FanMode::Performance:
    return QStringLiteral("performance");
  case FanMode::Manual:
    return QStringLiteral("manual");
  case FanMode::Custom:
    return QStringLiteral("custom");
  case FanMode::Auto:
  default:
    return QStringLiteral("auto");
  }
}

FanController::FanMode FanController::stringToMode(const QString &modeStr) {
  const QString lower = modeStr.trimmed().toLower();
  if (lower == QStringLiteral("silent"))
    return FanMode::Silent;
  if (lower == QStringLiteral("balanced"))
    return FanMode::Balanced;
  if (lower == QStringLiteral("performance"))
    return FanMode::Performance;
  if (lower == QStringLiteral("manual"))
    return FanMode::Manual;
  if (lower == QStringLiteral("custom"))
    return FanMode::Custom;
  return FanMode::Auto;
}

QString FanController::capabilityToString(ControlCapability cap) {
  switch (cap) {
  case ControlCapability::Controllable:
    return QStringLiteral("controllable");
  case ControlCapability::TelemetryOnly:
    return QStringLiteral("telemetry_only");
  case ControlCapability::PermissionDenied:
    return QStringLiteral("permission_denied");
  case ControlCapability::Unavailable:
    return QStringLiteral("unavailable");
  case ControlCapability::InitializationFailed:
    return QStringLiteral("initialization_failed");
  case ControlCapability::Unsupported:
  default:
    return QStringLiteral("unsupported");
  }
}

int FanController::calculateCurveFanSpeed(const QVector<FanCurvePoint> &curve,
                                          int temperatureC) {
  if (curve.isEmpty()) {
    return 50;
  }
  if (curve.size() == 1) {
    return std::clamp(curve.first().fanSpeedPercent, 0, 100);
  }

  QVector<FanCurvePoint> sortedCurve = curve;
  std::sort(sortedCurve.begin(), sortedCurve.end(),
            [](const FanCurvePoint &a, const FanCurvePoint &b) {
              if (a.temperatureC == b.temperatureC) {
                return a.fanSpeedPercent < b.fanSpeedPercent;
              }
              return a.temperatureC < b.temperatureC;
            });

  // Deduplicate points with identical temperature by taking the maximum speed
  QVector<FanCurvePoint> cleanCurve;
  cleanCurve.reserve(sortedCurve.size());
  for (const auto &pt : sortedCurve) {
    if (!cleanCurve.isEmpty() &&
        cleanCurve.last().temperatureC == pt.temperatureC) {
      cleanCurve.last().fanSpeedPercent =
          std::max(cleanCurve.last().fanSpeedPercent, pt.fanSpeedPercent);
    } else {
      cleanCurve.append(pt);
    }
  }

  if (cleanCurve.size() == 1) {
    return std::clamp(cleanCurve.first().fanSpeedPercent, 0, 100);
  }

  if (temperatureC <= cleanCurve.first().temperatureC) {
    return std::clamp(cleanCurve.first().fanSpeedPercent, 0, 100);
  }

  if (temperatureC >= cleanCurve.last().temperatureC) {
    return std::clamp(cleanCurve.last().fanSpeedPercent, 0, 100);
  }

  for (qsizetype i = 0; i < cleanCurve.size() - 1; ++i) {
    const auto &p1 = cleanCurve.at(i);
    const auto &p2 = cleanCurve.at(i + 1);

    if (temperatureC >= p1.temperatureC && temperatureC <= p2.temperatureC) {
      if (p2.temperatureC == p1.temperatureC) {
        return std::clamp(std::max(p1.fanSpeedPercent, p2.fanSpeedPercent), 0,
                          100);
      }

      const double ratio =
          static_cast<double>(temperatureC - p1.temperatureC) /
          static_cast<double>(p2.temperatureC - p1.temperatureC);
      const double speed =
          static_cast<double>(p1.fanSpeedPercent) +
          ratio * static_cast<double>(p2.fanSpeedPercent - p1.fanSpeedPercent);
      return std::clamp(static_cast<int>(std::round(speed)), 0, 100);
    }
  }

  return 50;
}

QVector<FanCurvePoint> FanController::defaultSilentCurve() {
  return {{40, 0}, {55, 30}, {68, 50}, {78, 75}, {85, 100}};
}

QVector<FanCurvePoint> FanController::defaultBalancedCurve() {
  return {{40, 30}, {55, 45}, {68, 65}, {78, 85}, {85, 100}};
}

QVector<FanCurvePoint> FanController::defaultPerformanceCurve() {
  return {{35, 45}, {50, 65}, {65, 80}, {75, 90}, {82, 100}};
}

QVector<FanCurvePoint> FanController::defaultCustomCurve() {
  return {{40, 30}, {55, 50}, {70, 70}, {85, 100}};
}

void FanController::start() {
  if (!m_timer.isActive()) {
    m_timer.start();
    emit runningChanged();
  }
}

void FanController::stop() {
  if (m_timer.isActive()) {
    m_timer.stop();
    emit runningChanged();
  }
}

void FanController::refresh() {
  detectHardwareCapabilities();
  readCurrentFanTelemetry();
  evaluateAndApplyFanSpeed(false);
}

void FanController::updateTemperature(int tempC) {
  if (tempC >= 0 && m_gpuTemperatureC != tempC) {
    m_gpuTemperatureC = tempC;
    emit gpuTemperatureCChanged();
    evaluateAndApplyFanSpeed(false);
  }
}

void FanController::updateGpuFanSpeed(int percent) {
  if (percent < 0 || percent > 100 || m_currentFanSpeedPercent == percent) {
    return;
  }
  m_currentFanSpeedPercent = percent;
  emit currentFanSpeedPercentChanged();
  setSupported(true);
  updateSystemFansTelemetry();
}

void FanController::updateGpuThermalLimit(int limitC) {
  if (limitC >= 60 && limitC <= 110 && m_thermalThresholdC != limitC) {
    m_thermalThresholdC = limitC;
    emit thermalThresholdCChanged();
  }
}

void FanController::setFanMode(const QString &mode) {
  const FanMode newMode = stringToMode(mode);
  if (m_mode == newMode) {
    return;
  }

  m_mode = newMode;
  saveSettings();
  emit fanModeChanged();
  evaluateAndApplyFanSpeed(true);
}

void FanController::setManualFanSpeedPercent(int percent) {
  const int clamped = std::clamp(percent, 0, 100);
  if (m_manualFanSpeedPercent == clamped) {
    return;
  }

  m_manualFanSpeedPercent = clamped;
  saveSettings();
  emit manualFanSpeedPercentChanged();

  if (m_mode == FanMode::Manual) {
    evaluateAndApplyFanSpeed(true);
  }
}

bool FanController::setCustomCurvePoint(int index, int tempC,
                                        int speedPercent) {
  if (index < 0 || index >= m_customCurve.size()) {
    return false;
  }

  const int clampedTemp = std::clamp(tempC, 20, 100);
  const int clampedSpeed = std::clamp(speedPercent, 0, 100);

  m_customCurve[index].temperatureC = clampedTemp;
  m_customCurve[index].fanSpeedPercent = clampedSpeed;

  std::sort(m_customCurve.begin(), m_customCurve.end(),
            [](const FanCurvePoint &a, const FanCurvePoint &b) {
              if (a.temperatureC == b.temperatureC) {
                return a.fanSpeedPercent < b.fanSpeedPercent;
              }
              return a.temperatureC < b.temperatureC;
            });

  saveSettings();
  emit customCurvePointsChanged();

  if (m_mode == FanMode::Custom) {
    evaluateAndApplyFanSpeed(true);
  }

  return true;
}

void FanController::resetCustomCurve() {
  m_customCurve = defaultCustomCurve();
  saveSettings();
  emit customCurvePointsChanged();

  if (m_mode == FanMode::Custom) {
    evaluateAndApplyFanSpeed(true);
  }
}

static QVector<FanCurvePoint> presetToCurve(const QString &presetName) {
  const QString lower = presetName.trimmed().toLower();
  if (lower == QStringLiteral("stealth") ||
      lower == QStringLiteral("zero-db") || lower == QStringLiteral("quiet")) {
    return {{45, 0}, {55, 30}, {68, 55}, {80, 80}, {88, 100}};
  }
  if (lower == QStringLiteral("aggressive") ||
      lower == QStringLiteral("overclock") ||
      lower == QStringLiteral("extreme") || lower == QStringLiteral("gaming")) {
    return {{35, 50}, {50, 70}, {65, 85}, {75, 95}, {82, 100}};
  }
  if (lower == QStringLiteral("stepped") || lower == QStringLiteral("ladder")) {
    return {{40, 30}, {55, 30}, {56, 60}, {70, 60}, {71, 90}, {85, 100}};
  }
  if (lower == QStringLiteral("silent")) {
    return FanController::defaultSilentCurve();
  }
  if (lower == QStringLiteral("performance")) {
    return FanController::defaultPerformanceCurve();
  }
  return FanController::defaultBalancedCurve();
}

bool FanController::applyCurvePreset(const QString &presetName) {
  m_customCurve = presetToCurve(presetName);
  saveSettings();
  emit customCurvePointsChanged();

  if (m_mode == FanMode::Custom) {
    evaluateAndApplyFanSpeed(true);
  }
  return true;
}

QString FanController::cycleFanMode() {
  const QString current = fanMode();
  QString next;
  if (current == QStringLiteral("auto")) {
    next = QStringLiteral("silent");
  } else if (current == QStringLiteral("silent")) {
    next = QStringLiteral("balanced");
  } else if (current == QStringLiteral("balanced")) {
    next = QStringLiteral("performance");
  } else if (current == QStringLiteral("performance")) {
    next = QStringLiteral("manual");
  } else if (current == QStringLiteral("manual")) {
    next = QStringLiteral("custom");
  } else {
    next = QStringLiteral("auto");
  }
  setFanMode(next);
  return next;
}

void FanController::resetToAuto() { setFanMode(QStringLiteral("auto")); }

void FanController::loadSettings() {
  QSettings settings;
  settings.beginGroup(QStringLiteral("FanControl"));

  const QString savedMode =
      settings.value(QStringLiteral("mode"), QStringLiteral("auto")).toString();
  m_mode = stringToMode(savedMode);

  m_manualFanSpeedPercent =
      settings.value(QStringLiteral("manualSpeed"), 50).toInt();
  m_manualFanSpeedPercent = std::clamp(m_manualFanSpeedPercent, 0, 100);

  const int count = settings.value(QStringLiteral("curveCount"), 0).toInt();
  if (count >= 2 && count <= 8) {
    QVector<FanCurvePoint> loadedCurve;
    for (int i = 0; i < count; ++i) {
      const int t =
          settings.value(QStringLiteral("curveTemp_%1").arg(i), 40 + i * 15)
              .toInt();
      const int s =
          settings.value(QStringLiteral("curveSpeed_%1").arg(i), 30 + i * 20)
              .toInt();
      loadedCurve.append({std::clamp(t, 20, 100), std::clamp(s, 0, 100)});
    }
    if (!loadedCurve.isEmpty()) {
      std::sort(loadedCurve.begin(), loadedCurve.end(),
                [](const FanCurvePoint &a, const FanCurvePoint &b) {
                  if (a.temperatureC == b.temperatureC) {
                    return a.fanSpeedPercent < b.fanSpeedPercent;
                  }
                  return a.temperatureC < b.temperatureC;
                });
      m_customCurve = loadedCurve;
    }
  } else {
    m_customCurve = defaultCustomCurve();
  }

  m_thermalThresholdC =
      settings.value(QStringLiteral("thermalThreshold"), 85).toInt();
  m_thermalThresholdC = std::clamp(m_thermalThresholdC, 60, 105);

  m_smoothingEnabled =
      settings.value(QStringLiteral("smoothingEnabled"), false).toBool();
  m_hardwareSetupComplete =
      settings.value(QStringLiteral("hardwareSetupComplete"), false).toBool();
  m_batteryProfileSyncEnabled =
      settings.value(QStringLiteral("batteryProfileSyncEnabled"), false)
          .toBool();
  m_rampUpRatePercent = std::clamp(
      settings.value(QStringLiteral("rampUpRate"), 20).toInt(), 1, 100);
  m_rampDownRatePercent = std::clamp(
      settings.value(QStringLiteral("rampDownRate"), 5).toInt(), 1, 100);
  m_hysteresisTempC = std::clamp(
      settings.value(QStringLiteral("hysteresisTempC"), 2).toInt(), 0, 15);
  m_gpuDisplayName =
      settings.value(QStringLiteral("displayName")).toString().trimmed();

  settings.endGroup();

  // CPU Profile initialization & loading
  m_cpuProfile.id = QStringLiteral("cpu_fan_0");
  m_cpuProfile.name = QStringLiteral("Intel CPU Cooler Fan");
  m_cpuProfile.type = QStringLiteral("CPU");
  m_cpuProfile.customCurve = defaultBalancedCurve();
  settings.beginGroup(QStringLiteral("FanControl_cpu"));
  m_cpuProfile.mode = stringToMode(
      settings.value(QStringLiteral("mode"), QStringLiteral("auto"))
          .toString());
  m_cpuProfile.manualSpeedPercent = std::clamp(
      settings.value(QStringLiteral("manualSpeed"), 50).toInt(), 0, 100);
  m_cpuProfile.thermalThresholdC = std::clamp(
      settings.value(QStringLiteral("thermalThreshold"), 90).toInt(), 60, 105);
  m_cpuProfile.name =
      settings.value(QStringLiteral("displayName")).toString().trimmed();
  const int cpuCurveCount =
      settings.value(QStringLiteral("curveCount"), 0).toInt();
  if (cpuCurveCount >= 2 && cpuCurveCount <= 8) {
    QVector<FanCurvePoint> loadedCurve;
    for (int i = 0; i < cpuCurveCount; ++i) {
      const int t =
          settings.value(QStringLiteral("curveTemp_%1").arg(i), 40 + i * 15)
              .toInt();
      const int s =
          settings.value(QStringLiteral("curveSpeed_%1").arg(i), 30 + i * 20)
              .toInt();
      loadedCurve.append({std::clamp(t, 20, 100), std::clamp(s, 0, 100)});
    }
    if (!loadedCurve.isEmpty()) {
      m_cpuProfile.customCurve = loadedCurve;
    }
  }
  settings.endGroup();

  // SYS Profile initialization & loading
  m_sysProfile.id = QStringLiteral("sys_fan_0");
  m_sysProfile.name = QStringLiteral("Chassis Airflow Fan");
  m_sysProfile.type = QStringLiteral("SYS");
  m_sysProfile.customCurve = defaultSilentCurve();
  settings.beginGroup(QStringLiteral("FanControl_sys"));
  m_sysProfile.mode = stringToMode(
      settings.value(QStringLiteral("mode"), QStringLiteral("auto"))
          .toString());
  m_sysProfile.manualSpeedPercent = std::clamp(
      settings.value(QStringLiteral("manualSpeed"), 35).toInt(), 0, 100);
  m_sysProfile.thermalThresholdC = std::clamp(
      settings.value(QStringLiteral("thermalThreshold"), 75).toInt(), 50, 95);
  m_sysProfile.name =
      settings.value(QStringLiteral("displayName")).toString().trimmed();
  const int sysCurveCount =
      settings.value(QStringLiteral("curveCount"), 0).toInt();
  if (sysCurveCount >= 2 && sysCurveCount <= 8) {
    QVector<FanCurvePoint> loadedCurve;
    for (int i = 0; i < sysCurveCount; ++i) {
      const int t =
          settings.value(QStringLiteral("curveTemp_%1").arg(i), 35 + i * 15)
              .toInt();
      const int s =
          settings.value(QStringLiteral("curveSpeed_%1").arg(i), 25 + i * 20)
              .toInt();
      loadedCurve.append({std::clamp(t, 20, 100), std::clamp(s, 0, 100)});
    }
    if (!loadedCurve.isEmpty()) {
      m_sysProfile.customCurve = loadedCurve;
    }
  }
  settings.endGroup();
}

void FanController::saveSettings() {
  QSettings settings;
  settings.beginGroup(QStringLiteral("FanControl"));

  settings.setValue(QStringLiteral("mode"), modeToString(m_mode));
  settings.setValue(QStringLiteral("manualSpeed"), m_manualFanSpeedPercent);
  settings.setValue(QStringLiteral("thermalThreshold"), m_thermalThresholdC);
  settings.setValue(QStringLiteral("smoothingEnabled"), m_smoothingEnabled);
  settings.setValue(QStringLiteral("hardwareSetupComplete"),
                    m_hardwareSetupComplete);
  settings.setValue(QStringLiteral("batteryProfileSyncEnabled"),
                    m_batteryProfileSyncEnabled);
  settings.setValue(QStringLiteral("rampUpRate"), m_rampUpRatePercent);
  settings.setValue(QStringLiteral("rampDownRate"), m_rampDownRatePercent);
  settings.setValue(QStringLiteral("hysteresisTempC"), m_hysteresisTempC);
  settings.setValue(QStringLiteral("displayName"), m_gpuDisplayName);
  settings.setValue(QStringLiteral("curveCount"), m_customCurve.size());

  for (qsizetype i = 0; i < m_customCurve.size(); ++i) {
    settings.setValue(QStringLiteral("curveTemp_%1").arg(i),
                      m_customCurve.at(i).temperatureC);
    settings.setValue(QStringLiteral("curveSpeed_%1").arg(i),
                      m_customCurve.at(i).fanSpeedPercent);
  }
  settings.endGroup();

  // CPU
  settings.beginGroup(QStringLiteral("FanControl_cpu"));
  settings.setValue(QStringLiteral("mode"), modeToString(m_cpuProfile.mode));
  settings.setValue(QStringLiteral("manualSpeed"),
                    m_cpuProfile.manualSpeedPercent);
  settings.setValue(QStringLiteral("thermalThreshold"),
                    m_cpuProfile.thermalThresholdC);
  settings.setValue(QStringLiteral("displayName"), m_cpuProfile.name);
  settings.setValue(QStringLiteral("curveCount"),
                    m_cpuProfile.customCurve.size());
  for (qsizetype i = 0; i < m_cpuProfile.customCurve.size(); ++i) {
    settings.setValue(QStringLiteral("curveTemp_%1").arg(i),
                      m_cpuProfile.customCurve.at(i).temperatureC);
    settings.setValue(QStringLiteral("curveSpeed_%1").arg(i),
                      m_cpuProfile.customCurve.at(i).fanSpeedPercent);
  }
  settings.endGroup();

  // SYS
  settings.beginGroup(QStringLiteral("FanControl_sys"));
  settings.setValue(QStringLiteral("mode"), modeToString(m_sysProfile.mode));
  settings.setValue(QStringLiteral("manualSpeed"),
                    m_sysProfile.manualSpeedPercent);
  settings.setValue(QStringLiteral("thermalThreshold"),
                    m_sysProfile.thermalThresholdC);
  settings.setValue(QStringLiteral("displayName"), m_sysProfile.name);
  settings.setValue(QStringLiteral("curveCount"),
                    m_sysProfile.customCurve.size());
  for (qsizetype i = 0; i < m_sysProfile.customCurve.size(); ++i) {
    settings.setValue(QStringLiteral("curveTemp_%1").arg(i),
                      m_sysProfile.customCurve.at(i).temperatureC);
    settings.setValue(QStringLiteral("curveSpeed_%1").arg(i),
                      m_sysProfile.customCurve.at(i).fanSpeedPercent);
  }
  settings.endGroup();
}

void FanController::detectHardwareCapabilities(bool force) {
  if (m_capabilitiesDetected && !force) {
    return;
  }

  const QString mockCap = qEnvironmentVariable("RO_CONTROL_MOCK_FAN_CAPABILITY")
                              .trimmed()
                              .toLower();
  if (!mockCap.isEmpty()) {
    m_capabilitiesDetected = true;
    if (mockCap == QStringLiteral("controllable")) {
      setSupported(true);
      setControlSupported(true);
      setCapability(ControlCapability::Controllable);
      setHardwareType(QStringLiteral("NVIDIA (NV-CONTROL)"));
      return;
    }
    if (mockCap == QStringLiteral("telemetry_only")) {
      setSupported(true);
      setControlSupported(false);
      setCapability(ControlCapability::TelemetryOnly);
      setHardwareType(QStringLiteral("NVIDIA (Telemetry Only)"));
      return;
    }
    if (mockCap == QStringLiteral("permission_denied")) {
      setSupported(true);
      setControlSupported(false);
      setCapability(ControlCapability::PermissionDenied);
      setHardwareType(QStringLiteral("Linux HWMON (sysfs)"));
      return;
    }
    if (mockCap == QStringLiteral("unsupported")) {
      setSupported(false);
      setControlSupported(false);
      setCapability(ControlCapability::Unsupported);
      setHardwareType(QStringLiteral("None"));
      return;
    }
  }

  m_capabilitiesDetected = true;

  const bool hasSysfsOverride =
      !qEnvironmentVariable("RO_CONTROL_FAN_SYSFS_ROOT").trimmed().isEmpty();

  if (!hasSysfsOverride) {
    // 1. Query NVIDIA's control endpoint. Hardware discovery must never
    // change the current fan-control mode, so do not use a mutating `-a`
    // command here. Writes are attempted only when the user explicitly
    // changes a fan profile or starts the acoustic test.
    const QString nvidiaSettingsProg =
        CommandRunner::resolveProgramPath(QStringLiteral("nvidia-settings"));
    if (!nvidiaSettingsProg.isEmpty()) {
      CommandRunner runner;
      CommandRunner::RunOptions testOpts;
      testOpts.timeoutMs = 1500;
      const auto testRes =
          runner.run(QStringLiteral("nvidia-settings"),
                     {QStringLiteral("-q"),
                      QStringLiteral("[gpu:0]/GPUFanControlState"),
                      QStringLiteral("-t")},
                     testOpts);

      const bool hasPermissionError =
          testRes.stdout.contains(QStringLiteral("permission"),
                                  Qt::CaseInsensitive) ||
          testRes.stderr.contains(QStringLiteral("permission"),
                                  Qt::CaseInsensitive) ||
          testRes.stdout.contains(QStringLiteral("Operation not permitted"),
                                  Qt::CaseInsensitive) ||
          testRes.stderr.contains(QStringLiteral("Operation not permitted"),
                                  Qt::CaseInsensitive);

      if (hasPermissionError) {
        setSupported(true);
        setControlSupported(false);
        setCapability(ControlCapability::TelemetryOnly);
        setHardwareType(QStringLiteral("NVIDIA (Telemetry Only)"));
        setStatusMessage(tr("Automatic Mode: NVIDIA telemetry active."));
        return;
      }

      if (testRes.success() && !hasPermissionError) {
        setSupported(true);
        setControlSupported(true);
        setCapability(ControlCapability::Controllable);
        setHardwareType(QStringLiteral("NVIDIA (NV-CONTROL)"));
        return;
      }
    }
  }

  // 2. Check Sysfs HWMON for GPU fan PWM controls
  m_verifiedHwmonPwmPath.clear();
  m_verifiedHwmonPwmEnablePath.clear();

  const QFileInfoList hwmonEntries =
      QDir(fanSysfsRoot())
          .entryInfoList({QStringLiteral("hwmon*")},
                         QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

  bool foundGpuTelemetry = false;
  bool foundPwm = false;
  bool pwmWritable = false;

  for (const QFileInfo &entry : hwmonEntries) {
    const QString basePath = entry.absoluteFilePath();
    const QString chipName = readTextFile(basePath + QStringLiteral("/name"));
    const bool isGpu = isGpuHwmon(chipName);

    // Check fan RPM inputs
    const QFileInfoList fanInputs = QDir(basePath).entryInfoList(
        {QStringLiteral("fan*_input")}, QDir::Files, QDir::Name);
    if (isGpu && !fanInputs.isEmpty()) {
      foundGpuTelemetry = true;
    }

    // Check PWM controls on GPU devices or designated hwmon
    const QString pwmPath = basePath + QStringLiteral("/pwm1");
    const QString pwmEnablePath = basePath + QStringLiteral("/pwm1_enable");

    if (QFile::exists(pwmPath) && (isGpu || hwmonEntries.size() == 1)) {
      foundPwm = true;
      QFileInfo pwmInfo(pwmPath);
      if (pwmInfo.isWritable()) {
        pwmWritable = true;
        m_verifiedHwmonPwmPath = pwmPath;
        m_verifiedHwmonPwmEnablePath = pwmEnablePath;
        break;
      }
    }
  }

  if (pwmWritable) {
    setSupported(true);
    setControlSupported(true);
    setCapability(ControlCapability::Controllable);
    setHardwareType(QStringLiteral("Linux HWMON (sysfs)"));
    return;
  }

  if (foundPwm) {
    setSupported(true);
    setControlSupported(false);
    setCapability(ControlCapability::PermissionDenied);
    setHardwareType(QStringLiteral("Linux HWMON (sysfs)"));
    return;
  }

  if (foundGpuTelemetry) {
    setSupported(true);
    setControlSupported(false);
    setCapability(ControlCapability::TelemetryOnly);
    setHardwareType(QStringLiteral("Linux HWMON (Read-Only)"));
    return;
  }

  setSupported(false);
  setControlSupported(false);
  setCapability(ControlCapability::Unsupported);
  setHardwareType(QStringLiteral("None"));
}

void FanController::updateSystemFansTelemetry() {
  QVariantList fanList;

  // 1. GPU fan: never create a placeholder when no GPU fan telemetry/control
  // is present. Other vendors remain visible through their actual hwmon fan.
  if (m_supported) {
    const bool isZeroRpm = (m_currentRpm == 0 && m_currentFanSpeedPercent == 0);
    static const QString s_detectedGpuName = []() {
      const QString name = NvidiaDetector::detectGpuNameFromProc();
      return name.isEmpty() ? QStringLiteral("NVIDIA GeForce GPU") : name;
    }();
    const QString fanDisplayName =
        s_detectedGpuName.endsWith(QStringLiteral("Fan"), Qt::CaseInsensitive)
            ? s_detectedGpuName
            : (s_detectedGpuName + QStringLiteral(" Fan"));
    QVariantMap gpuFan;
    gpuFan.insert(QStringLiteral("id"), QStringLiteral("gpu_0"));
    gpuFan.insert(QStringLiteral("name"), m_gpuDisplayName.isEmpty()
                                              ? fanDisplayName
                                              : m_gpuDisplayName);
    gpuFan.insert(QStringLiteral("type"), QStringLiteral("GPU"));
    gpuFan.insert(QStringLiteral("speedPercent"), m_currentFanSpeedPercent);
    gpuFan.insert(QStringLiteral("rpm"), m_currentRpm);
    gpuFan.insert(QStringLiteral("isZeroRpm"), isZeroRpm);
    gpuFan.insert(QStringLiteral("temperatureC"), m_gpuTemperatureC);
    gpuFan.insert(QStringLiteral("targetSpeedPercent"),
                  m_targetFanSpeedPercent);
    gpuFan.insert(QStringLiteral("manualSpeedPercent"),
                  m_manualFanSpeedPercent);
    gpuFan.insert(QStringLiteral("thermalThresholdC"), m_thermalThresholdC);
    gpuFan.insert(QStringLiteral("controllable"), m_controlSupported);
    gpuFan.insert(QStringLiteral("telemetryAvailable"), m_supported);
    gpuFan.insert(QStringLiteral("speedAvailable"), m_supported);
    gpuFan.insert(QStringLiteral("customCurvePoints"),
                  customCurvePointsVariant());
    gpuFan.insert(QStringLiteral("statusLabel"),
                  m_controlSupported ? tr("Controllable")
                  : isZeroRpm        ? tr("0 RPM (Silent)")
                                     : tr("Active (Auto)"));
    gpuFan.insert(QStringLiteral("capability"), capabilityString());
    gpuFan.insert(QStringLiteral("capabilityReason"),
                  m_controlSupported
                      ? tr("Direct hardware fan control active via NV-CONTROL.")
                  : isZeroRpm
                      ? tr("GPU is in 0 RPM silent mode (temperature < 50°C). "
                           "Fans automatically spin up under load.")
                      : tr("Automatic VBIOS cooling curve active."));
    gpuFan.insert(QStringLiteral("mode"), fanMode());
    fanList.append(gpuFan);
  }

  // 2. CPU Fan (Intel CPU Cooler)
  int cpuTemp = m_cpuTemperatureC;
  int cpuRpm = 0;

  int ambientTemp = 0;

  if (!s_hwmonTopologyProbed) {
    s_hwmonTopologyProbed = true;
    const QFileInfoList hwmonEntries =
        QDir(fanSysfsRoot())
            .entryInfoList({QStringLiteral("hwmon*")},
                           QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &entry : hwmonEntries) {
      const QString basePath = entry.absoluteFilePath();
      const QString chipName = readTextFile(basePath + QStringLiteral("/name"));

      const QFileInfoList tempInputs = QDir(basePath).entryInfoList(
          {QStringLiteral("temp*_input")}, QDir::Files, QDir::Name);
      for (const QFileInfo &tFile : tempInputs) {
        if (chipName.contains(QStringLiteral("coretemp"),
                              Qt::CaseInsensitive) &&
            s_cachedCoretempInput.isEmpty()) {
          s_cachedCoretempInput = tFile.absoluteFilePath();
        } else if (chipName.contains(QStringLiteral("acpitz"),
                                     Qt::CaseInsensitive) &&
                   s_cachedAcpitzInput.isEmpty()) {
          s_cachedAcpitzInput = tFile.absoluteFilePath();
        }
      }

      const QFileInfoList fanInputs = QDir(basePath).entryInfoList(
          {QStringLiteral("fan*_input")}, QDir::Files, QDir::Name);
      for (const QFileInfo &fFile : fanInputs) {
        if (isGpuHwmon(chipName)) {
          continue;
        }
        QString baseName = fFile.fileName();
        baseName.remove(QRegularExpression(QStringLiteral("_input$")));
        const QString label =
            readTextFile(basePath + QStringLiteral("/%1_label").arg(baseName));
        const QString normalizedLabel = label.toLower();
        if (s_cachedCpuFanRpmInput.isEmpty() &&
            normalizedLabel.contains(QStringLiteral("cpu"))) {
          s_cachedCpuFanRpmInput = fFile.absoluteFilePath();
        } else if (s_cachedSysFanRpmInput.isEmpty() &&
                   (normalizedLabel.contains(QStringLiteral("sys")) ||
                    normalizedLabel.contains(QStringLiteral("chassis")) ||
                    normalizedLabel.contains(QStringLiteral("case")))) {
          s_cachedSysFanRpmInput = fFile.absoluteFilePath();
        }
      }
    }
  }

  if (s_extraFanChannelsProbed == false) {
    s_extraFanChannelsProbed = true;
    const QFileInfoList hwmonEntries =
        QDir(fanSysfsRoot())
            .entryInfoList({QStringLiteral("hwmon*")},
                           QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &entry : hwmonEntries) {
      const QString basePath = entry.absoluteFilePath();
      const QString chipName = readTextFile(basePath + QStringLiteral("/name"));
      const bool gpuHwmon = isGpuHwmon(chipName);

      const QFileInfoList fanInputs = QDir(basePath).entryInfoList(
          {QStringLiteral("fan*_input")}, QDir::Files, QDir::Name);
      for (const QFileInfo &fFile : fanInputs) {
        const QString abs = fFile.absoluteFilePath();
        if (abs == s_cachedCpuFanRpmInput || abs == s_cachedSysFanRpmInput) {
          continue;
        }
        ExtraFanChannel channel;
        channel.gpuHwmon = gpuHwmon;
        channel.basePath = basePath;
        channel.inputName = fFile.fileName();
        channel.inputName.remove(QRegularExpression(QStringLiteral("_input$")));
        channel.label = readTextFile(
            basePath + QStringLiteral("/%1_label").arg(channel.inputName));
        channel.id =
            QStringLiteral("%1_%2").arg(entry.fileName(), channel.inputName);
        s_extraFanChannels.append(channel);
      }
    }
  }

  if (!s_cachedCoretempInput.isEmpty()) {
    bool ok = false;
    const int tVal = readTextFile(s_cachedCoretempInput).toInt(&ok) / 1000;
    if (ok && tVal > 0 && tVal < 115) {
      cpuTemp = tVal;
    }
  }
  if (!s_cachedAcpitzInput.isEmpty()) {
    bool ok = false;
    const int tVal = readTextFile(s_cachedAcpitzInput).toInt(&ok) / 1000;
    if (ok && tVal > 0 && tVal < 115) {
      ambientTemp = tVal;
    }
  }
  if (!s_cachedCpuFanRpmInput.isEmpty()) {
    bool ok = false;
    const int rpm = readTextFile(s_cachedCpuFanRpmInput).toInt(&ok);
    if (ok && rpm >= 0) {
      cpuRpm = std::clamp(rpm, 0, 100000);
    }
  }

  // A readable tachometer remains a real fan channel while stopped/at 0 RPM.
  const bool cpuTelemetryAvailable = !s_cachedCpuFanRpmInput.isEmpty();
  int cpuSpeedPct = 0;
  if (cpuTelemetryAvailable) {
    switch (m_cpuProfile.mode) {
    case FanMode::Manual:
      cpuSpeedPct = m_cpuProfile.manualSpeedPercent;
      break;
    case FanMode::Silent:
      cpuSpeedPct = calculateCurveFanSpeed(defaultSilentCurve(), cpuTemp);
      break;
    case FanMode::Balanced:
      cpuSpeedPct = calculateCurveFanSpeed(defaultBalancedCurve(), cpuTemp);
      break;
    case FanMode::Performance:
      cpuSpeedPct = calculateCurveFanSpeed(defaultPerformanceCurve(), cpuTemp);
      break;
    case FanMode::Custom:
      cpuSpeedPct = calculateCurveFanSpeed(m_cpuProfile.customCurve, cpuTemp);
      break;
    case FanMode::Auto:
    default:
      cpuSpeedPct = std::clamp(28 + ((cpuTemp - 30) * 8) / 5, 25, 100);
      break;
    }
  }

  QVariantList cpuCurvePoints;
  for (const auto &pt : m_cpuProfile.customCurve) {
    QVariantMap map;
    map.insert(QStringLiteral("temp"), pt.temperatureC);
    map.insert(QStringLiteral("speed"), pt.fanSpeedPercent);
    cpuCurvePoints.append(map);
  }

  QVariantMap cpuFan;
  cpuFan.insert(QStringLiteral("id"), QStringLiteral("cpu_fan_0"));
  cpuFan.insert(QStringLiteral("name"),
                m_cpuProfile.name.isEmpty()
                    ? QStringLiteral("Intel CPU Cooler Fan")
                    : m_cpuProfile.name);
  cpuFan.insert(QStringLiteral("type"), QStringLiteral("CPU"));
  cpuFan.insert(QStringLiteral("speedPercent"), cpuSpeedPct);
  cpuFan.insert(QStringLiteral("rpm"), cpuRpm);
  cpuFan.insert(QStringLiteral("isZeroRpm"), false);
  cpuFan.insert(QStringLiteral("temperatureC"), cpuTemp);
  cpuFan.insert(QStringLiteral("targetSpeedPercent"), cpuSpeedPct);
  cpuFan.insert(QStringLiteral("manualSpeedPercent"),
                m_cpuProfile.manualSpeedPercent);
  cpuFan.insert(QStringLiteral("thermalThresholdC"),
                m_cpuProfile.thermalThresholdC);
  cpuFan.insert(QStringLiteral("controllable"), false);
  cpuFan.insert(QStringLiteral("telemetryAvailable"), cpuTelemetryAvailable);
  cpuFan.insert(QStringLiteral("speedAvailable"), false);
  cpuFan.insert(QStringLiteral("customCurvePoints"), cpuCurvePoints);
  cpuFan.insert(QStringLiteral("statusLabel"),
                m_cpuProfile.mode == FanMode::Auto
                    ? QStringLiteral("Active (BIOS Auto)")
                    : QStringLiteral("Profile (%1)")
                          .arg(modeToString(m_cpuProfile.mode).toUpper()));
  cpuFan.insert(QStringLiteral("capability"),
                QStringLiteral("hardware_managed"));
  cpuFan.insert(QStringLiteral("capabilityReason"),
                tr("Hardware BIOS thermal curve active with dynamic acoustic "
                   "regulation."));
  cpuFan.insert(QStringLiteral("mode"), modeToString(m_cpuProfile.mode));
  if (cpuTelemetryAvailable) {
    fanList.append(cpuFan);
  }

  // 3. Chassis Airflow Fan
  int sysRpm = 0;
  if (!s_cachedSysFanRpmInput.isEmpty()) {
    bool ok = false;
    const int rpm = readTextFile(s_cachedSysFanRpmInput).toInt(&ok);
    if (ok && rpm >= 0) {
      sysRpm = std::clamp(rpm, 0, 100000);
    }
  }
  const bool sysTelemetryAvailable = !s_cachedSysFanRpmInput.isEmpty();
  int sysSpeedPct = 0;
  if (sysTelemetryAvailable) {
    switch (m_sysProfile.mode) {
    case FanMode::Manual:
      sysSpeedPct = m_sysProfile.manualSpeedPercent;
      break;
    case FanMode::Silent:
      sysSpeedPct = calculateCurveFanSpeed(defaultSilentCurve(), ambientTemp);
      break;
    case FanMode::Balanced:
      sysSpeedPct = calculateCurveFanSpeed(defaultBalancedCurve(), ambientTemp);
      break;
    case FanMode::Performance:
      sysSpeedPct =
          calculateCurveFanSpeed(defaultPerformanceCurve(), ambientTemp);
      break;
    case FanMode::Custom:
      sysSpeedPct =
          calculateCurveFanSpeed(m_sysProfile.customCurve, ambientTemp);
      break;
    case FanMode::Auto:
    default:
      sysSpeedPct = 0;
      break;
    }
  }

  QVariantList sysCurvePoints;
  for (const auto &pt : m_sysProfile.customCurve) {
    QVariantMap map;
    map.insert(QStringLiteral("temp"), pt.temperatureC);
    map.insert(QStringLiteral("speed"), pt.fanSpeedPercent);
    sysCurvePoints.append(map);
  }

  QVariantMap chassisFan;
  chassisFan.insert(QStringLiteral("id"), QStringLiteral("sys_fan_0"));
  chassisFan.insert(QStringLiteral("name"),
                    m_sysProfile.name.isEmpty()
                        ? QStringLiteral("Chassis Airflow Fan")
                        : m_sysProfile.name);
  chassisFan.insert(QStringLiteral("type"), QStringLiteral("SYS"));
  chassisFan.insert(QStringLiteral("speedPercent"), sysSpeedPct);
  chassisFan.insert(QStringLiteral("rpm"), sysRpm);
  chassisFan.insert(QStringLiteral("isZeroRpm"), false);
  chassisFan.insert(QStringLiteral("temperatureC"), ambientTemp);
  chassisFan.insert(QStringLiteral("targetSpeedPercent"), sysSpeedPct);
  chassisFan.insert(QStringLiteral("manualSpeedPercent"),
                    m_sysProfile.manualSpeedPercent);
  chassisFan.insert(QStringLiteral("thermalThresholdC"),
                    m_sysProfile.thermalThresholdC);
  chassisFan.insert(QStringLiteral("controllable"), false);
  chassisFan.insert(QStringLiteral("telemetryAvailable"),
                    sysTelemetryAvailable);
  chassisFan.insert(QStringLiteral("speedAvailable"), false);
  chassisFan.insert(QStringLiteral("customCurvePoints"), sysCurvePoints);
  chassisFan.insert(QStringLiteral("statusLabel"),
                    m_sysProfile.mode == FanMode::Auto
                        ? QStringLiteral("Active (Auto)")
                        : QStringLiteral("Profile (%1)")
                              .arg(modeToString(m_sysProfile.mode).toUpper()));
  chassisFan.insert(QStringLiteral("capability"),
                    QStringLiteral("hardware_managed"));
  chassisFan.insert(QStringLiteral("capabilityReason"),
                    tr("Motherboard chassis airflow management curve active."));
  chassisFan.insert(QStringLiteral("mode"), modeToString(m_sysProfile.mode));
  if (sysTelemetryAvailable) {
    fanList.append(chassisFan);
  }

  // Retain every live RPM channel. Motherboard firmware frequently uses labels
  // such as AUXFAN or omits labels entirely, so filtering only CPU/SYS names
  // would hide real cooling hardware.
  for (const ExtraFanChannel &channel : std::as_const(s_extraFanChannels)) {
    bool ok = false;
    const int rpm = readTextFile(channel.basePath + QLatin1Char('/') +
                                 channel.inputName + QStringLiteral("_input"))
                        .toInt(&ok);
    if (!ok || rpm < 0) {
      continue;
    }

    const QString normalizedLabel = channel.label.toLower();
    const QString type = channel.gpuHwmon ? QStringLiteral("GPU")
                         : normalizedLabel.contains(QStringLiteral("cpu"))
                             ? QStringLiteral("CPU")
                             : QStringLiteral("SYS");
    const QString fallbackName =
        type == QStringLiteral("CPU")   ? QStringLiteral("CPU Fan")
        : type == QStringLiteral("GPU") ? QStringLiteral("GPU Fan")
                                        : QStringLiteral("System Fan");
    QVariantMap detectedFan;
    detectedFan.insert(QStringLiteral("id"), channel.id);
    detectedFan.insert(QStringLiteral("name"),
                       channel.label.isEmpty() ? fallbackName : channel.label);
    detectedFan.insert(QStringLiteral("type"), type);
    detectedFan.insert(QStringLiteral("speedPercent"), 0);
    detectedFan.insert(QStringLiteral("rpm"), rpm);
    detectedFan.insert(QStringLiteral("isZeroRpm"), rpm == 0);
    detectedFan.insert(QStringLiteral("temperatureC"),
                       type == QStringLiteral("CPU")   ? cpuTemp
                       : type == QStringLiteral("GPU") ? m_gpuTemperatureC
                                                       : ambientTemp);
    detectedFan.insert(QStringLiteral("targetSpeedPercent"), 0);
    detectedFan.insert(QStringLiteral("manualSpeedPercent"), 0);
    detectedFan.insert(QStringLiteral("thermalThresholdC"), 0);
    detectedFan.insert(QStringLiteral("controllable"), false);
    detectedFan.insert(QStringLiteral("telemetryAvailable"), true);
    detectedFan.insert(QStringLiteral("speedAvailable"), false);
    detectedFan.insert(QStringLiteral("customCurvePoints"), QVariantList{});
    detectedFan.insert(QStringLiteral("statusLabel"), tr("Hardware-managed"));
    detectedFan.insert(QStringLiteral("capability"),
                       QStringLiteral("telemetry_only"));
    detectedFan.insert(QStringLiteral("capabilityReason"),
                       tr("Live RPM telemetry is available; this channel is "
                          "managed by system firmware."));
    detectedFan.insert(QStringLiteral("mode"), QStringLiteral("auto"));
    fanList.append(detectedFan);
  }

  std::stable_sort(
      fanList.begin(), fanList.end(),
      [](const QVariant &left, const QVariant &right) {
        const auto rank = [](const QString &type) {
          if (type == QStringLiteral("CPU"))
            return 0;
          if (type == QStringLiteral("GPU"))
            return 1;
          return 2;
        };
        return rank(left.toMap().value(QStringLiteral("type")).toString()) <
               rank(right.toMap().value(QStringLiteral("type")).toString());
      });

  const bool fansChanged = fanList != m_systemFans;
  m_systemFans = fanList;

  // Selection must always point to a detected channel, never a stale card.
  int selectedIndex = -1;
  for (int i = 0; i < m_systemFans.size(); ++i) {
    if (m_systemFans.at(i).toMap().value(QStringLiteral("id")).toString() ==
        m_selectedFanId) {
      selectedIndex = i;
      break;
    }
  }
  if (selectedIndex < 0 && !m_systemFans.isEmpty()) {
    m_selectedFanId =
        m_systemFans.first().toMap().value(QStringLiteral("id")).toString();
    selectedIndex = 0;
    emit selectedFanIdChanged();
  }
  if (m_selectedFanIndex != selectedIndex) {
    m_selectedFanIndex = selectedIndex;
    emit selectedFanIndexChanged();
  }

  // Keep m_fanCount in sync with the number of GPU fans in the topology so
  // that executeSetFanSpeed can correctly address [fan:1] on dual-fan cards.
  int gpuFanCount = 0;
  for (const QVariant &item : std::as_const(m_systemFans)) {
    const QVariantMap fan = item.toMap();
    if (fan.value(QStringLiteral("type")).toString() == QStringLiteral("GPU"))
      ++gpuFanCount;
  }
  if (gpuFanCount > 0 && m_fanCount != gpuFanCount) {
    m_fanCount = gpuFanCount;
    emit fanCountChanged();
  }

  if (fansChanged) {
    emit systemFansChanged();
  }
}

void FanController::readCurrentFanTelemetry() {
  const QString mockRpm =
      qEnvironmentVariable("RO_CONTROL_MOCK_FAN_RPM").trimmed();
  if (!mockRpm.isEmpty()) {
    bool ok = false;
    const int rpm = mockRpm.toInt(&ok);
    if (ok && rpm >= 0) {
      if (m_currentRpm != rpm) {
        m_currentRpm = rpm;
        emit currentRpmChanged();
      }
      setSupported(true);
      updateSystemFansTelemetry();
      return;
    }
  }

  if (m_capability == ControlCapability::Unsupported) {
    updateSystemFansTelemetry();
    return;
  }

  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 500;

  // 1. Live telemetry queries via nvidia-settings (when available).
  // A missing display/NV-CONTROL endpoint can otherwise make two timed-out
  // subprocesses run on every two-second fan tick. Keep sysfs telemetry live
  // while retrying that optional source at a conservative cadence.
  const QString nvidiaSettingsProg =
      CommandRunner::resolveProgramPath(QStringLiteral("nvidia-settings"));
  constexpr qint64 kNvidiaTelemetrySuccessIntervalMs = 5000;
  constexpr qint64 kNvidiaTelemetryFailureIntervalMs = 30000;
  const qint64 retryIntervalMs = m_nvidiaTelemetryAvailable
                                     ? kNvidiaTelemetrySuccessIntervalMs
                                     : kNvidiaTelemetryFailureIntervalMs;
  const bool shouldQueryNvidiaTelemetry =
      !nvidiaSettingsProg.isEmpty() &&
      (!m_nvidiaTelemetryQueryTimer.isValid() ||
       m_nvidiaTelemetryQueryTimer.elapsed() >= retryIntervalMs);
  if (shouldQueryNvidiaTelemetry) {
    m_nvidiaTelemetryQueryTimer.restart();
    // Query live RPM
    auto rpmRes = runner.run(QStringLiteral("nvidia-settings"),
                             {QStringLiteral("-q"),
                              QStringLiteral("[fan:0]/GPUCurrentFanSpeedRPM"),
                              QStringLiteral("-t")},
                             options);
    if (!rpmRes.success()) {
      rpmRes = runner.run(QStringLiteral("nvidia-settings"),
                          {QStringLiteral("-q"),
                           QStringLiteral("GPUCurrentFanSpeedRPM"),
                           QStringLiteral("-t")},
                          options);
    }
    if (rpmRes.success()) {
      bool ok = false;
      const int rpm = rpmRes.stdout.trimmed()
                          .split(QLatin1Char('\n'))
                          .value(0)
                          .trimmed()
                          .toInt(&ok);
      if (ok && rpm >= 0) {
        if (m_currentRpm != rpm) {
          m_currentRpm = rpm;
          emit currentRpmChanged();
        }
        setSupported(true);
      }
    }
    m_nvidiaTelemetryAvailable = rpmRes.success();
  }

  // 2. Try sysfs hwmon fan input fallback
  const QFileInfoList hwmonEntries =
      QDir(fanSysfsRoot())
          .entryInfoList({QStringLiteral("hwmon*")},
                         QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

  for (const QFileInfo &entry : hwmonEntries) {
    const QString path = entry.absoluteFilePath();
    const QString fanInputPath = path + QStringLiteral("/fan1_input");
    if (QFile::exists(fanInputPath)) {
      bool ok = false;
      const int rpm = readTextFile(fanInputPath).toInt(&ok);
      if (ok && rpm >= 0) {
        if (m_currentRpm != rpm) {
          m_currentRpm = rpm;
          emit currentRpmChanged();
        }
        setSupported(true);
      }
    }

    const QString pwmPath = path + QStringLiteral("/pwm1");
    if (QFile::exists(pwmPath) && m_currentFanSpeedPercent == 0) {
      bool ok = false;
      const int rawPwm = readTextFile(pwmPath).toInt(&ok);
      if (ok && rawPwm >= 0) {
        const int pct =
            std::clamp(static_cast<int>((rawPwm * 100) / 255), 0, 100);
        if (m_currentFanSpeedPercent != pct) {
          m_currentFanSpeedPercent = pct;
          emit currentFanSpeedPercentChanged();
        }
        setSupported(true);
      }
    }
  }

  updateSystemFansTelemetry();
}

void FanController::evaluateAndApplyFanSpeed(bool force) {
  // Thermal safety watchdog model with hysteresis margin
  if (m_gpuTemperatureC >= m_thermalThresholdC) {
    if (!m_safetyOverrideActive) {
      m_safetyOverrideActive = true;
      emit safetyOverrideActiveChanged();
    }
  } else if (m_gpuTemperatureC <=
                 (m_thermalThresholdC - m_thermalRecoveryMarginC) &&
             m_safetyOverrideActive) {
    m_safetyOverrideActive = false;
    emit safetyOverrideActiveChanged();
  }

  int calculatedSpeed = 0;
  bool isAuto = false;

  if (m_safetyOverrideActive) {
    calculatedSpeed = 100;
    setStatusMessage(tr("Safety Override Active: GPU is hot (%1°C >= %2°C). "
                        "Fan forced to 100%.")
                         .arg(m_gpuTemperatureC)
                         .arg(m_thermalThresholdC));
  } else {
    switch (m_mode) {
    case FanMode::Auto:
      isAuto = true;
      calculatedSpeed = 0;
      setStatusMessage(tr("Automatic Mode: Managed by VBIOS and driver."));
      break;
    case FanMode::Silent:
      calculatedSpeed =
          calculateCurveFanSpeed(defaultSilentCurve(), m_gpuTemperatureC);
      setStatusMessage(tr("Silent Profile Active (%1% @ %2°C).")
                           .arg(calculatedSpeed)
                           .arg(m_gpuTemperatureC));
      break;
    case FanMode::Balanced:
      calculatedSpeed =
          calculateCurveFanSpeed(defaultBalancedCurve(), m_gpuTemperatureC);
      setStatusMessage(tr("Balanced Optimization Active (%1% @ %2°C).")
                           .arg(calculatedSpeed)
                           .arg(m_gpuTemperatureC));
      break;
    case FanMode::Performance:
      calculatedSpeed =
          calculateCurveFanSpeed(defaultPerformanceCurve(), m_gpuTemperatureC);
      setStatusMessage(tr("Performance Profile Active (%1% @ %2°C).")
                           .arg(calculatedSpeed)
                           .arg(m_gpuTemperatureC));
      break;
    case FanMode::Manual:
      calculatedSpeed = m_manualFanSpeedPercent;
      setStatusMessage(
          tr("Manual Fan Speed Locked at %1%.").arg(calculatedSpeed));
      break;
    case FanMode::Custom:
      calculatedSpeed =
          calculateCurveFanSpeed(m_customCurve, m_gpuTemperatureC);
      setStatusMessage(tr("Custom Curve Active (%1% @ %2°C).")
                           .arg(calculatedSpeed)
                           .arg(m_gpuTemperatureC));
      break;
    }

    // Directional Hysteresis:
    // When temperature rises, increase fan speed immediately.
    // When temperature falls, prevent fan hunting if temperature hasn't dropped
    // by at least m_hysteresisTempC.
    if (!isAuto && !force && m_lastEvaluatedTempC > 0 &&
        m_gpuTemperatureC < m_lastEvaluatedTempC) {
      if (m_gpuTemperatureC > (m_lastEvaluatedTempC - m_hysteresisTempC) &&
          !m_lastAppliedModeWasAuto && m_lastAppliedPercent > 0) {
        calculatedSpeed = std::max(calculatedSpeed, m_lastAppliedPercent);
      }
    }

    // Fan smoothing / step rate limiter:
    if (!isAuto && !force && m_smoothingEnabled && m_lastAppliedPercent > 0 &&
        !m_lastAppliedModeWasAuto) {
      if (calculatedSpeed > m_lastAppliedPercent) {
        calculatedSpeed = std::min(calculatedSpeed,
                                   m_lastAppliedPercent + m_rampUpRatePercent);
      } else if (calculatedSpeed < m_lastAppliedPercent) {
        calculatedSpeed = std::max(calculatedSpeed, m_lastAppliedPercent -
                                                        m_rampDownRatePercent);
      }
    }
  }

  m_lastEvaluatedTempC = m_gpuTemperatureC;

  if (m_targetFanSpeedPercent != calculatedSpeed) {
    m_targetFanSpeedPercent = calculatedSpeed;
    emit targetFanSpeedPercentChanged();
  }

  const bool skipHardwareWrite =
      !force && ((isAuto && m_lastAppliedModeWasAuto) ||
                 (!isAuto && !m_lastAppliedModeWasAuto &&
                  calculatedSpeed == m_lastAppliedPercent));

  m_lastAppliedPercent = calculatedSpeed;
  m_lastAppliedModeWasAuto = isAuto;

  if (!skipHardwareWrite) {
    executeSetFanSpeed(calculatedSpeed, isAuto);
  }
}

bool FanController::executeSetFanSpeed(int percent, bool isAutoMode) {
  if (!m_controlSupported) {
    emit fanSpeedApplied(percent, false);
    return false;
  }

  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 2000;

  bool success = false;

  const QString nvidiaSettingsProg =
      CommandRunner::resolveProgramPath(QStringLiteral("nvidia-settings"));

  if (!nvidiaSettingsProg.isEmpty() &&
      m_hardwareType.contains(QStringLiteral("NVIDIA"))) {
    QStringList args;
    if (isAutoMode) {
      args << QStringLiteral("-a")
           << QStringLiteral("[gpu:0]/GPUFanControlState=0");
    } else {
      args << QStringLiteral("-a")
           << QStringLiteral("[gpu:0]/GPUFanControlState=1")
           << QStringLiteral("-a")
           << QStringLiteral("[fan:0]/GPUTargetFanSpeed=%1").arg(percent);
      // Address every fan exposed by NV-CONTROL. Multi-fan cards are common,
      // and leaving channels after fan:1 in automatic mode defeats a manual
      // cooling profile.
      for (int fanIndex = 1; fanIndex < m_fanCount; ++fanIndex) {
        args << QStringLiteral("-a")
             << QStringLiteral("[fan:%1]/GPUTargetFanSpeed=%2")
                    .arg(fanIndex)
                    .arg(percent);
      }
    }

    const auto result =
        runner.run(QStringLiteral("nvidia-settings"), args, options);
    const bool hasPermissionError =
        result.stdout.contains(QStringLiteral("permission"),
                               Qt::CaseInsensitive) ||
        result.stderr.contains(QStringLiteral("permission"),
                               Qt::CaseInsensitive) ||
        result.stdout.contains(QStringLiteral("Operation not permitted"),
                               Qt::CaseInsensitive) ||
        result.stderr.contains(QStringLiteral("Operation not permitted"),
                               Qt::CaseInsensitive) ||
        result.stdout.contains(QStringLiteral("ERROR:"), Qt::CaseInsensitive) ||
        result.stderr.contains(QStringLiteral("ERROR:"), Qt::CaseInsensitive);

    if (hasPermissionError) {
      success = false;
      setStatusMessage(tr("NVIDIA fan control rejected by driver: Coolbits "
                          "option is required in Xorg configuration."));
      setControlSupported(false);
      setCapability(ControlCapability::TelemetryOnly);
    } else {
      success = result.success();
    }
  } else if (!m_verifiedHwmonPwmPath.isEmpty()) {
    if (!m_verifiedHwmonPwmEnablePath.isEmpty() &&
        QFile::exists(m_verifiedHwmonPwmEnablePath)) {
      writeTextFile(m_verifiedHwmonPwmEnablePath,
                    isAutoMode ? QStringLiteral("2") : QStringLiteral("1"));
    }
    if (!isAutoMode && QFile::exists(m_verifiedHwmonPwmPath)) {
      const int rawPwm = std::clamp((percent * 255) / 100, 0, 255);
      if (writeTextFile(m_verifiedHwmonPwmPath, QString::number(rawPwm))) {
        success = true;
      }
    } else if (isAutoMode) {
      success = true;
    }
  }

  m_lastAppliedPercent = percent;
  m_lastAppliedModeWasAuto = isAutoMode;

  emit fanSpeedApplied(percent, success);
  return success;
}

void FanController::setSupported(bool value) {
  if (m_supported != value) {
    m_supported = value;
    emit supportedChanged();
  }
}

void FanController::setControlSupported(bool value) {
  if (m_controlSupported != value) {
    m_controlSupported = value;
    emit controlSupportedChanged();
  }
}

void FanController::setCapability(ControlCapability cap) {
  if (m_capability != cap) {
    m_capability = cap;
    emit capabilityChanged();
  }
}

void FanController::setHardwareType(const QString &hwType) {
  if (m_hardwareType != hwType) {
    m_hardwareType = hwType;
    emit hardwareTypeChanged();
  }
}

void FanController::setStatusMessage(const QString &msg) {
  if (m_statusMessage != msg) {
    m_statusMessage = msg;
    emit statusMessageChanged();
  }
}

QVariantMap FanController::getFanConfig(const QString &fanId) {
  for (const auto &item : m_systemFans) {
    const QVariantMap map = item.toMap();
    if (map.value(QStringLiteral("id")).toString() == fanId) {
      return map;
    }
  }
  return {};
}

bool FanController::setFanModeForFan(const QString &fanId,
                                     const QString &mode) {
  if (fanId == QStringLiteral("gpu_0") || fanId.isEmpty()) {
    if (!m_controlSupported) {
      setStatusMessage(tr("Direct GPU fan control is not available."));
      return false;
    }
    setFanMode(mode);
    return true;
  }
  setStatusMessage(tr("This fan is monitored by firmware and cannot be controlled by ro-Control."));
  return false;
}

bool FanController::setManualSpeedForFan(const QString &fanId, int percent) {
  const int clamped = std::clamp(percent, 0, 100);
  if (fanId == QStringLiteral("gpu_0") || fanId.isEmpty()) {
    if (!m_controlSupported)
      return false;
    setManualFanSpeedPercent(clamped);
    return true;
  }
  return false;
}

bool FanController::setCustomCurvePointForFan(const QString &fanId, int index,
                                              int tempC, int speedPercent) {
  if (fanId == QStringLiteral("gpu_0") || fanId.isEmpty()) {
    if (!m_controlSupported)
      return false;
    return setCustomCurvePoint(index, tempC, speedPercent);
  }
  return false;
}

bool FanController::applyCurvePresetForFan(const QString &fanId,
                                           const QString &presetName) {
  if (fanId == QStringLiteral("gpu_0") || fanId.isEmpty()) {
    if (!m_controlSupported)
      return false;
    return applyCurvePreset(presetName);
  }
  return false;
}

bool FanController::resetCustomCurveForFan(const QString &fanId) {
  if (fanId == QStringLiteral("gpu_0") || fanId.isEmpty()) {
    if (!m_controlSupported)
      return false;
    resetCustomCurve();
    return true;
  }
  return false;
}

bool FanController::setThermalThresholdForFan(const QString &fanId, int tempC) {
  const int clamped = std::clamp(tempC, 50, 105);
  if (fanId == QStringLiteral("gpu_0") || fanId.isEmpty()) {
    if (!m_controlSupported)
      return false;
    if (m_thermalThresholdC != clamped) {
      m_thermalThresholdC = clamped;
      emit thermalThresholdCChanged();
      saveSettings();
      updateSystemFansTelemetry();
    }
    return true;
  }
  return false;
}

bool FanController::resetFanToAuto(const QString &fanId) {
  return setFanModeForFan(fanId, QStringLiteral("auto"));
}

bool FanController::applyFanConfiguration(const QString &fanId) {
  if (fanId == QStringLiteral("gpu_0") || fanId.isEmpty()) {
    if (!m_controlSupported)
      return false;
    evaluateAndApplyFanSpeed(true);
    return m_controlSupported;
  }
  return false;
}

bool FanController::setFanDisplayName(const QString &fanId,
                                      const QString &displayName) {
  const QString name = displayName.trimmed().left(64);
  if (name.isEmpty()) {
    return false;
  }

  if (fanId == QStringLiteral("gpu_0")) {
    m_gpuDisplayName = name;
  } else if (fanId == QStringLiteral("cpu_fan_0")) {
    m_cpuProfile.name = name;
  } else if (fanId == QStringLiteral("sys_fan_0")) {
    m_sysProfile.name = name;
  } else {
    return false;
  }

  saveSettings();
  updateSystemFansTelemetry();
  return true;
}

bool FanController::testFanSpeedForFan(const QString &fanId, int percent) {
  if (fanId != QStringLiteral("gpu_0") || !m_controlSupported) {
    return false;
  }
  return executeSetFanSpeed(std::clamp(percent, 0, 100), false);
}

bool FanController::restoreFanControlForFan(const QString &fanId) {
  if (fanId != QStringLiteral("gpu_0")) {
    return false;
  }
  evaluateAndApplyFanSpeed(true);
  return true;
}

bool FanController::batteryProfileSyncEnabled() const {
  return m_batteryProfileSyncEnabled;
}

void FanController::setBatteryProfileSyncEnabled(bool enabled) {
  if (m_batteryProfileSyncEnabled == enabled) {
    return;
  }
  m_batteryProfileSyncEnabled = enabled;
  saveSettings();
  emit batteryProfileSyncEnabledChanged();
}

void FanController::syncPowerSource(bool onBattery) {
  if (!m_batteryProfileSyncEnabled) {
    return;
  }

  if (onBattery) {
    if (m_mode != FanMode::Silent && m_mode != FanMode::Auto) {
      m_preBatteryFanMode = fanMode();
      setFanMode(QStringLiteral("silent"));
    }
  } else {
    if (!m_preBatteryFanMode.isEmpty()) {
      setFanMode(m_preBatteryFanMode);
      m_preBatteryFanMode.clear();
    }
  }
}

bool FanController::exportProfile(const QString &profileName,
                                  const QString &filePath) {
  const QString name = profileName.trimmed().isEmpty()
                           ? QStringLiteral("ro-control-fan-profile")
                           : profileName.trimmed();

  QString targetPath = filePath.trimmed();
  if (targetPath.isEmpty()) {
    const QString baseDir =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
        QStringLiteral("/profiles");
    QDir().mkpath(baseDir);
    targetPath = baseDir + QLatin1Char('/') + name + QStringLiteral(".json");
  }

  QJsonObject rootObj;
  rootObj[QStringLiteral("version")] = 1;
  rootObj[QStringLiteral("profileName")] = name;
  rootObj[QStringLiteral("fanMode")] = fanMode();
  rootObj[QStringLiteral("manualSpeed")] = m_manualFanSpeedPercent;
  rootObj[QStringLiteral("thermalThreshold")] = m_thermalThresholdC;

  QJsonArray curveArray;
  for (const FanCurvePoint &pt : m_customCurve) {
    QJsonObject ptObj;
    ptObj[QStringLiteral("temp")] = pt.temperatureC;
    ptObj[QStringLiteral("speed")] = pt.fanSpeedPercent;
    curveArray.append(ptObj);
  }
  rootObj[QStringLiteral("curve")] = curveArray;

  QFile file(targetPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    emit profileExported(name, false);
    return false;
  }

  const QJsonDocument doc(rootObj);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();

  emit profileExported(name, true);
  return true;
}

bool FanController::importProfile(const QString &filePath) {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    emit profileImported(filePath, false);
    return false;
  }

  const QByteArray data = file.readAll();
  file.close();

  QJsonParseError parseError;
  const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
  if (doc.isNull() || !doc.isObject()) {
    emit profileImported(filePath, false);
    return false;
  }

  const QJsonObject rootObj = doc.object();
  const QString name = rootObj.value(QStringLiteral("profileName")).toString();
  const QString mode = rootObj.value(QStringLiteral("fanMode")).toString();
  const int manualSpeed =
      rootObj.value(QStringLiteral("manualSpeed")).toInt(50);
  const int thermalThresh =
      rootObj.value(QStringLiteral("thermalThreshold")).toInt(85);

  if (rootObj.contains(QStringLiteral("curve"))) {
    const QJsonArray curveArray =
        rootObj.value(QStringLiteral("curve")).toArray();
    QVector<FanCurvePoint> importedCurve;
    for (const auto &val : curveArray) {
      const QJsonObject ptObj = val.toObject();
      FanCurvePoint pt;
      pt.temperatureC = ptObj.value(QStringLiteral("temp")).toInt();
      pt.fanSpeedPercent = ptObj.value(QStringLiteral("speed")).toInt();
      if (pt.temperatureC > 0 && pt.fanSpeedPercent >= 0) {
        importedCurve.append(pt);
      }
    }
    if (!importedCurve.isEmpty()) {
      std::sort(importedCurve.begin(), importedCurve.end(),
                [](const FanCurvePoint &a, const FanCurvePoint &b) {
                  return a.temperatureC < b.temperatureC;
                });
      m_customCurve = importedCurve;
      emit customCurvePointsChanged();
    }
  }

  m_thermalThresholdC = std::clamp(thermalThresh, 60, 95);
  emit thermalThresholdCChanged();

  setManualFanSpeedPercent(manualSpeed);
  if (!mode.isEmpty()) {
    setFanMode(mode);
  }

  saveSettings();
  evaluateAndApplyFanSpeed(true);

  emit profileImported(name.isEmpty() ? filePath : name, true);
  return true;
}

QStringList FanController::listSavedProfiles() const {
  const QString baseDir =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
      QStringLiteral("/profiles");
  QDir dir(baseDir);
  if (!dir.exists()) {
    return {};
  }
  return dir.entryList({QStringLiteral("*.json")}, QDir::Files, QDir::Name);
}
