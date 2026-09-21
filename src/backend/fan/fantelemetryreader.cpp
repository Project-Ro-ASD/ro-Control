#include "fantelemetryreader.h"
#include "nvidia/detector.h"
#include "system/commandrunner.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <algorithm>

namespace {
QString tr(const char *text) {
  return QCoreApplication::translate("FanController", text);
}
} // namespace

FanTelemetryReader::GpuTelemetryResult FanTelemetryReader::readGpuTelemetry(
    int currentRpm, int currentSpeedPercent,
    FanHardwareProbe::ProbeCapability capability,
    QElapsedTimer &nvidiaQueryTimer, bool &nvidiaTelemetryAvailable) {
  GpuTelemetryResult result;
  result.rpm = currentRpm;
  result.speedPercent = currentSpeedPercent;

  const QString mockRpm =
      qEnvironmentVariable("RO_CONTROL_MOCK_FAN_RPM").trimmed();
  if (!mockRpm.isEmpty()) {
    bool ok = false;
    const int rpm = mockRpm.toInt(&ok);
    if (ok && rpm >= 0) {
      if (result.rpm != rpm) {
        result.rpm = rpm;
        result.rpmChanged = true;
      }
      result.supported = true;
      return result;
    }
  }

  if (capability == FanHardwareProbe::ProbeCapability::Unsupported) {
    return result;
  }

  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 500;

  const QString nvidiaSettingsProg =
      CommandRunner::resolveProgramPath(QStringLiteral("nvidia-settings"));
  constexpr qint64 kNvidiaTelemetrySuccessIntervalMs = 5000;
  constexpr qint64 kNvidiaTelemetryFailureIntervalMs = 30000;
  const qint64 retryIntervalMs = nvidiaTelemetryAvailable
                                     ? kNvidiaTelemetrySuccessIntervalMs
                                     : kNvidiaTelemetryFailureIntervalMs;
  const bool shouldQueryNvidiaTelemetry =
      !nvidiaSettingsProg.isEmpty() &&
      (!nvidiaQueryTimer.isValid() ||
       nvidiaQueryTimer.elapsed() >= retryIntervalMs);
  if (shouldQueryNvidiaTelemetry) {
    nvidiaQueryTimer.restart();
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
        if (result.rpm != rpm) {
          result.rpm = rpm;
          result.rpmChanged = true;
        }
        result.supported = true;
      }
    }
    nvidiaTelemetryAvailable = rpmRes.success();
  }

  // Sysfs hwmon fan input fallback
  const QFileInfoList hwmonEntries =
      QDir(FanHardwareProbe::fanSysfsRoot())
          .entryInfoList({QStringLiteral("hwmon*")},
                         QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

  for (const QFileInfo &entry : hwmonEntries) {
    const QString path = entry.absoluteFilePath();
    const QString fanInputPath = path + QStringLiteral("/fan1_input");
    if (QFile::exists(fanInputPath)) {
      bool ok = false;
      const int rpm = FanHardwareProbe::readTextFile(fanInputPath).toInt(&ok);
      if (ok && rpm >= 0) {
        if (result.rpm != rpm) {
          result.rpm = rpm;
          result.rpmChanged = true;
        }
        result.supported = true;
      }
    }

    const QString pwmPath = path + QStringLiteral("/pwm1");
    if (QFile::exists(pwmPath) && result.speedPercent == 0) {
      bool ok = false;
      const int rawPwm = FanHardwareProbe::readTextFile(pwmPath).toInt(&ok);
      if (ok && rawPwm >= 0) {
        const int pct =
            std::clamp(static_cast<int>((rawPwm * 100) / 255), 0, 100);
        if (result.speedPercent != pct) {
          result.speedPercent = pct;
          result.speedPercentChanged = true;
        }
        result.supported = true;
      }
    }
  }

  return result;
}

QVariantList FanTelemetryReader::collectSystemFans(
    bool supported, bool controlSupported, const QString &capabilityString,
    int currentFanSpeedPercent, int currentRpm, int targetFanSpeedPercent,
    int manualFanSpeedPercent, int thermalThresholdC, const QString &fanMode,
    const QString &gpuDisplayName, const QVariantList &customCurvePointsVariant,
    int gpuTemperatureC, int cpuTemperatureC,
    const FanProfileManager::SystemFanProfile &cpuProfile,
    const FanProfileManager::SystemFanProfile &sysProfile,
    FanHardwareProbe::HwmonTopology &topology) {
  QVariantList fanList;

  if (supported) {
    const bool isZeroRpm = (currentRpm == 0 && currentFanSpeedPercent == 0);
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
    gpuFan.insert(QStringLiteral("name"),
                  gpuDisplayName.isEmpty() ? fanDisplayName : gpuDisplayName);
    gpuFan.insert(QStringLiteral("type"), QStringLiteral("GPU"));
    gpuFan.insert(QStringLiteral("speedPercent"), currentFanSpeedPercent);
    gpuFan.insert(QStringLiteral("rpm"), currentRpm);
    gpuFan.insert(QStringLiteral("isZeroRpm"), isZeroRpm);
    gpuFan.insert(QStringLiteral("temperatureC"), gpuTemperatureC);
    gpuFan.insert(QStringLiteral("targetSpeedPercent"), targetFanSpeedPercent);
    gpuFan.insert(QStringLiteral("manualSpeedPercent"), manualFanSpeedPercent);
    gpuFan.insert(QStringLiteral("thermalThresholdC"), thermalThresholdC);
    gpuFan.insert(QStringLiteral("controllable"), controlSupported);
    gpuFan.insert(QStringLiteral("telemetryAvailable"), supported);
    gpuFan.insert(QStringLiteral("speedAvailable"), supported);
    gpuFan.insert(QStringLiteral("customCurvePoints"),
                  customCurvePointsVariant);
    gpuFan.insert(QStringLiteral("statusLabel"),
                  controlSupported ? tr("Controllable")
                  : isZeroRpm      ? tr("0 RPM (Silent)")
                                   : tr("Active (Auto)"));
    gpuFan.insert(QStringLiteral("capability"), capabilityString);
    gpuFan.insert(QStringLiteral("capabilityReason"),
                  controlSupported
                      ? tr("Direct hardware fan control active via NV-CONTROL.")
                  : isZeroRpm
                      ? tr("GPU is in 0 RPM silent mode (temperature < 50°C). "
                           "Fans automatically spin up under load.")
                      : tr("Automatic VBIOS cooling curve active."));
    gpuFan.insert(QStringLiteral("mode"), fanMode);
    fanList.append(gpuFan);
  }

  FanHardwareProbe::probeHwmonTopology(topology);
  FanHardwareProbe::probeExtraChannels(topology);

  int cpuTemp = cpuTemperatureC;
  int cpuRpm = 0;
  int ambientTemp = 0;

  if (!topology.coretempInput.isEmpty()) {
    bool ok = false;
    const int tVal =
        FanHardwareProbe::readTextFile(topology.coretempInput).toInt(&ok) /
        1000;
    if (ok && tVal > 0 && tVal < 115) {
      cpuTemp = tVal;
    }
  }
  if (!topology.acpitzInput.isEmpty()) {
    bool ok = false;
    const int tVal =
        FanHardwareProbe::readTextFile(topology.acpitzInput).toInt(&ok) / 1000;
    if (ok && tVal > 0 && tVal < 115) {
      ambientTemp = tVal;
    }
  }
  if (!topology.cpuFanRpmInput.isEmpty()) {
    bool ok = false;
    const int rpm =
        FanHardwareProbe::readTextFile(topology.cpuFanRpmInput).toInt(&ok);
    if (ok && rpm >= 0) {
      cpuRpm = std::clamp(rpm, 0, 100000);
    }
  }

  const bool cpuTelemetryAvailable = !topology.cpuFanRpmInput.isEmpty();
  int cpuSpeedPct = 0;
  if (cpuTelemetryAvailable) {
    switch (cpuProfile.mode) {
    case FanProfileManager::FanMode::Manual:
      cpuSpeedPct = cpuProfile.manualSpeedPercent;
      break;
    case FanProfileManager::FanMode::Silent:
      cpuSpeedPct = FanProfileManager::calculateCurveFanSpeed(
          FanProfileManager::defaultSilentCurve(), cpuTemp);
      break;
    case FanProfileManager::FanMode::Balanced:
      cpuSpeedPct = FanProfileManager::calculateCurveFanSpeed(
          FanProfileManager::defaultBalancedCurve(), cpuTemp);
      break;
    case FanProfileManager::FanMode::Performance:
      cpuSpeedPct = FanProfileManager::calculateCurveFanSpeed(
          FanProfileManager::defaultPerformanceCurve(), cpuTemp);
      break;
    case FanProfileManager::FanMode::Custom:
      cpuSpeedPct = FanProfileManager::calculateCurveFanSpeed(
          cpuProfile.customCurve, cpuTemp);
      break;
    case FanProfileManager::FanMode::Auto:
    default:
      cpuSpeedPct = std::clamp(28 + ((cpuTemp - 30) * 8) / 5, 25, 100);
      break;
    }
  }

  QVariantList cpuCurvePoints;
  for (const auto &pt : cpuProfile.customCurve) {
    QVariantMap map;
    map.insert(QStringLiteral("temp"), pt.temperatureC);
    map.insert(QStringLiteral("speed"), pt.fanSpeedPercent);
    cpuCurvePoints.append(map);
  }

  QVariantMap cpuFan;
  cpuFan.insert(QStringLiteral("id"), QStringLiteral("cpu_fan_0"));
  cpuFan.insert(QStringLiteral("name"),
                cpuProfile.name.isEmpty()
                    ? QStringLiteral("Intel CPU Cooler Fan")
                    : cpuProfile.name);
  cpuFan.insert(QStringLiteral("type"), QStringLiteral("CPU"));
  cpuFan.insert(QStringLiteral("speedPercent"), cpuSpeedPct);
  cpuFan.insert(QStringLiteral("rpm"), cpuRpm);
  cpuFan.insert(QStringLiteral("isZeroRpm"), false);
  cpuFan.insert(QStringLiteral("temperatureC"), cpuTemp);
  cpuFan.insert(QStringLiteral("targetSpeedPercent"), cpuSpeedPct);
  cpuFan.insert(QStringLiteral("manualSpeedPercent"),
                cpuProfile.manualSpeedPercent);
  cpuFan.insert(QStringLiteral("thermalThresholdC"),
                cpuProfile.thermalThresholdC);
  cpuFan.insert(QStringLiteral("controllable"), false);
  cpuFan.insert(QStringLiteral("telemetryAvailable"), cpuTelemetryAvailable);
  cpuFan.insert(QStringLiteral("speedAvailable"), false);
  cpuFan.insert(QStringLiteral("customCurvePoints"), cpuCurvePoints);
  cpuFan.insert(QStringLiteral("statusLabel"),
                cpuProfile.mode == FanProfileManager::FanMode::Auto
                    ? QStringLiteral("Active (BIOS Auto)")
                    : QStringLiteral("Profile (%1)")
                          .arg(FanProfileManager::modeToString(cpuProfile.mode)
                                   .toUpper()));
  cpuFan.insert(QStringLiteral("capability"),
                QStringLiteral("hardware_managed"));
  cpuFan.insert(QStringLiteral("capabilityReason"),
                tr("Hardware BIOS thermal curve active with dynamic acoustic "
                   "regulation."));
  cpuFan.insert(QStringLiteral("mode"),
                FanProfileManager::modeToString(cpuProfile.mode));
  if (cpuTelemetryAvailable) {
    fanList.append(cpuFan);
  }

  int sysRpm = 0;
  if (!topology.sysFanRpmInput.isEmpty()) {
    bool ok = false;
    const int rpm =
        FanHardwareProbe::readTextFile(topology.sysFanRpmInput).toInt(&ok);
    if (ok && rpm >= 0) {
      sysRpm = std::clamp(rpm, 0, 100000);
    }
  }
  const bool sysTelemetryAvailable = !topology.sysFanRpmInput.isEmpty();
  int sysSpeedPct = 0;
  if (sysTelemetryAvailable) {
    switch (sysProfile.mode) {
    case FanProfileManager::FanMode::Manual:
      sysSpeedPct = sysProfile.manualSpeedPercent;
      break;
    case FanProfileManager::FanMode::Silent:
      sysSpeedPct = FanProfileManager::calculateCurveFanSpeed(
          FanProfileManager::defaultSilentCurve(), ambientTemp);
      break;
    case FanProfileManager::FanMode::Balanced:
      sysSpeedPct = FanProfileManager::calculateCurveFanSpeed(
          FanProfileManager::defaultBalancedCurve(), ambientTemp);
      break;
    case FanProfileManager::FanMode::Performance:
      sysSpeedPct = FanProfileManager::calculateCurveFanSpeed(
          FanProfileManager::defaultPerformanceCurve(), ambientTemp);
      break;
    case FanProfileManager::FanMode::Custom:
      sysSpeedPct = FanProfileManager::calculateCurveFanSpeed(
          sysProfile.customCurve, ambientTemp);
      break;
    case FanProfileManager::FanMode::Auto:
    default:
      sysSpeedPct = 0;
      break;
    }
  }

  QVariantList sysCurvePoints;
  for (const auto &pt : sysProfile.customCurve) {
    QVariantMap map;
    map.insert(QStringLiteral("temp"), pt.temperatureC);
    map.insert(QStringLiteral("speed"), pt.fanSpeedPercent);
    sysCurvePoints.append(map);
  }

  QVariantMap chassisFan;
  chassisFan.insert(QStringLiteral("id"), QStringLiteral("sys_fan_0"));
  chassisFan.insert(QStringLiteral("name"),
                    sysProfile.name.isEmpty()
                        ? QStringLiteral("Chassis Airflow Fan")
                        : sysProfile.name);
  chassisFan.insert(QStringLiteral("type"), QStringLiteral("SYS"));
  chassisFan.insert(QStringLiteral("speedPercent"), sysSpeedPct);
  chassisFan.insert(QStringLiteral("rpm"), sysRpm);
  chassisFan.insert(QStringLiteral("isZeroRpm"), false);
  chassisFan.insert(QStringLiteral("temperatureC"), ambientTemp);
  chassisFan.insert(QStringLiteral("targetSpeedPercent"), sysSpeedPct);
  chassisFan.insert(QStringLiteral("manualSpeedPercent"),
                    sysProfile.manualSpeedPercent);
  chassisFan.insert(QStringLiteral("thermalThresholdC"),
                    sysProfile.thermalThresholdC);
  chassisFan.insert(QStringLiteral("controllable"), false);
  chassisFan.insert(QStringLiteral("telemetryAvailable"),
                    sysTelemetryAvailable);
  chassisFan.insert(QStringLiteral("speedAvailable"), false);
  chassisFan.insert(QStringLiteral("customCurvePoints"), sysCurvePoints);
  chassisFan.insert(
      QStringLiteral("statusLabel"),
      sysProfile.mode == FanProfileManager::FanMode::Auto
          ? QStringLiteral("Active (Auto)")
          : QStringLiteral("Profile (%1)")
                .arg(FanProfileManager::modeToString(sysProfile.mode)
                         .toUpper()));
  chassisFan.insert(QStringLiteral("capability"),
                    QStringLiteral("hardware_managed"));
  chassisFan.insert(QStringLiteral("capabilityReason"),
                    tr("Motherboard chassis airflow management curve active."));
  chassisFan.insert(QStringLiteral("mode"),
                    FanProfileManager::modeToString(sysProfile.mode));
  if (sysTelemetryAvailable) {
    fanList.append(chassisFan);
  }

  for (const auto &channel : std::as_const(topology.extraChannels)) {
    bool ok = false;
    const int rpm = FanHardwareProbe::readTextFile(
                        channel.basePath + QLatin1Char('/') +
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
                       : type == QStringLiteral("GPU") ? gpuTemperatureC
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

  return fanList;
}
