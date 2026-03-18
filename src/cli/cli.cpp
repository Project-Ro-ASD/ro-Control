#include "cli.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QJsonObject>
#include <QLocale>
#include <QTextStream>
#include <QThread>

#include "backend/monitor/cpumonitor.h"
#include "backend/monitor/gpumonitor.h"
#include "backend/monitor/rammonitor.h"
#include "backend/nvidia/detector.h"
#include "backend/nvidia/updater.h"

namespace RoControlCli {

namespace {

QString commandActionToString(CommandAction action) {
  switch (action) {
  case CommandAction::PrintStatusText:
  case CommandAction::PrintStatusJson:
    return QStringLiteral("status");
  case CommandAction::PrintDiagnosticsText:
  case CommandAction::PrintDiagnosticsJson:
    return QStringLiteral("diagnostics");
  default:
    return QStringLiteral("unknown");
  }
}

QString boolText(bool value) {
  return value ? QStringLiteral("yes") : QStringLiteral("no");
}

QString dashIfEmpty(const QString &value) {
  return value.isEmpty() ? QStringLiteral("-") : value;
}

QString buildHelpText(const QString &applicationName,
                      const QString &applicationVersion,
                      const QString &applicationDescription) {
  QString help;
  QTextStream stream(&help);

  stream << applicationName << ' ' << applicationVersion << '\n';
  stream << applicationDescription << "\n\n";
  stream << "Usage:\n";
  stream << "  " << applicationName << " [command] [options]\n";
  stream << "  " << applicationName << " --diagnostics [--json]\n";
  stream << "  " << applicationName << " --version\n\n";
  stream << "Commands:\n";
  stream << "  help                       Show this help text.\n";
  stream << "  version                    Print the application version.\n";
  stream << "  status [--json]            Print a concise system and driver "
            "status.\n";
  stream << "  diagnostics [--json]       Print a full diagnostics snapshot.\n";
  stream << "  driver install [options]   Install the NVIDIA driver.\n";
  stream << "  driver remove              Remove installed NVIDIA packages.\n";
  stream
      << "  driver update              Update the installed NVIDIA driver.\n";
  stream << "  driver deep-clean          Remove legacy NVIDIA leftovers.\n\n";
  stream << "Driver install options:\n";
  stream << "  --proprietary              Install the proprietary akmod-nvidia "
            "stack.\n";
  stream << "  --open-source              Install the open-source Nouveau "
            "stack.\n";
  stream << "  --accept-license           Confirm proprietary driver license "
            "acceptance.\n\n";
  stream << "Global options:\n";
  stream << "  -h, --help                 Show help and exit.\n";
  stream << "  -v, --version              Show version and exit.\n";
  stream << "  -d, --diagnostics          Legacy alias for `diagnostics`.\n";
  stream << "  --json                     Render `status` or `diagnostics` as "
            "JSON.\n\n";
  stream << "Examples:\n";
  stream << "  " << applicationName << " status\n";
  stream << "  " << applicationName << " diagnostics --json\n";
  stream << "  " << applicationName
         << " driver install --proprietary --accept-license\n";
  stream << "  " << applicationName << " driver update\n";

  return help;
}

void configureParser(QCommandLineParser &parser, const QString &applicationName,
                     const QString &applicationVersion,
                     const QString &applicationDescription) {
  parser.setApplicationDescription(applicationDescription);
  parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

  QCoreApplication::setApplicationName(applicationName);
  QCoreApplication::setApplicationVersion(applicationVersion);

  parser.addOption(
      QCommandLineOption({QStringLiteral("h"), QStringLiteral("help")},
                         QStringLiteral("Display CLI usage information.")));
  parser.addOption(
      QCommandLineOption({QStringLiteral("v"), QStringLiteral("version")},
                         QStringLiteral("Display the application version.")));
  parser.addOption(QCommandLineOption(
      {QStringLiteral("d"), QStringLiteral("diagnostics")},
      QStringLiteral(
          "Print a one-shot system and driver diagnostics snapshot.")));
  parser.addOption(QCommandLineOption(
      {QStringLiteral("json")},
      QStringLiteral("Render status or diagnostics output as JSON.")));
  parser.addOption(QCommandLineOption(
      {QStringLiteral("proprietary")},
      QStringLiteral("Use the proprietary NVIDIA driver install path.")));
  parser.addOption(QCommandLineOption(
      {QStringLiteral("open-source")},
      QStringLiteral("Use the open-source Nouveau install path.")));
  parser.addOption(QCommandLineOption(
      {QStringLiteral("accept-license")},
      QStringLiteral(
          "Confirm that the proprietary NVIDIA license was reviewed.")));
  parser.addPositionalArgument(QStringLiteral("command"),
                               QStringLiteral("CLI command to execute."));
  parser.addPositionalArgument(
      QStringLiteral("subcommand"),
      QStringLiteral("Optional command scope or action."),
      QStringLiteral("[subcommand]"));
}

ParsedCommand invalidCommand(const QString &message) {
  ParsedCommand command;
  command.action = CommandAction::Invalid;
  command.payload = message;
  return command;
}

bool hasConflictingInstallModeOptions(const QCommandLineParser &parser) {
  return parser.isSet(QStringLiteral("proprietary")) &&
         parser.isSet(QStringLiteral("open-source"));
}

} // namespace

ParsedCommand parseArguments(const QStringList &arguments,
                             const QString &applicationName,
                             const QString &applicationVersion,
                             const QString &applicationDescription) {
  QCommandLineParser parser;
  configureParser(parser, applicationName, applicationVersion,
                  applicationDescription);

  if (!parser.parse(arguments)) {
    return invalidCommand(parser.errorText());
  }

  parser.process(arguments);

  const QString helpText = buildHelpText(applicationName, applicationVersion,
                                         applicationDescription);

  const bool help = parser.isSet(QStringLiteral("help"));
  const bool version = parser.isSet(QStringLiteral("version"));
  const bool diagnosticsFlag = parser.isSet(QStringLiteral("diagnostics"));
  const bool json = parser.isSet(QStringLiteral("json"));
  const bool proprietary = parser.isSet(QStringLiteral("proprietary"));
  const bool openSource = parser.isSet(QStringLiteral("open-source"));
  const bool acceptLicense = parser.isSet(QStringLiteral("accept-license"));
  const QStringList positional = parser.positionalArguments();

  if (hasConflictingInstallModeOptions(parser)) {
    return invalidCommand(QStringLiteral(
        "--proprietary and --open-source cannot be used together."));
  }

  if (help) {
    ParsedCommand command;
    command.action = CommandAction::PrintHelp;
    command.payload = helpText;
    return command;
  }

  if (version && positional.isEmpty() && !diagnosticsFlag) {
    ParsedCommand command;
    command.action = CommandAction::PrintVersion;
    command.payload = applicationVersion;
    return command;
  }

  if (diagnosticsFlag) {
    if (!positional.isEmpty()) {
      return invalidCommand(QStringLiteral(
          "--diagnostics cannot be combined with positional commands."));
    }

    ParsedCommand command;
    command.action = json ? CommandAction::PrintDiagnosticsJson
                          : CommandAction::PrintDiagnosticsText;
    return command;
  }

  if (json && positional.isEmpty()) {
    return invalidCommand(QStringLiteral(
        "--json can only be used with `status` or `diagnostics`."));
  }

  if (proprietary || openSource || acceptLicense) {
    if (positional.value(0) != QStringLiteral("driver") ||
        positional.value(1) != QStringLiteral("install")) {
      return invalidCommand(
          QStringLiteral("--proprietary, --open-source and --accept-license "
                         "can only be used with `driver install`."));
    }
  }

  if (positional.isEmpty()) {
    return ParsedCommand{};
  }

  const QString commandName = positional.at(0).toLower();

  if (commandName == QStringLiteral("help")) {
    ParsedCommand command;
    command.action = CommandAction::PrintHelp;
    command.payload = helpText;
    return command;
  }

  if (commandName == QStringLiteral("version")) {
    if (positional.size() != 1) {
      return invalidCommand(
          QStringLiteral("`version` does not take arguments."));
    }

    ParsedCommand command;
    command.action = CommandAction::PrintVersion;
    command.payload = applicationVersion;
    return command;
  }

  if (commandName == QStringLiteral("status")) {
    if (positional.size() != 1) {
      return invalidCommand(
          QStringLiteral("`status` does not take arguments."));
    }

    ParsedCommand command;
    command.action =
        json ? CommandAction::PrintStatusJson : CommandAction::PrintStatusText;
    return command;
  }

  if (commandName == QStringLiteral("diagnostics")) {
    if (positional.size() != 1) {
      return invalidCommand(
          QStringLiteral("`diagnostics` does not take arguments."));
    }

    ParsedCommand command;
    command.action = json ? CommandAction::PrintDiagnosticsJson
                          : CommandAction::PrintDiagnosticsText;
    return command;
  }

  if (commandName != QStringLiteral("driver")) {
    return invalidCommand(
        QStringLiteral("Unknown command: %1").arg(commandName));
  }

  if (positional.size() < 2) {
    return invalidCommand(
        QStringLiteral("`driver` requires a subcommand: install, remove, "
                       "update, deep-clean."));
  }

  if (json) {
    return invalidCommand(QStringLiteral(
        "--json is only supported by `status` and `diagnostics`."));
  }

  const QString driverAction = positional.at(1).toLower();
  if (driverAction == QStringLiteral("install")) {
    if (positional.size() != 2) {
      return invalidCommand(QStringLiteral(
          "`driver install` does not take positional arguments."));
    }

    ParsedCommand command;
    command.acceptLicense = acceptLicense;
    command.action = openSource ? CommandAction::InstallOpenSourceDriver
                                : CommandAction::InstallProprietaryDriver;

    if (openSource && acceptLicense) {
      return invalidCommand(QStringLiteral(
          "--accept-license is only valid with the proprietary install path."));
    }

    return command;
  }

  if (proprietary || openSource || acceptLicense) {
    return invalidCommand(QStringLiteral(
        "Install-specific options can only be used with `driver install`."));
  }

  if (positional.size() != 2) {
    return invalidCommand(
        QStringLiteral("`driver %1` does not take positional arguments.")
            .arg(driverAction));
  }

  if (driverAction == QStringLiteral("remove")) {
    ParsedCommand command;
    command.action = CommandAction::RemoveDriver;
    return command;
  }

  if (driverAction == QStringLiteral("update")) {
    ParsedCommand command;
    command.action = CommandAction::UpdateDriver;
    return command;
  }

  if (driverAction == QStringLiteral("deep-clean")) {
    ParsedCommand command;
    command.action = CommandAction::DeepCleanDriver;
    return command;
  }

  return invalidCommand(
      QStringLiteral("Unknown `driver` subcommand: %1").arg(driverAction));
}

DiagnosticsSnapshot collectDiagnostics(const QString &applicationName,
                                       const QString &applicationVersion) {
  DiagnosticsSnapshot snapshot;
  snapshot.applicationName = applicationName;
  snapshot.applicationVersion = applicationVersion;
  snapshot.locale = QLocale::system().name();

  NvidiaDetector detector;
  detector.refresh();

  snapshot.gpuFound = detector.gpuFound();
  snapshot.gpuName = detector.gpuName();
  snapshot.driverVersion = detector.driverVersion();
  snapshot.activeDriver = detector.activeDriver();
  snapshot.secureBootEnabled = detector.secureBootEnabled();
  snapshot.sessionType = detector.sessionType();
  snapshot.verificationReport = detector.verificationReport();

  NvidiaUpdater updater;
  updater.checkForUpdate();
  snapshot.currentDriverVersion = updater.currentVersion();
  snapshot.latestDriverVersion = updater.latestVersion();
  snapshot.updateAvailable = updater.updateAvailable();

  CpuMonitor cpuMonitor;
  cpuMonitor.stop();
  cpuMonitor.refresh();
  QThread::msleep(25);
  cpuMonitor.refresh();
  snapshot.cpuAvailable = cpuMonitor.available();
  snapshot.cpuUsagePercent = cpuMonitor.usagePercent();
  snapshot.cpuTemperatureC = cpuMonitor.temperatureC();

  GpuMonitor gpuMonitor;
  gpuMonitor.stop();
  gpuMonitor.refresh();
  snapshot.gpuMonitorAvailable = gpuMonitor.available();
  snapshot.gpuMonitorName = gpuMonitor.gpuName();
  snapshot.gpuTemperatureC = gpuMonitor.temperatureC();
  snapshot.gpuUtilizationPercent = gpuMonitor.utilizationPercent();
  snapshot.gpuMemoryUsedMiB = gpuMonitor.memoryUsedMiB();
  snapshot.gpuMemoryTotalMiB = gpuMonitor.memoryTotalMiB();
  snapshot.gpuMemoryUsagePercent = gpuMonitor.memoryUsagePercent();

  RamMonitor ramMonitor;
  ramMonitor.stop();
  ramMonitor.refresh();
  snapshot.ramAvailable = ramMonitor.available();
  snapshot.ramTotalMiB = ramMonitor.totalMiB();
  snapshot.ramUsedMiB = ramMonitor.usedMiB();
  snapshot.ramUsagePercent = ramMonitor.usagePercent();

  return snapshot;
}

QString renderStatusText(const DiagnosticsSnapshot &snapshot) {
  QString output;
  output += QStringLiteral("application: %1\n").arg(snapshot.applicationName);
  output += QStringLiteral("version: %1\n").arg(snapshot.applicationVersion);
  output += QStringLiteral("gpu_found: %1\n").arg(boolText(snapshot.gpuFound));
  output += QStringLiteral("gpu_name: %1\n").arg(dashIfEmpty(snapshot.gpuName));
  output += QStringLiteral("active_driver: %1\n")
                .arg(dashIfEmpty(snapshot.activeDriver));
  output += QStringLiteral("driver_version: %1\n")
                .arg(dashIfEmpty(snapshot.driverVersion));
  output += QStringLiteral("session_type: %1\n")
                .arg(dashIfEmpty(snapshot.sessionType));
  output += QStringLiteral("secure_boot_enabled: %1\n")
                .arg(boolText(snapshot.secureBootEnabled));
  output += QStringLiteral("update_available: %1\n")
                .arg(boolText(snapshot.updateAvailable));
  output += QStringLiteral("latest_driver_version: %1\n")
                .arg(dashIfEmpty(snapshot.latestDriverVersion));
  return output;
}

QString renderDiagnosticsText(const DiagnosticsSnapshot &snapshot) {
  QString output;
  output += renderStatusText(snapshot);
  output += QStringLiteral("locale: %1\n").arg(snapshot.locale);
  output += QStringLiteral("current_driver_version: %1\n")
                .arg(dashIfEmpty(snapshot.currentDriverVersion));
  output += QStringLiteral("cpu_available: %1\n")
                .arg(boolText(snapshot.cpuAvailable));
  output += QStringLiteral("cpu_usage_percent: %1\n")
                .arg(snapshot.cpuUsagePercent, 0, 'f', 1);
  output +=
      QStringLiteral("cpu_temperature_c: %1\n").arg(snapshot.cpuTemperatureC);
  output += QStringLiteral("gpu_monitor_available: %1\n")
                .arg(boolText(snapshot.gpuMonitorAvailable));
  output += QStringLiteral("gpu_monitor_name: %1\n")
                .arg(dashIfEmpty(snapshot.gpuMonitorName));
  output +=
      QStringLiteral("gpu_temperature_c: %1\n").arg(snapshot.gpuTemperatureC);
  output += QStringLiteral("gpu_utilization_percent: %1\n")
                .arg(snapshot.gpuUtilizationPercent);
  output += QStringLiteral("gpu_memory_used_mib: %1\n")
                .arg(snapshot.gpuMemoryUsedMiB);
  output += QStringLiteral("gpu_memory_total_mib: %1\n")
                .arg(snapshot.gpuMemoryTotalMiB);
  output += QStringLiteral("gpu_memory_usage_percent: %1\n")
                .arg(snapshot.gpuMemoryUsagePercent);
  output += QStringLiteral("ram_available: %1\n")
                .arg(boolText(snapshot.ramAvailable));
  output += QStringLiteral("ram_total_mib: %1\n").arg(snapshot.ramTotalMiB);
  output += QStringLiteral("ram_used_mib: %1\n").arg(snapshot.ramUsedMiB);
  output +=
      QStringLiteral("ram_usage_percent: %1\n").arg(snapshot.ramUsagePercent);

  if (!snapshot.verificationReport.isEmpty()) {
    output += QStringLiteral("verification_report:\n%1\n")
                  .arg(snapshot.verificationReport);
  }

  return output;
}

QJsonObject renderStatusJsonObject(const DiagnosticsSnapshot &snapshot) {
  QJsonObject object;
  object.insert(QStringLiteral("command"),
                commandActionToString(CommandAction::PrintStatusJson));
  object.insert(QStringLiteral("application"), snapshot.applicationName);
  object.insert(QStringLiteral("version"), snapshot.applicationVersion);
  object.insert(QStringLiteral("gpuFound"), snapshot.gpuFound);
  object.insert(QStringLiteral("gpuName"), snapshot.gpuName);
  object.insert(QStringLiteral("activeDriver"), snapshot.activeDriver);
  object.insert(QStringLiteral("driverVersion"), snapshot.driverVersion);
  object.insert(QStringLiteral("sessionType"), snapshot.sessionType);
  object.insert(QStringLiteral("secureBootEnabled"),
                snapshot.secureBootEnabled);
  object.insert(QStringLiteral("updateAvailable"), snapshot.updateAvailable);
  object.insert(QStringLiteral("latestDriverVersion"),
                snapshot.latestDriverVersion);
  return object;
}

QJsonObject renderDiagnosticsJsonObject(const DiagnosticsSnapshot &snapshot) {
  QJsonObject object = renderStatusJsonObject(snapshot);
  object.insert(QStringLiteral("command"),
                commandActionToString(CommandAction::PrintDiagnosticsJson));
  object.insert(QStringLiteral("locale"), snapshot.locale);
  object.insert(QStringLiteral("verificationReport"),
                snapshot.verificationReport);
  object.insert(QStringLiteral("currentDriverVersion"),
                snapshot.currentDriverVersion);
  object.insert(QStringLiteral("cpuAvailable"), snapshot.cpuAvailable);
  object.insert(QStringLiteral("cpuUsagePercent"), snapshot.cpuUsagePercent);
  object.insert(QStringLiteral("cpuTemperatureC"), snapshot.cpuTemperatureC);
  object.insert(QStringLiteral("gpuMonitorAvailable"),
                snapshot.gpuMonitorAvailable);
  object.insert(QStringLiteral("gpuMonitorName"), snapshot.gpuMonitorName);
  object.insert(QStringLiteral("gpuTemperatureC"), snapshot.gpuTemperatureC);
  object.insert(QStringLiteral("gpuUtilizationPercent"),
                snapshot.gpuUtilizationPercent);
  object.insert(QStringLiteral("gpuMemoryUsedMiB"), snapshot.gpuMemoryUsedMiB);
  object.insert(QStringLiteral("gpuMemoryTotalMiB"),
                snapshot.gpuMemoryTotalMiB);
  object.insert(QStringLiteral("gpuMemoryUsagePercent"),
                snapshot.gpuMemoryUsagePercent);
  object.insert(QStringLiteral("ramAvailable"), snapshot.ramAvailable);
  object.insert(QStringLiteral("ramTotalMiB"), snapshot.ramTotalMiB);
  object.insert(QStringLiteral("ramUsedMiB"), snapshot.ramUsedMiB);
  object.insert(QStringLiteral("ramUsagePercent"), snapshot.ramUsagePercent);
  return object;
}

} // namespace RoControlCli
