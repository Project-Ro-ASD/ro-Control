#include "fanhardwareprobe.h"
#include "system/commandrunner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

QString FanHardwareProbe::fanSysfsRoot() {
  const QString overridePath =
      qEnvironmentVariable("RO_CONTROL_FAN_SYSFS_ROOT").trimmed();
  return overridePath.isEmpty() ? QStringLiteral("/sys/class/hwmon")
                                : overridePath;
}

bool FanHardwareProbe::writeTextFile(const QString &path,
                                     const QString &content) {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }
  return file.write(content.toUtf8()) != -1;
}

QString FanHardwareProbe::readTextFile(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }
  return QString::fromUtf8(file.readAll()).trimmed();
}

bool FanHardwareProbe::isGpuHwmon(const QString &name) {
  const QString lower = name.trimmed().toLower();
  return lower.contains(QStringLiteral("nvidia")) ||
         lower.contains(QStringLiteral("amdgpu")) ||
         lower.contains(QStringLiteral("radeon")) ||
         lower.contains(QStringLiteral("gpu"));
}

FanHardwareProbe::ProbeResult FanHardwareProbe::detectHardwareCapabilities() {
  ProbeResult result;

  const QString mockCap = qEnvironmentVariable("RO_CONTROL_MOCK_FAN_CAPABILITY")
                              .trimmed()
                              .toLower();
  if (!mockCap.isEmpty()) {
    if (mockCap == QStringLiteral("controllable")) {
      result.supported = true;
      result.controlSupported = true;
      result.capability = ProbeCapability::Controllable;
      result.hardwareType = QStringLiteral("NVIDIA (NV-CONTROL)");
      return result;
    }
    if (mockCap == QStringLiteral("telemetry_only")) {
      result.supported = true;
      result.controlSupported = false;
      result.capability = ProbeCapability::TelemetryOnly;
      result.hardwareType = QStringLiteral("NVIDIA (Telemetry Only)");
      return result;
    }
    if (mockCap == QStringLiteral("permission_denied")) {
      result.supported = true;
      result.controlSupported = false;
      result.capability = ProbeCapability::PermissionDenied;
      result.hardwareType = QStringLiteral("Linux HWMON (sysfs)");
      return result;
    }
    if (mockCap == QStringLiteral("unsupported")) {
      result.supported = false;
      result.controlSupported = false;
      result.capability = ProbeCapability::Unsupported;
      result.hardwareType = QStringLiteral("None");
      return result;
    }
  }

  const bool hasSysfsOverride =
      !qEnvironmentVariable("RO_CONTROL_FAN_SYSFS_ROOT").trimmed().isEmpty();

  if (!hasSysfsOverride) {
    const QString nvidiaSettingsProg =
        CommandRunner::resolveProgramPath(QStringLiteral("nvidia-settings"));
    if (!nvidiaSettingsProg.isEmpty()) {
      CommandRunner runner;
      CommandRunner::RunOptions testOpts;
      testOpts.timeoutMs = 1500;
      const auto testRes = runner.run(
          QStringLiteral("nvidia-settings"),
          {QStringLiteral("-q"), QStringLiteral("[gpu:0]/GPUFanControlState"),
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
        result.supported = true;
        result.controlSupported = false;
        result.capability = ProbeCapability::TelemetryOnly;
        result.hardwareType = QStringLiteral("NVIDIA (Telemetry Only)");
        result.statusMessage =
            QStringLiteral("Automatic Mode: NVIDIA telemetry active.");
        return result;
      }

      if (testRes.success() && !hasPermissionError) {
        result.supported = true;
        result.controlSupported = true;
        result.capability = ProbeCapability::Controllable;
        result.hardwareType = QStringLiteral("NVIDIA (NV-CONTROL)");
        return result;
      }
    }
  }

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

    const QFileInfoList fanInputs = QDir(basePath).entryInfoList(
        {QStringLiteral("fan*_input")}, QDir::Files, QDir::Name);
    if (isGpu && !fanInputs.isEmpty()) {
      foundGpuTelemetry = true;
    }

    const QString pwmPath = basePath + QStringLiteral("/pwm1");
    const QString pwmEnablePath = basePath + QStringLiteral("/pwm1_enable");

    if (QFile::exists(pwmPath) && (isGpu || hwmonEntries.size() == 1)) {
      foundPwm = true;
      QFileInfo pwmInfo(pwmPath);
      if (pwmInfo.isWritable()) {
        pwmWritable = true;
        result.verifiedHwmonPwmPath = pwmPath;
        result.verifiedHwmonPwmEnablePath = pwmEnablePath;
        break;
      }
    }
  }

  if (pwmWritable) {
    result.supported = true;
    result.controlSupported = true;
    result.capability = ProbeCapability::Controllable;
    result.hardwareType = QStringLiteral("Linux HWMON (sysfs)");
    return result;
  }

  if (foundPwm) {
    result.supported = true;
    result.controlSupported = false;
    result.capability = ProbeCapability::PermissionDenied;
    result.hardwareType = QStringLiteral("Linux HWMON (sysfs)");
    return result;
  }

  if (foundGpuTelemetry) {
    result.supported = true;
    result.controlSupported = false;
    result.capability = ProbeCapability::TelemetryOnly;
    result.hardwareType = QStringLiteral("Linux HWMON (Read-Only)");
    return result;
  }

  result.supported = false;
  result.controlSupported = false;
  result.capability = ProbeCapability::Unsupported;
  result.hardwareType = QStringLiteral("None");
  return result;
}

void FanHardwareProbe::probeHwmonTopology(HwmonTopology &topology) {
  if (topology.topologyProbed) {
    return;
  }
  topology.topologyProbed = true;

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
      if (chipName.contains(QStringLiteral("coretemp"), Qt::CaseInsensitive) &&
          topology.coretempInput.isEmpty()) {
        topology.coretempInput = tFile.absoluteFilePath();
      } else if (chipName.contains(QStringLiteral("acpitz"),
                                   Qt::CaseInsensitive) &&
                 topology.acpitzInput.isEmpty()) {
        topology.acpitzInput = tFile.absoluteFilePath();
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
      if (topology.cpuFanRpmInput.isEmpty() &&
          normalizedLabel.contains(QStringLiteral("cpu"))) {
        topology.cpuFanRpmInput = fFile.absoluteFilePath();
      } else if (topology.sysFanRpmInput.isEmpty() &&
                 (normalizedLabel.contains(QStringLiteral("sys")) ||
                  normalizedLabel.contains(QStringLiteral("chassis")) ||
                  normalizedLabel.contains(QStringLiteral("case")))) {
        topology.sysFanRpmInput = fFile.absoluteFilePath();
      }
    }
  }
}

void FanHardwareProbe::probeExtraChannels(HwmonTopology &topology) {
  if (topology.extraChannelsProbed) {
    return;
  }
  topology.extraChannelsProbed = true;

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
      if (abs == topology.cpuFanRpmInput || abs == topology.sysFanRpmInput) {
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
      topology.extraChannels.append(channel);
    }
  }
}
