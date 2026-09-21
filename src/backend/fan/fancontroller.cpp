#include "fancontroller.h"
#include "fanhardwareprobe.h"
#include "fanhardwarewriter.h"
#include "fanprofilemanager.h"
#include "fantelemetryreader.h"

#include <algorithm>
#include <cmath>

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
  if (m_cpuTemperatureC == tempC) {
    return;
  }
  m_cpuTemperatureC = tempC;
  emit cpuTemperatureCChanged();
  updateSystemFansTelemetry();
}

QVariantList FanController::systemFans() const { return m_systemFans; }
int FanController::systemFanCount() const { return m_systemFans.size(); }
int FanController::selectedFanIndex() const { return m_selectedFanIndex; }
QString FanController::selectedFanId() const { return m_selectedFanId; }

void FanController::setSelectedFanIndex(int index) {
  if (index < 0 || index >= m_systemFans.size() ||
      index == m_selectedFanIndex) {
    return;
  }
  m_selectedFanIndex = index;
  m_selectedFanId =
      m_systemFans.at(index).toMap().value(QStringLiteral("id")).toString();
  emit selectedFanIndexChanged();
  emit selectedFanIdChanged();
}

void FanController::setSelectedFanId(const QString &id) {
  if (id.isEmpty() || id == m_selectedFanId) {
    return;
  }
  for (int i = 0; i < m_systemFans.size(); ++i) {
    if (m_systemFans.at(i).toMap().value(QStringLiteral("id")).toString() ==
        id) {
      m_selectedFanId = id;
      m_selectedFanIndex = i;
      emit selectedFanIdChanged();
      emit selectedFanIndexChanged();
      return;
    }
  }
}

void FanController::selectFan(int index) { setSelectedFanIndex(index); }
void FanController::selectFanById(const QString &id) { setSelectedFanId(id); }
bool FanController::smoothingEnabled() const { return m_smoothingEnabled; }
bool FanController::hardwareSetupComplete() const {
  return m_hardwareSetupComplete;
}

QVariantMap FanController::runHardwareSetup() {
  m_topology.reset();
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
  if (m_smoothingEnabled == enabled) {
    return;
  }
  m_smoothingEnabled = enabled;
  saveSettings();
  emit smoothingEnabledChanged();
}

void FanController::setRampUpRatePercent(int percent) {
  const int clamped = std::clamp(percent, 1, 100);
  if (m_rampUpRatePercent == clamped) {
    return;
  }
  m_rampUpRatePercent = clamped;
  saveSettings();
  emit rampRateChanged();
}

void FanController::setRampDownRatePercent(int percent) {
  const int clamped = std::clamp(percent, 1, 100);
  if (m_rampDownRatePercent == clamped) {
    return;
  }
  m_rampDownRatePercent = clamped;
  saveSettings();
  emit rampRateChanged();
}

void FanController::setHysteresisTempC(int degrees) {
  const int clamped = std::clamp(degrees, 0, 15);
  if (m_hysteresisTempC == clamped) {
    return;
  }
  m_hysteresisTempC = clamped;
  saveSettings();
  emit hysteresisTempCChanged();
}

bool FanController::coolbitsEnabled() const {
  return FanHardwareWriter::coolbitsEnabled();
}

bool FanController::enableNvidiaCoolbits() {
  QString statusMsg;
  const bool ok = FanHardwareWriter::enableNvidiaCoolbits(statusMsg);
  setStatusMessage(statusMsg);
  if (ok) {
    emit coolbitsEnabledChanged();
    refresh();
  }
  return ok;
}

QString FanController::modeToString(FanMode mode) {
  return FanProfileManager::modeToString(
      static_cast<FanProfileManager::FanMode>(mode));
}

FanController::FanMode FanController::stringToMode(const QString &modeStr) {
  return static_cast<FanController::FanMode>(
      FanProfileManager::stringToMode(modeStr));
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
    return QStringLiteral("init_failed");
  case ControlCapability::Unsupported:
  default:
    return QStringLiteral("unsupported");
  }
}

int FanController::calculateCurveFanSpeed(const QVector<FanCurvePoint> &curve,
                                          int temperatureC) {
  return FanProfileManager::calculateCurveFanSpeed(curve, temperatureC);
}

QVector<FanCurvePoint> FanController::defaultSilentCurve() {
  return FanProfileManager::defaultSilentCurve();
}

QVector<FanCurvePoint> FanController::defaultBalancedCurve() {
  return FanProfileManager::defaultBalancedCurve();
}

QVector<FanCurvePoint> FanController::defaultPerformanceCurve() {
  return FanProfileManager::defaultPerformanceCurve();
}

QVector<FanCurvePoint> FanController::defaultCustomCurve() {
  return FanProfileManager::defaultCustomCurve();
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
  evaluateAndApplyFanSpeed();
}

void FanController::updateTemperature(int tempC) {
  if (m_gpuTemperatureC == tempC) {
    return;
  }
  m_gpuTemperatureC = tempC;
  emit gpuTemperatureCChanged();
  evaluateAndApplyFanSpeed();
}

void FanController::updateGpuFanSpeed(int percent) {
  const int clamped = std::clamp(percent, 0, 100);
  if (m_currentFanSpeedPercent == clamped) {
    return;
  }
  m_currentFanSpeedPercent = clamped;
  emit currentFanSpeedPercentChanged();
  updateSystemFansTelemetry();
}

void FanController::updateGpuThermalLimit(int limitC) {
  if (limitC > 60 && limitC <= 100 && m_thermalThresholdC != limitC) {
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

  m_customCurve[index].temperatureC = std::clamp(tempC, 20, 100);
  m_customCurve[index].fanSpeedPercent = std::clamp(speedPercent, 0, 100);

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

bool FanController::applyCurvePreset(const QString &presetName) {
  const QVector<FanCurvePoint> points =
      FanProfileManager::presetToCurve(presetName);
  if (points.isEmpty()) {
    return false;
  }

  m_customCurve = points;
  emit customCurvePointsChanged();
  saveSettings();

  if (m_mode == FanMode::Custom) {
    evaluateAndApplyFanSpeed(true);
  }
  return true;
}

QString FanController::cycleFanMode() {
  switch (m_mode) {
  case FanMode::Auto:
    setFanMode(QStringLiteral("silent"));
    break;
  case FanMode::Silent:
    setFanMode(QStringLiteral("balanced"));
    break;
  case FanMode::Balanced:
    setFanMode(QStringLiteral("performance"));
    break;
  case FanMode::Performance:
    setFanMode(QStringLiteral("manual"));
    break;
  case FanMode::Manual:
    setFanMode(QStringLiteral("custom"));
    break;
  case FanMode::Custom:
    setFanMode(QStringLiteral("auto"));
    break;
  }
  return fanMode();
}

void FanController::resetToAuto() { setFanMode(QStringLiteral("auto")); }

void FanController::loadSettings() {
  FanProfileManager::FanMode fMode =
      static_cast<FanProfileManager::FanMode>(m_mode);
  m_profileManager.loadSettings(
      fMode, m_manualFanSpeedPercent, m_thermalThresholdC, m_smoothingEnabled,
      m_hardwareSetupComplete, m_batteryProfileSyncEnabled, m_rampUpRatePercent,
      m_rampDownRatePercent, m_hysteresisTempC, m_gpuDisplayName, m_customCurve,
      m_cpuProfile, m_sysProfile);
  m_mode = static_cast<FanMode>(fMode);
}

void FanController::saveSettings() {
  m_profileManager.saveSettings(
      static_cast<FanProfileManager::FanMode>(m_mode), m_manualFanSpeedPercent,
      m_thermalThresholdC, m_smoothingEnabled, m_hardwareSetupComplete,
      m_batteryProfileSyncEnabled, m_rampUpRatePercent, m_rampDownRatePercent,
      m_hysteresisTempC, m_gpuDisplayName, m_customCurve, m_cpuProfile,
      m_sysProfile);
}

void FanController::detectHardwareCapabilities(bool force) {
  if (m_capabilitiesDetected && !force) {
    return;
  }
  m_capabilitiesDetected = true;

  const auto res = FanHardwareProbe::detectHardwareCapabilities();
  setSupported(res.supported);
  setControlSupported(res.controlSupported);
  setCapability(static_cast<ControlCapability>(res.capability));
  setHardwareType(res.hardwareType);
  if (!res.verifiedHwmonPwmPath.isEmpty()) {
    m_verifiedHwmonPwmPath = res.verifiedHwmonPwmPath;
    m_verifiedHwmonPwmEnablePath = res.verifiedHwmonPwmEnablePath;
  }
  if (!res.statusMessage.isEmpty()) {
    setStatusMessage(res.statusMessage);
  }
}

void FanController::updateSystemFansTelemetry() {
  const QVariantList fanList = FanTelemetryReader::collectSystemFans(
      m_supported, m_controlSupported, capabilityString(),
      m_currentFanSpeedPercent, m_currentRpm, m_targetFanSpeedPercent,
      m_manualFanSpeedPercent, m_thermalThresholdC, fanMode(), m_gpuDisplayName,
      customCurvePointsVariant(), m_gpuTemperatureC, m_cpuTemperatureC,
      m_cpuProfile, m_sysProfile, m_topology);

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
  const auto res = FanTelemetryReader::readGpuTelemetry(
      m_currentRpm, m_currentFanSpeedPercent,
      static_cast<FanHardwareProbe::ProbeCapability>(m_capability),
      m_nvidiaTelemetryQueryTimer, m_nvidiaTelemetryAvailable);

  if (res.rpmChanged) {
    m_currentRpm = res.rpm;
    emit currentRpmChanged();
  }
  if (res.speedPercentChanged) {
    m_currentFanSpeedPercent = res.speedPercent;
    emit currentFanSpeedPercentChanged();
  }
  if (res.supported) {
    setSupported(true);
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

    // Directional Hysteresis
    if (!isAuto && !force && m_lastEvaluatedTempC > 0 &&
        m_gpuTemperatureC < m_lastEvaluatedTempC) {
      if (m_gpuTemperatureC > (m_lastEvaluatedTempC - m_hysteresisTempC) &&
          !m_lastAppliedModeWasAuto && m_lastAppliedPercent > 0) {
        calculatedSpeed = std::max(calculatedSpeed, m_lastAppliedPercent);
      }
    }

    // Fan smoothing / step rate limiter
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
  const auto res = FanHardwareWriter::executeSetFanSpeed(
      percent, isAutoMode, m_controlSupported, m_hardwareType, m_fanCount,
      m_verifiedHwmonPwmPath, m_verifiedHwmonPwmEnablePath);

  if (res.controlSupportedChanged) {
    setControlSupported(res.newControlSupported);
  }
  if (res.capabilityChanged) {
    setCapability(static_cast<ControlCapability>(res.newCapability));
  }
  if (!res.statusMessage.isEmpty()) {
    setStatusMessage(res.statusMessage);
  }

  m_lastAppliedPercent = percent;
  m_lastAppliedModeWasAuto = isAutoMode;

  emit fanSpeedApplied(percent, res.success);
  return res.success;
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
  setStatusMessage(tr("This fan is monitored by firmware and cannot be "
                      "controlled by ro-Control."));
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
  const bool ok = FanProfileManager::exportProfile(
      name, filePath, fanMode(), m_manualFanSpeedPercent, m_thermalThresholdC,
      m_customCurve);
  emit profileExported(name, ok);
  return ok;
}

bool FanController::importProfile(const QString &filePath) {
  QString name;
  QString mode;
  int manualSpeed = 50;
  int thermalThresh = 85;
  QVector<FanCurvePoint> importedCurve;

  const bool ok = FanProfileManager::importProfile(
      filePath, name, mode, manualSpeed, thermalThresh, importedCurve);
  if (!ok) {
    emit profileImported(filePath, false);
    return false;
  }

  if (!importedCurve.isEmpty()) {
    m_customCurve = importedCurve;
    emit customCurvePointsChanged();
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
  return FanProfileManager::listSavedProfiles();
}
