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
  int gpuUtilizationPercent = 0;
  int gpuMemoryUsedMiB = 0;
  int gpuMemoryTotalMiB = 0;
  int gpuMemoryUsagePercent = 0;

  bool ramAvailable = false;
  int ramTotalMiB = 0;
  int ramUsedMiB = 0;
  int ramUsagePercent = 0;
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
