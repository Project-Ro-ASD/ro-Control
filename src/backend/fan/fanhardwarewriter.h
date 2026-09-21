#pragma once

#include "fanhardwareprobe.h"
#include <QString>

class FanHardwareWriter {
public:
  struct WriteResult {
    bool success = false;
    bool controlSupportedChanged = false;
    bool newControlSupported = false;
    bool capabilityChanged = false;
    FanHardwareProbe::ProbeCapability newCapability =
        FanHardwareProbe::ProbeCapability::Unsupported;
    QString statusMessage;
  };

  static WriteResult
  executeSetFanSpeed(int percent, bool isAutoMode, bool controlSupported,
                     const QString &hardwareType, int fanCount,
                     const QString &verifiedHwmonPwmPath,
                     const QString &verifiedHwmonPwmEnablePath);

  static bool coolbitsEnabled();
  static bool enableNvidiaCoolbits(QString &outStatusMessage);
};
