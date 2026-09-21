#include "fanprofilemanager.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>

FanProfileManager::FanProfileManager() = default;

QString FanProfileManager::modeToString(FanMode mode) {
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

FanProfileManager::FanMode
FanProfileManager::stringToMode(const QString &modeStr) {
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

int FanProfileManager::calculateCurveFanSpeed(
    const QVector<FanCurvePoint> &curve, int temperatureC) {
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

QVector<FanCurvePoint> FanProfileManager::defaultSilentCurve() {
  return {{40, 0}, {55, 30}, {68, 50}, {78, 75}, {85, 100}};
}

QVector<FanCurvePoint> FanProfileManager::defaultBalancedCurve() {
  return {{40, 30}, {55, 45}, {68, 65}, {78, 85}, {85, 100}};
}

QVector<FanCurvePoint> FanProfileManager::defaultPerformanceCurve() {
  return {{35, 45}, {50, 65}, {65, 80}, {75, 90}, {82, 100}};
}

QVector<FanCurvePoint> FanProfileManager::defaultCustomCurve() {
  return {{40, 30}, {55, 50}, {70, 70}, {85, 100}};
}

QVector<FanCurvePoint>
FanProfileManager::presetToCurve(const QString &presetName) {
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
    return defaultSilentCurve();
  }
  if (lower == QStringLiteral("performance")) {
    return defaultPerformanceCurve();
  }
  return defaultBalancedCurve();
}

void FanProfileManager::loadSettings(FanMode &mode, int &manualSpeed,
                                     int &thermalThreshold, bool &smoothing,
                                     bool &hwSetupComplete, bool &batterySync,
                                     int &rampUp, int &rampDown,
                                     int &hysteresis, QString &gpuDisplayName,
                                     QVector<FanCurvePoint> &customCurve,
                                     SystemFanProfile &cpuProfile,
                                     SystemFanProfile &sysProfile) {
  QSettings settings;
  settings.beginGroup(QStringLiteral("FanControl"));

  const QString savedMode =
      settings.value(QStringLiteral("mode"), QStringLiteral("auto")).toString();
  mode = stringToMode(savedMode);

  manualSpeed = settings.value(QStringLiteral("manualSpeed"), 50).toInt();
  manualSpeed = std::clamp(manualSpeed, 0, 100);

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
      customCurve = loadedCurve;
    }
  } else {
    customCurve = defaultCustomCurve();
  }

  thermalThreshold =
      settings.value(QStringLiteral("thermalThreshold"), 85).toInt();
  thermalThreshold = std::clamp(thermalThreshold, 60, 105);

  smoothing =
      settings.value(QStringLiteral("smoothingEnabled"), false).toBool();
  hwSetupComplete =
      settings.value(QStringLiteral("hardwareSetupComplete"), false).toBool();
  batterySync =
      settings.value(QStringLiteral("batteryProfileSyncEnabled"), false)
          .toBool();
  rampUp = std::clamp(settings.value(QStringLiteral("rampUpRate"), 20).toInt(),
                      1, 100);
  rampDown = std::clamp(
      settings.value(QStringLiteral("rampDownRate"), 5).toInt(), 1, 100);
  hysteresis = std::clamp(
      settings.value(QStringLiteral("hysteresisTempC"), 2).toInt(), 0, 15);
  gpuDisplayName =
      settings.value(QStringLiteral("displayName")).toString().trimmed();

  settings.endGroup();

  // CPU Profile initialization & loading
  cpuProfile.id = QStringLiteral("cpu_fan_0");
  cpuProfile.name = QStringLiteral("Intel CPU Cooler Fan");
  cpuProfile.type = QStringLiteral("CPU");
  cpuProfile.customCurve = defaultBalancedCurve();
  settings.beginGroup(QStringLiteral("FanControl_cpu"));
  cpuProfile.mode = stringToMode(
      settings.value(QStringLiteral("mode"), QStringLiteral("auto"))
          .toString());
  cpuProfile.manualSpeedPercent = std::clamp(
      settings.value(QStringLiteral("manualSpeed"), 50).toInt(), 0, 100);
  cpuProfile.thermalThresholdC = std::clamp(
      settings.value(QStringLiteral("thermalThreshold"), 90).toInt(), 60, 105);
  cpuProfile.name =
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
      cpuProfile.customCurve = loadedCurve;
    }
  }
  settings.endGroup();

  // SYS Profile initialization & loading
  sysProfile.id = QStringLiteral("sys_fan_0");
  sysProfile.name = QStringLiteral("Chassis Airflow Fan");
  sysProfile.type = QStringLiteral("SYS");
  sysProfile.customCurve = defaultSilentCurve();
  settings.beginGroup(QStringLiteral("FanControl_sys"));
  sysProfile.mode = stringToMode(
      settings.value(QStringLiteral("mode"), QStringLiteral("auto"))
          .toString());
  sysProfile.manualSpeedPercent = std::clamp(
      settings.value(QStringLiteral("manualSpeed"), 35).toInt(), 0, 100);
  sysProfile.thermalThresholdC = std::clamp(
      settings.value(QStringLiteral("thermalThreshold"), 75).toInt(), 50, 95);
  sysProfile.name =
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
      sysProfile.customCurve = loadedCurve;
    }
  }
  settings.endGroup();
}

void FanProfileManager::saveSettings(FanMode mode, int manualSpeed,
                                     int thermalThreshold, bool smoothing,
                                     bool hwSetupComplete, bool batterySync,
                                     int rampUp, int rampDown, int hysteresis,
                                     const QString &gpuDisplayName,
                                     const QVector<FanCurvePoint> &customCurve,
                                     const SystemFanProfile &cpuProfile,
                                     const SystemFanProfile &sysProfile) {
  QSettings settings;
  settings.beginGroup(QStringLiteral("FanControl"));

  settings.setValue(QStringLiteral("mode"), modeToString(mode));
  settings.setValue(QStringLiteral("manualSpeed"), manualSpeed);
  settings.setValue(QStringLiteral("thermalThreshold"), thermalThreshold);
  settings.setValue(QStringLiteral("smoothingEnabled"), smoothing);
  settings.setValue(QStringLiteral("hardwareSetupComplete"), hwSetupComplete);
  settings.setValue(QStringLiteral("batteryProfileSyncEnabled"), batterySync);
  settings.setValue(QStringLiteral("rampUpRate"), rampUp);
  settings.setValue(QStringLiteral("rampDownRate"), rampDown);
  settings.setValue(QStringLiteral("hysteresisTempC"), hysteresis);
  settings.setValue(QStringLiteral("displayName"), gpuDisplayName);
  settings.setValue(QStringLiteral("curveCount"), customCurve.size());

  for (qsizetype i = 0; i < customCurve.size(); ++i) {
    settings.setValue(QStringLiteral("curveTemp_%1").arg(i),
                      customCurve.at(i).temperatureC);
    settings.setValue(QStringLiteral("curveSpeed_%1").arg(i),
                      customCurve.at(i).fanSpeedPercent);
  }
  settings.endGroup();

  // CPU
  settings.beginGroup(QStringLiteral("FanControl_cpu"));
  settings.setValue(QStringLiteral("mode"), modeToString(cpuProfile.mode));
  settings.setValue(QStringLiteral("manualSpeed"),
                    cpuProfile.manualSpeedPercent);
  settings.setValue(QStringLiteral("thermalThreshold"),
                    cpuProfile.thermalThresholdC);
  settings.setValue(QStringLiteral("displayName"), cpuProfile.name);
  settings.setValue(QStringLiteral("curveCount"),
                    cpuProfile.customCurve.size());
  for (qsizetype i = 0; i < cpuProfile.customCurve.size(); ++i) {
    settings.setValue(QStringLiteral("curveTemp_%1").arg(i),
                      cpuProfile.customCurve.at(i).temperatureC);
    settings.setValue(QStringLiteral("curveSpeed_%1").arg(i),
                      cpuProfile.customCurve.at(i).fanSpeedPercent);
  }
  settings.endGroup();

  // SYS
  settings.beginGroup(QStringLiteral("FanControl_sys"));
  settings.setValue(QStringLiteral("mode"), modeToString(sysProfile.mode));
  settings.setValue(QStringLiteral("manualSpeed"),
                    sysProfile.manualSpeedPercent);
  settings.setValue(QStringLiteral("thermalThreshold"),
                    sysProfile.thermalThresholdC);
  settings.setValue(QStringLiteral("displayName"), sysProfile.name);
  settings.setValue(QStringLiteral("curveCount"),
                    sysProfile.customCurve.size());
  for (qsizetype i = 0; i < sysProfile.customCurve.size(); ++i) {
    settings.setValue(QStringLiteral("curveTemp_%1").arg(i),
                      sysProfile.customCurve.at(i).temperatureC);
    settings.setValue(QStringLiteral("curveSpeed_%1").arg(i),
                      sysProfile.customCurve.at(i).fanSpeedPercent);
  }
  settings.endGroup();
}

bool FanProfileManager::exportProfile(const QString &profileName,
                                      const QString &filePath,
                                      const QString &fanMode, int manualSpeed,
                                      int thermalThreshold,
                                      const QVector<FanCurvePoint> &curve) {
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
  rootObj[QStringLiteral("fanMode")] = fanMode;
  rootObj[QStringLiteral("manualSpeed")] = manualSpeed;
  rootObj[QStringLiteral("thermalThreshold")] = thermalThreshold;

  QJsonArray curveArray;
  for (const FanCurvePoint &pt : curve) {
    QJsonObject ptObj;
    ptObj[QStringLiteral("temp")] = pt.temperatureC;
    ptObj[QStringLiteral("speed")] = pt.fanSpeedPercent;
    curveArray.append(ptObj);
  }
  rootObj[QStringLiteral("curve")] = curveArray;

  QFile file(targetPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }

  const QJsonDocument doc(rootObj);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();
  return true;
}

bool FanProfileManager::importProfile(const QString &filePath,
                                      QString &outProfileName,
                                      QString &outFanMode, int &outManualSpeed,
                                      int &outThermalThreshold,
                                      QVector<FanCurvePoint> &outCurve) {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }

  const QByteArray data = file.readAll();
  file.close();

  QJsonParseError parseError;
  const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
  if (doc.isNull() || !doc.isObject()) {
    return false;
  }

  const QJsonObject rootObj = doc.object();
  outProfileName = rootObj.value(QStringLiteral("profileName")).toString();
  outFanMode = rootObj.value(QStringLiteral("fanMode")).toString();
  outManualSpeed = rootObj.value(QStringLiteral("manualSpeed")).toInt(50);
  outThermalThreshold = std::clamp(
      rootObj.value(QStringLiteral("thermalThreshold")).toInt(85), 60, 95);

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
      outCurve = importedCurve;
    }
  }

  return true;
}

QStringList FanProfileManager::listSavedProfiles() {
  const QString baseDir =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
      QStringLiteral("/profiles");
  QDir dir(baseDir);
  if (!dir.exists()) {
    return {};
  }
  return dir.entryList({QStringLiteral("*.json")}, QDir::Files, QDir::Name);
}
