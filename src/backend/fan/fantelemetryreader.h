#pragma once

#include "fanhardwareprobe.h"
#include "fanprofilemanager.h"

#include <QElapsedTimer>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class FanTelemetryReader {
public:
  struct GpuTelemetryResult {
    int rpm = 0;
    int speedPercent = 0;
    bool supported = false;
    bool rpmChanged = false;
    bool speedPercentChanged = false;
  };

  static GpuTelemetryResult
  readGpuTelemetry(int currentRpm, int currentSpeedPercent,
                   FanHardwareProbe::ProbeCapability capability,
                   QElapsedTimer &nvidiaQueryTimer,
                   bool &nvidiaTelemetryAvailable);

  static QVariantList
  collectSystemFans(bool supported, bool controlSupported,
                    const QString &capabilityString, int currentFanSpeedPercent,
                    int currentRpm, int targetFanSpeedPercent,
                    int manualFanSpeedPercent, int thermalThresholdC,
                    const QString &fanMode, const QString &gpuDisplayName,
                    const QVariantList &customCurvePointsVariant,
                    int gpuTemperatureC, int cpuTemperatureC,
                    const FanProfileManager::SystemFanProfile &cpuProfile,
                    const FanProfileManager::SystemFanProfile &sysProfile,
                    FanHardwareProbe::HwmonTopology &topology);
};
