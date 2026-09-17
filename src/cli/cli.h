#pragma once

#include <QString>
#include <QStringList>

class QJsonObject;

namespace RoControlCli {

enum class CommandAction {
  LaunchGui,
  PrintHelp,
  PrintVersion,
  PrintStatusText,
  PrintStatusJson,
  PrintDiagnosticsText,
  PrintDiagnosticsJson,
  InstallProprietaryDriver,
  InstallOpenSourceDriver,
  RemoveDriver,
  UpdateDriver,
  DeepCleanDriver,
  FanSetMode,
  FanSetSpeed,
  FanReset,
  PrintFanStatusText,
  PrintFanStatusJson,
  PrintPowerStatusText,
  PrintPowerStatusJson,
  PowerSetLimit,
  PowerSetPersistence,
  PowerSetPreset,
  PowerSetClocks,
  PrintProcessesText,
  PrintProcessesJson,
  KillProcess,
  PrintGpusText,
  PrintGpusJson,
  SelectGpu,
  FanSetSmoothing,
  RunDaemon,
  Invalid,
};

struct ParsedCommand {
  CommandAction action = CommandAction::LaunchGui;
  QString payload;
  bool acceptLicense = false;
};

struct DiagnosticsSnapshot {
  QString applicationName;
  QString applicationVersion;
  QString locale;

  bool gpuFound = false;
  QString gpuName;
  QString driverVersion;
  QString activeDriver;
  bool secureBootEnabled = false;
  QString sessionType;
  QString verificationReport;

  QString currentDriverVersion;
  QString latestDriverVersion;
  bool updateAvailable = false;

  bool cpuAvailable = false;
  double cpuUsagePercent = 0.0;
  int cpuTemperatureC = 0;

  bool gpuMonitorAvailable = false;
  QString gpuMonitorName;
  int gpuTemperatureC = 0;
  int gpuHotspotTemperatureC = 0;
  int gpuMemoryTemperatureC = 0;
  int gpuUtilizationPercent = 0;
  int gpuMemoryUsedMiB = 0;
  int gpuMemoryTotalMiB = 0;
  int gpuMemoryUsagePercent = 0;
  int gpuFanSpeedPercent = 0;
  int gpuCount = 1;
  int gpuProcessCount = 0;

  bool fanSupported = false;
  bool fanControlSupported = false;
  QString fanCapability;
  QString fanHardwareType;
  QString fanMode;
  int fanTargetSpeedPercent = 0;
  int fanRpm = 0;
  bool fanSafetyOverride = false;
  int fanThermalThresholdC = 85;
  bool fanSmoothingEnabled = true;

  bool ramAvailable = false;
  int ramTotalMiB = 0;
  int ramUsedMiB = 0;
  int ramUsagePercent = 0;

  bool powerSupported = false;
  bool powerControlSupported = false;
  double powerDrawW = 0.0;
  double powerLimitW = 0.0;
  double minPowerLimitW = 0.0;
  double maxPowerLimitW = 0.0;
  double defaultPowerLimitW = 0.0;
  bool persistenceModeEnabled = false;
  QString powerPreset;
  int coreClockOffsetMHz = 0;
  int memoryClockOffsetMHz = 0;
};

ParsedCommand parseArguments(const QStringList &arguments,
                             const QString &applicationName,
                             const QString &applicationVersion,
                             const QString &applicationDescription);

DiagnosticsSnapshot collectDiagnostics(const QString &applicationName,
                                       const QString &applicationVersion);

QString renderStatusText(const DiagnosticsSnapshot &snapshot);
QString renderDiagnosticsText(const DiagnosticsSnapshot &snapshot);
QJsonObject renderDiagnosticsJsonObject(const DiagnosticsSnapshot &snapshot);
QJsonObject renderStatusJsonObject(const DiagnosticsSnapshot &snapshot);

} // namespace RoControlCli
