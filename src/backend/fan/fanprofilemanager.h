#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVector>
#include <algorithm>
#include <cmath>

struct FanCurvePoint {
  int temperatureC = 0;
  int fanSpeedPercent = 0;

  bool operator==(const FanCurvePoint &other) const {
    return temperatureC == other.temperatureC &&
           fanSpeedPercent == other.fanSpeedPercent;
  }
};

class FanProfileManager {
public:
  enum class FanMode { Auto, Silent, Balanced, Performance, Manual, Custom };

  struct SystemFanProfile {
    QString id;
    QString name;
    QString type;
    FanMode mode = FanMode::Auto;
    int manualSpeedPercent = 50;
    int thermalThresholdC = 85;
    QVector<FanCurvePoint> customCurve;
  };

  FanProfileManager();

  static QString modeToString(FanMode mode);
  static FanMode stringToMode(const QString &modeStr);

  static int calculateCurveFanSpeed(const QVector<FanCurvePoint> &curve,
                                    int temperatureC);
  static QVector<FanCurvePoint> defaultSilentCurve();
  static QVector<FanCurvePoint> defaultBalancedCurve();
  static QVector<FanCurvePoint> defaultPerformanceCurve();
  static QVector<FanCurvePoint> defaultCustomCurve();
  static QVector<FanCurvePoint> presetToCurve(const QString &presetName);

  void loadSettings(FanMode &mode, int &manualSpeed, int &thermalThreshold,
                    bool &smoothing, bool &hwSetupComplete, bool &batterySync,
                    int &rampUp, int &rampDown, int &hysteresis,
                    QString &gpuDisplayName,
                    QVector<FanCurvePoint> &customCurve,
                    SystemFanProfile &cpuProfile, SystemFanProfile &sysProfile);

  void saveSettings(FanMode mode, int manualSpeed, int thermalThreshold,
                    bool smoothing, bool hwSetupComplete, bool batterySync,
                    int rampUp, int rampDown, int hysteresis,
                    const QString &gpuDisplayName,
                    const QVector<FanCurvePoint> &customCurve,
                    const SystemFanProfile &cpuProfile,
                    const SystemFanProfile &sysProfile);

  static bool exportProfile(const QString &profileName, const QString &filePath,
                            const QString &fanMode, int manualSpeed,
                            int thermalThreshold,
                            const QVector<FanCurvePoint> &curve);

  static bool importProfile(const QString &filePath, QString &outProfileName,
                            QString &outFanMode, int &outManualSpeed,
                            int &outThermalThreshold,
                            QVector<FanCurvePoint> &outCurve);

  static QStringList listSavedProfiles();
};
