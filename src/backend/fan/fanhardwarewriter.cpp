#include "fanhardwarewriter.h"
#include "fanhardwareprobe.h"
#include "system/commandrunner.h"
#include "system/polkit.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <algorithm>

FanHardwareWriter::WriteResult FanHardwareWriter::executeSetFanSpeed(
    int percent, bool isAutoMode, bool controlSupported,
    const QString &hardwareType, int fanCount,
    const QString &verifiedHwmonPwmPath,
    const QString &verifiedHwmonPwmEnablePath) {
  WriteResult result;

  if (!controlSupported) {
    result.success = false;
    return result;
  }

  const QString mockCap =
      QString::fromLatin1(qgetenv("RO_CONTROL_MOCK_FAN_CAPABILITY"))
          .trimmed()
          .toLower();
  if (mockCap == QStringLiteral("controllable") &&
      qEnvironmentVariableIsEmpty("RO_CONTROL_COMMAND_NVIDIA_SETTINGS")) {
    result.success = true;
    return result;
  }

  const QString nvidiaSettingsProg =
      CommandRunner::resolveProgramPath(QStringLiteral("nvidia-settings"));

  if (!nvidiaSettingsProg.isEmpty() &&
      hardwareType.contains(QStringLiteral("NVIDIA"))) {
    CommandRunner runner;
    CommandRunner::RunOptions options;
    options.timeoutMs = 2000;

    QStringList args;
    if (isAutoMode) {
      args << QStringLiteral("-a")
           << QStringLiteral("[gpu:0]/GPUFanControlState=0");
    } else {
      args << QStringLiteral("-a")
           << QStringLiteral("[gpu:0]/GPUFanControlState=1")
           << QStringLiteral("-a")
           << QStringLiteral("[fan:0]/GPUTargetFanSpeed=%1").arg(percent);
      for (int fanIndex = 1; fanIndex < fanCount; ++fanIndex) {
        args << QStringLiteral("-a")
             << QStringLiteral("[fan:%1]/GPUTargetFanSpeed=%2")
                    .arg(fanIndex)
                    .arg(percent);
      }
    }

    const auto cmdResult =
        runner.run(QStringLiteral("nvidia-settings"), args, options);
    const bool hasPermissionError =
        cmdResult.stdout.contains(QStringLiteral("permission"),
                                  Qt::CaseInsensitive) ||
        cmdResult.stderr.contains(QStringLiteral("permission"),
                                  Qt::CaseInsensitive) ||
        cmdResult.stdout.contains(QStringLiteral("Operation not permitted"),
                                  Qt::CaseInsensitive) ||
        cmdResult.stderr.contains(QStringLiteral("Operation not permitted"),
                                  Qt::CaseInsensitive) ||
        cmdResult.stdout.contains(QStringLiteral("ERROR:"),
                                  Qt::CaseInsensitive) ||
        cmdResult.stderr.contains(QStringLiteral("ERROR:"),
                                  Qt::CaseInsensitive);

    if (hasPermissionError) {
      result.success = false;
      result.statusMessage =
          QStringLiteral("NVIDIA fan control rejected by driver: Coolbits "
                         "option is required in Xorg configuration.");
      result.controlSupportedChanged = true;
      result.newControlSupported = false;
      result.capabilityChanged = true;
      result.newCapability = FanHardwareProbe::ProbeCapability::TelemetryOnly;
    } else {
      result.success = cmdResult.success();
    }
  } else if (!verifiedHwmonPwmPath.isEmpty()) {
    if (!verifiedHwmonPwmEnablePath.isEmpty() &&
        QFile::exists(verifiedHwmonPwmEnablePath)) {
      FanHardwareProbe::writeTextFile(verifiedHwmonPwmEnablePath,
                                      isAutoMode ? QStringLiteral("2")
                                                 : QStringLiteral("1"));
    }
    if (!isAutoMode && QFile::exists(verifiedHwmonPwmPath)) {
      const int rawPwm = std::clamp((percent * 255) / 100, 0, 255);
      if (FanHardwareProbe::writeTextFile(verifiedHwmonPwmPath,
                                          QString::number(rawPwm))) {
        result.success = true;
      }
    } else if (isAutoMode) {
      result.success = true;
    }
  }

  return result;
}

bool FanHardwareWriter::coolbitsEnabled() {
  const QString confPath =
      QStringLiteral("/etc/X11/xorg.conf.d/99-nvidia-coolbits.conf");
  if (QFile::exists(confPath)) {
    return true;
  }

  const QFileInfoList entries =
      QDir(QStringLiteral("/etc/X11/xorg.conf.d")).entryInfoList(QDir::Files);
  for (const auto &e : entries) {
    if (FanHardwareProbe::readTextFile(e.absoluteFilePath())
            .contains(QStringLiteral("Coolbits"), Qt::CaseInsensitive)) {
      return true;
    }
  }
  return false;
}

bool FanHardwareWriter::enableNvidiaCoolbits(QString &outStatusMessage) {
  PolkitHelper polkit;
  if (!polkit.isPkexecAvailable()) {
    outStatusMessage = QStringLiteral(
        "Polkit (pkexec) is not available to configure Coolbits.");
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
    outStatusMessage =
        QStringLiteral("Coolbits enabled successfully! A session restart or "
                       "reboot is required to activate manual fan control.");
    return true;
  }

  outStatusMessage =
      QStringLiteral("Failed to enable Coolbits: %1")
          .arg(result.stderr.isEmpty() ? result.stdout : result.stderr);
  return false;
}
