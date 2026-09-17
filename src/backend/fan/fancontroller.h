#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariantList>
#include <QVector>

struct FanCurvePoint {
  int temperatureC = 0;
  int fanSpeedPercent = 0;

  bool operator==(const FanCurvePoint &other) const {
    return temperatureC == other.temperatureC &&
           fanSpeedPercent == other.fanSpeedPercent;
  }
};

class FanController : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool supported READ supported NOTIFY supportedChanged)
  Q_PROPERTY(bool controlSupported READ controlSupported NOTIFY
                 controlSupportedChanged)
  Q_PROPERTY(bool running READ running NOTIFY runningChanged)
  Q_PROPERTY(int fanCount READ fanCount NOTIFY fanCountChanged)
  Q_PROPERTY(int currentFanSpeedPercent READ currentFanSpeedPercent NOTIFY
                 currentFanSpeedPercentChanged)
  Q_PROPERTY(int currentRpm READ currentRpm NOTIFY currentRpmChanged)
  Q_PROPERTY(int targetFanSpeedPercent READ targetFanSpeedPercent NOTIFY
                 targetFanSpeedPercentChanged)
  Q_PROPERTY(int manualFanSpeedPercent READ manualFanSpeedPercent WRITE
                 setManualFanSpeedPercent NOTIFY manualFanSpeedPercentChanged)
  Q_PROPERTY(
      QString fanMode READ fanMode WRITE setFanMode NOTIFY fanModeChanged)
  Q_PROPERTY(QStringList availableModes READ availableModes CONSTANT)
  Q_PROPERTY(bool safetyOverrideActive READ safetyOverrideActive NOTIFY
                 safetyOverrideActiveChanged)
  Q_PROPERTY(int thermalThresholdC READ thermalThresholdC NOTIFY
                 thermalThresholdCChanged)
  Q_PROPERTY(
      QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
  Q_PROPERTY(QString hardwareType READ hardwareType NOTIFY hardwareTypeChanged)
  Q_PROPERTY(
      QString capabilityString READ capabilityString NOTIFY capabilityChanged)
  Q_PROPERTY(QVariantList customCurvePoints READ customCurvePointsVariant NOTIFY
                 customCurvePointsChanged)
  Q_PROPERTY(
      int gpuTemperatureC READ gpuTemperatureC NOTIFY gpuTemperatureCChanged)
  Q_PROPERTY(
      int cpuTemperatureC READ cpuTemperatureC NOTIFY cpuTemperatureCChanged)
  Q_PROPERTY(QVariantList systemFans READ systemFans NOTIFY systemFansChanged)
  Q_PROPERTY(int systemFanCount READ systemFanCount NOTIFY systemFansChanged)
  Q_PROPERTY(int selectedFanIndex READ selectedFanIndex WRITE
                 setSelectedFanIndex NOTIFY selectedFanIndexChanged)
  Q_PROPERTY(QString selectedFanId READ selectedFanId WRITE setSelectedFanId
                 NOTIFY selectedFanIdChanged)
  Q_PROPERTY(
      bool coolbitsEnabled READ coolbitsEnabled NOTIFY coolbitsEnabledChanged)
  Q_PROPERTY(
      bool batteryProfileSyncEnabled READ batteryProfileSyncEnabled WRITE
          setBatteryProfileSyncEnabled NOTIFY batteryProfileSyncEnabledChanged)
  Q_PROPERTY(bool smoothingEnabled READ smoothingEnabled WRITE
                 setSmoothingEnabled NOTIFY smoothingEnabledChanged)
  Q_PROPERTY(bool hardwareSetupComplete READ hardwareSetupComplete NOTIFY
                 hardwareSetupCompleteChanged)
  Q_PROPERTY(int rampUpRatePercent READ rampUpRatePercent WRITE
                 setRampUpRatePercent NOTIFY rampRateChanged)
  Q_PROPERTY(int rampDownRatePercent READ rampDownRatePercent WRITE
                 setRampDownRatePercent NOTIFY rampRateChanged)
  Q_PROPERTY(int hysteresisTempC READ hysteresisTempC WRITE setHysteresisTempC
                 NOTIFY hysteresisTempCChanged)

public:
  enum class FanMode {
    Auto,        // VBIOS / Driver default
    Silent,      // Acoustic priority curve
    Balanced,    // Optimized balanced curve
    Performance, // Aggressive cooling curve
    Manual,      // User-defined fixed percentage
    Custom       // User-defined custom curve points
  };
  Q_ENUM(FanMode)

  enum class ControlCapability {
    Unsupported,
    TelemetryOnly,
    Controllable,
    PermissionDenied,
    Unavailable,
    InitializationFailed
  };
  Q_ENUM(ControlCapability)

  explicit FanController(QObject *parent = nullptr);
  ~FanController() override;

  bool supported() const;
  bool controlSupported() const;
  ControlCapability capability() const;
  QString capabilityString() const;
  QString hardwareType() const;
  bool running() const;
  int fanCount() const;
  int currentFanSpeedPercent() const;
  int currentRpm() const;
  int targetFanSpeedPercent() const;
  int manualFanSpeedPercent() const;
  QString fanMode() const;
  FanMode modeEnum() const;
  QStringList availableModes() const;
  bool safetyOverrideActive() const;
  int thermalThresholdC() const;
  QString statusMessage() const;
  QVariantList customCurvePointsVariant() const;
  QVector<FanCurvePoint> customCurvePoints() const;
  int gpuTemperatureC() const;
  int cpuTemperatureC() const;
  QVariantList systemFans() const;
  int systemFanCount() const;
  int selectedFanIndex() const;
  QString selectedFanId() const;
  bool coolbitsEnabled() const;
  bool batteryProfileSyncEnabled() const;
  void setBatteryProfileSyncEnabled(bool enabled);
  bool smoothingEnabled() const;
  bool hardwareSetupComplete() const;
  int rampUpRatePercent() const;
  int rampDownRatePercent() const;
  int hysteresisTempC() const;

  static QString modeToString(FanMode mode);
  static FanMode stringToMode(const QString &modeStr);
  static QString capabilityToString(ControlCapability cap);

  static int calculateCurveFanSpeed(const QVector<FanCurvePoint> &curve,
                                    int temperatureC);
  static QVector<FanCurvePoint> defaultSilentCurve();
  static QVector<FanCurvePoint> defaultBalancedCurve();
  static QVector<FanCurvePoint> defaultPerformanceCurve();
  static QVector<FanCurvePoint> defaultCustomCurve();

  Q_INVOKABLE void refresh();
  // Re-probes the real hardware topology. Used on first launch and by rescan.
  Q_INVOKABLE void runHardwareSetup();
  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();
  Q_INVOKABLE void setFanMode(const QString &mode);
  Q_INVOKABLE void setManualFanSpeedPercent(int percent);
  Q_INVOKABLE bool setCustomCurvePoint(int index, int tempC, int speedPercent);
  Q_INVOKABLE void resetCustomCurve();
  Q_INVOKABLE bool applyCurvePreset(const QString &presetName);
  Q_INVOKABLE QString cycleFanMode();
  Q_INVOKABLE void resetToAuto();
  Q_INVOKABLE void updateTemperature(int tempC);
  Q_INVOKABLE void updateCpuTemperature(int tempC);
  Q_INVOKABLE void updateGpuFanSpeed(int percent);
  Q_INVOKABLE void updateGpuThermalLimit(int limitC);
  Q_INVOKABLE void selectFan(int index);
  Q_INVOKABLE void selectFanById(const QString &id);
  Q_INVOKABLE void setSelectedFanIndex(int index);
  Q_INVOKABLE void setSelectedFanId(const QString &id);
  Q_INVOKABLE bool enableNvidiaCoolbits();
  Q_INVOKABLE void syncPowerSource(bool onBattery);
  Q_INVOKABLE void setSmoothingEnabled(bool enabled);
  Q_INVOKABLE void setRampUpRatePercent(int percent);
  Q_INVOKABLE void setRampDownRatePercent(int percent);
  Q_INVOKABLE void setHysteresisTempC(int degrees);

  // Profile JSON export / import API
  Q_INVOKABLE bool exportProfile(const QString &profileName,
                                 const QString &filePath = QString());
  Q_INVOKABLE bool importProfile(const QString &filePath);
  Q_INVOKABLE QStringList listSavedProfiles() const;

  // Per-fan management API for dedicated popup settings
  Q_INVOKABLE QVariantMap getFanConfig(const QString &fanId);
  Q_INVOKABLE bool setFanModeForFan(const QString &fanId, const QString &mode);
  Q_INVOKABLE bool setManualSpeedForFan(const QString &fanId, int percent);
  Q_INVOKABLE bool setCustomCurvePointForFan(const QString &fanId, int index,
                                             int tempC, int speedPercent);
  Q_INVOKABLE bool applyCurvePresetForFan(const QString &fanId,
                                          const QString &presetName);
  Q_INVOKABLE bool resetCustomCurveForFan(const QString &fanId);
  Q_INVOKABLE bool setThermalThresholdForFan(const QString &fanId, int tempC);
  Q_INVOKABLE bool resetFanToAuto(const QString &fanId);
  Q_INVOKABLE bool applyFanConfiguration(const QString &fanId);
  Q_INVOKABLE bool setFanDisplayName(const QString &fanId,
                                     const QString &displayName);
  // Runs a non-persistent hardware test. This never changes the saved profile.
  Q_INVOKABLE bool testFanSpeedForFan(const QString &fanId, int percent = 100);
  Q_INVOKABLE bool restoreFanControlForFan(const QString &fanId);

signals:
  void supportedChanged();
  void controlSupportedChanged();
  void capabilityChanged();
  void hardwareTypeChanged();
  void runningChanged();
  void fanCountChanged();
  void currentFanSpeedPercentChanged();
  void currentRpmChanged();
  void targetFanSpeedPercentChanged();
  void manualFanSpeedPercentChanged();
  void fanModeChanged();
  void safetyOverrideActiveChanged();
  void thermalThresholdCChanged();
  void statusMessageChanged();
  void customCurvePointsChanged();
  void gpuTemperatureCChanged();
  void cpuTemperatureCChanged();
  void fanSpeedApplied(int targetPercent, bool success);
  void systemFansChanged();
  void selectedFanIndexChanged();
  void selectedFanIdChanged();
  void coolbitsEnabledChanged();
  void batteryProfileSyncEnabledChanged();
  void smoothingEnabledChanged();
  void hardwareSetupCompleteChanged();
  void rampRateChanged();
  void hysteresisTempCChanged();
  void profileExported(const QString &profileName, bool success);
  void profileImported(const QString &profileName, bool success);

private:
  void loadSettings();
  void saveSettings();
  void detectHardwareCapabilities(bool force = false);
  void evaluateAndApplyFanSpeed(bool force = false);
  bool executeSetFanSpeed(int percent, bool isAutoMode);
  void readCurrentFanTelemetry();
  void updateSystemFansTelemetry();
  void setSupported(bool value);
  void setControlSupported(bool value);
  void setCapability(ControlCapability cap);
  void setHardwareType(const QString &hwType);
  void setStatusMessage(const QString &msg);

  QTimer m_timer;
  bool m_capabilitiesDetected = false;
  bool m_supported = false;
  bool m_controlSupported = false;
  ControlCapability m_capability = ControlCapability::Unsupported;
  QString m_hardwareType = QStringLiteral("None");
  int m_fanCount = 1;
  int m_currentFanSpeedPercent = 0;
  int m_currentRpm = 0;
  int m_targetFanSpeedPercent = 0;
  int m_manualFanSpeedPercent = 50;
  FanMode m_mode = FanMode::Auto;
  bool m_safetyOverrideActive = false;
  int m_thermalThresholdC = 85;
  int m_thermalRecoveryMarginC = 5;
  int m_hysteresisTempC = 2;
  int m_lastEvaluatedTempC = -1;
  bool m_smoothingEnabled = false;
  int m_rampUpRatePercent = 20;
  int m_rampDownRatePercent = 5;
  QString m_statusMessage;
  QVector<FanCurvePoint> m_customCurve;
  int m_gpuTemperatureC = 0;
  int m_cpuTemperatureC = 0;
  int m_lastAppliedPercent = -1;
  bool m_lastAppliedModeWasAuto = true;
  QElapsedTimer m_nvidiaTelemetryQueryTimer;
  bool m_nvidiaTelemetryAvailable = true;
  QString m_verifiedHwmonPwmPath;
  QString m_verifiedHwmonPwmEnablePath;
  struct SystemFanProfile {
    QString id;
    QString name;
    QString type;
    FanMode mode = FanMode::Auto;
    int manualSpeedPercent = 50;
    int thermalThresholdC = 85;
    QVector<FanCurvePoint> customCurve;
  };

  SystemFanProfile m_cpuProfile;
  SystemFanProfile m_sysProfile;
  QString m_gpuDisplayName;
  QVariantList m_systemFans;
  int m_selectedFanIndex = 0;
  QString m_selectedFanId = QStringLiteral("gpu_0");
  bool m_batteryProfileSyncEnabled = false;
  bool m_hardwareSetupComplete = false;
  QString m_preBatteryFanMode;
};
