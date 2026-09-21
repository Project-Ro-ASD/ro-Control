#pragma once

#include <QFileInfoList>
#include <QList>
#include <QString>

class FanHardwareProbe {
public:
  enum class ProbeCapability {
    Unsupported,
    TelemetryOnly,
    Controllable,
    PermissionDenied,
    Unavailable,
    InitializationFailed
  };

  struct ProbeResult {
    bool supported = false;
    bool controlSupported = false;
    ProbeCapability capability = ProbeCapability::Unsupported;
    QString hardwareType = QStringLiteral("None");
    QString verifiedHwmonPwmPath;
    QString verifiedHwmonPwmEnablePath;
    QString statusMessage;
  };

  struct ExtraFanChannel {
    bool gpuHwmon = false;
    QString basePath;
    QString inputName;
    QString label;
    QString id;
  };

  struct HwmonTopology {
    QString coretempInput;
    QString acpitzInput;
    QString cpuFanRpmInput;
    QString sysFanRpmInput;
    QList<ExtraFanChannel> extraChannels;
    bool topologyProbed = false;
    bool extraChannelsProbed = false;

    void reset() {
      coretempInput.clear();
      acpitzInput.clear();
      cpuFanRpmInput.clear();
      sysFanRpmInput.clear();
      extraChannels.clear();
      topologyProbed = false;
      extraChannelsProbed = false;
    }
  };

  static QString fanSysfsRoot();
  static bool writeTextFile(const QString &path, const QString &content);
  static QString readTextFile(const QString &path);
  static bool isGpuHwmon(const QString &name);

  static ProbeResult detectHardwareCapabilities();
  static void probeHwmonTopology(HwmonTopology &topology);
  static void probeExtraChannels(HwmonTopology &topology);
};
