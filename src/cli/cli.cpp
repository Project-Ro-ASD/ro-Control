#include "cli.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QEventLoop>
#include <QJsonObject>
#include <QLocale>
#include <QTextStream>
#include <QThread>

#include "backend/fan/fancontroller.h"
#include "backend/monitor/cpumonitor.h"
#include "backend/monitor/gpumonitor.h"
#include "backend/monitor/rammonitor.h"
#include "backend/nvidia/detector.h"
#include "backend/nvidia/updater.h"
#include "backend/power/powercontroller.h"

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
  case CommandAction::PrintFanStatusText:
  case CommandAction::PrintFanStatusJson:
    return QStringLiteral("fan-status");
  case CommandAction::PrintPowerStatusText:
  case CommandAction::PrintPowerStatusJson:
    return QStringLiteral("power-status");
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
  stream << "  driver deep-clean          Remove legacy NVIDIA leftovers.\n";
  stream << "  fan status [--json]        Print current GPU fan status and "
            "profile.\n";
  stream
      << "  fan set-speed <percent>    Set manual fixed fan speed (0-100%).\n";
  stream << "  fan set-mode <profile>     Set fan profile (auto, silent, "
            "balanced, performance, manual, custom).\n";
  stream << "  fan set-smoothing <on|off> [ramp_up] [ramp_down] [hysteresis] "
            "Set fan smoothing and ramp rates.\n";
  stream << "  fan reset                  Reset fan control to automatic "
            "mode.\n";
  stream << "  power status [--json]      Print GPU power draw, limits and "
            "persistence mode.\n";
  stream << "  power set-limit <watts>    Set GPU power limit in Watts.\n";
  stream << "  power set-preset <preset>  Set power preset (eco, balanced, "
            "performance, custom).\n";
  stream << "  power set-clocks <core> <mem> Set GPU core and memory clock "
            "offsets in MHz.\n";
  stream << "  power set-persistence <on|off> Enable or disable persistence "
            "mode.\n";
  stream << "  processes [--json]         List active GPU compute and display "
            "processes.\n";
  stream << "  kill-process <pid>         Terminate a running GPU process.\n";
  stream << "  gpus [--json]              List detected GPU adapters.\n";
  stream
      << "  select-gpu <index>         Select active GPU device by index.\n\n";
  stream << "Driver install options:\n";
  stream << "  --proprietary              Install the proprietary akmod-nvidia "
            "stack.\n";
  stream << "  --open-source              Switch to the community open-source "
            "graphics stack.\n";
  stream << "  --accept-license           Confirm NVIDIA license review for "
            "the proprietary install path.\n\n";
  stream << "Global options:\n";
  stream << "  -h, --help                 Show help and exit.\n";
  stream << "  -v, --version              Show version and exit.\n";
  stream << "  -d, --diagnostics          Legacy alias for `diagnostics`.\n";
  stream << "  --json                     Render `status` or `diagnostics` as "
            "JSON.\n";
  stream << "  --daemon                   Run headless in background with "
            "D-Bus service.\n\n";
  stream << "Examples:\n";
  stream << "  " << applicationName << " status\n";
  stream << "  " << applicationName << " diagnostics --json\n";
  stream << "  " << applicationName << " power status\n";
  stream << "  " << applicationName << " power set-limit 180\n";
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
      {QStringLiteral("daemon")},
      QStringLiteral("Run ro-Control in background daemon mode.")));
  parser.addOption(QCommandLineOption(
      {QStringLiteral("proprietary")},
      QStringLiteral("Use the proprietary NVIDIA driver install path.")));
  parser.addOption(QCommandLineOption(
      {QStringLiteral("open-source")},
      QStringLiteral("Use the community open-source graphics path.")));
  parser.addOption(QCommandLineOption(
      {QStringLiteral("accept-license")},
      QStringLiteral("Confirm that the NVIDIA license was reviewed.")));
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

  const QString helpText = buildHelpText(applicationName, applicationVersion,
                                         applicationDescription);

  const bool help = parser.isSet(QStringLiteral("help"));
  const bool version = parser.isSet(QStringLiteral("version"));
  const bool diagnosticsFlag = parser.isSet(QStringLiteral("diagnostics"));
  const bool json = parser.isSet(QStringLiteral("json"));
  const bool daemonFlag = parser.isSet(QStringLiteral("daemon"));
  const bool proprietary = parser.isSet(QStringLiteral("proprietary"));
  const bool openSource = parser.isSet(QStringLiteral("open-source"));
  const bool acceptLicense = parser.isSet(QStringLiteral("accept-license"));
  const QStringList positional = parser.positionalArguments();

  if (daemonFlag) {
    ParsedCommand command;
    command.action = CommandAction::RunDaemon;
    return command;
  }

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
    const bool isDriverInstall =
        (positional.value(0) == QStringLiteral("driver") &&
         positional.value(1) == QStringLiteral("install")) ||
        positional.value(0) == QStringLiteral("install-driver");
    if (!isDriverInstall) {
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

  if (commandName == QStringLiteral("fan-status")) {
    if (positional.size() != 1) {
      return invalidCommand(
          QStringLiteral("`fan-status` does not take extra arguments."));
    }

    ParsedCommand command;
    command.action = json ? CommandAction::PrintFanStatusJson
                          : CommandAction::PrintFanStatusText;
    return command;
  }

  if (commandName == QStringLiteral("check-updates")) {
    if (positional.size() != 1) {
      return invalidCommand(
          QStringLiteral("`check-updates` does not take arguments."));
    }

    ParsedCommand command;
    command.action = CommandAction::UpdateDriver;
    return command;
  }

  if (commandName == QStringLiteral("install-driver")) {
    ParsedCommand command;
    command.action = openSource ? CommandAction::InstallOpenSourceDriver
                                : CommandAction::InstallProprietaryDriver;
    command.acceptLicense = acceptLicense;
    return command;
  }

  if (commandName == QStringLiteral("fan")) {
    if (positional.size() < 2) {
      return invalidCommand(
          QStringLiteral("`fan` requires a subcommand: status, set-speed, "
                         "set-mode, reset."));
    }

    const QString fanAction = positional.at(1).toLower();
    if (fanAction == QStringLiteral("status")) {
      if (positional.size() != 2) {
        return invalidCommand(
            QStringLiteral("`fan status` does not take extra arguments."));
      }
      ParsedCommand command;
      command.action = json ? CommandAction::PrintFanStatusJson
                            : CommandAction::PrintFanStatusText;
      return command;
    }

    if (json) {
      return invalidCommand(QStringLiteral(
          "--json is only supported by `status`, `diagnostics`, and `fan "
          "status`."));
    }

    if (fanAction == QStringLiteral("set-speed")) {
      if (positional.size() != 3) {
        return invalidCommand(QStringLiteral(
            "`fan set-speed` requires a percentage argument (0-100)."));
      }
      bool ok = false;
      const int speed = positional.at(2).toInt(&ok);
      if (!ok || speed < 0 || speed > 100) {
        return invalidCommand(
            QStringLiteral("Fan speed must be an integer between 0 and 100."));
      }
      ParsedCommand command;
      command.action = CommandAction::FanSetSpeed;
      command.payload = QString::number(speed);
      return command;
    }

    if (fanAction == QStringLiteral("set-mode")) {
      if (positional.size() != 3) {
        return invalidCommand(QStringLiteral(
            "`fan set-mode` requires a profile argument (auto, silent, "
            "balanced, performance, manual, custom)."));
      }
      const QString mode = positional.at(2).toLower();
      const QStringList validModes = {
          QStringLiteral("auto"),     QStringLiteral("silent"),
          QStringLiteral("balanced"), QStringLiteral("performance"),
          QStringLiteral("manual"),   QStringLiteral("custom")};
      if (!validModes.contains(mode)) {
        return invalidCommand(QStringLiteral(
            "Invalid fan mode. Choose from: auto, silent, balanced, "
            "performance, manual, custom."));
      }
      ParsedCommand command;
      command.action = CommandAction::FanSetMode;
      command.payload = mode;
      return command;
    }

    if (fanAction == QStringLiteral("set-smoothing")) {
      if (positional.size() < 3) {
        return invalidCommand(QStringLiteral(
            "`fan set-smoothing` requires a state argument (on|off) and "
            "optional [ramp_up] [ramp_down] [hysteresis]."));
      }
      const QString state = positional.at(2).toLower();
      const bool on =
          (state == QStringLiteral("on") || state == QStringLiteral("1") ||
           state == QStringLiteral("true"));
      QStringList params;
      params << (on ? QStringLiteral("1") : QStringLiteral("0"));
      for (int i = 3; i < positional.size(); ++i) {
        params << positional.at(i);
      }
      ParsedCommand command;
      command.action = CommandAction::FanSetSmoothing;
      command.payload = params.join(QLatin1Char(':'));
      return command;
    }

    if (fanAction == QStringLiteral("reset")) {
      if (positional.size() != 2) {
        return invalidCommand(
            QStringLiteral("`fan reset` does not take extra arguments."));
      }
      ParsedCommand command;
      command.action = CommandAction::FanReset;
      return command;
    }

    return invalidCommand(
        QStringLiteral("Unknown `fan` subcommand: %1").arg(fanAction));
  }

  if (commandName == QStringLiteral("power")) {
    if (positional.size() < 2) {
      return invalidCommand(
          QStringLiteral("`power` requires a subcommand: status, set-limit, "
                         "set-preset, set-clocks, set-persistence."));
    }

    const QString powerAction = positional.at(1).toLower();
    if (powerAction == QStringLiteral("status")) {
      if (positional.size() != 2) {
        return invalidCommand(
            QStringLiteral("`power status` does not take extra arguments."));
      }
      ParsedCommand command;
      command.action = json ? CommandAction::PrintPowerStatusJson
                            : CommandAction::PrintPowerStatusText;
      return command;
    }

    if (json) {
      return invalidCommand(QStringLiteral(
          "--json is only supported by `status`, `diagnostics`, `fan status` "
          "and `power status`."));
    }

    if (powerAction == QStringLiteral("set-limit")) {
      if (positional.size() != 3) {
        return invalidCommand(QStringLiteral(
            "`power set-limit` requires a wattage argument (e.g. 180)."));
      }
      bool ok = false;
      const double watts = positional.at(2).toDouble(&ok);
      if (!ok || watts <= 0.0) {
        return invalidCommand(
            QStringLiteral("Power limit must be a positive number."));
      }
      ParsedCommand command;
      command.action = CommandAction::PowerSetLimit;
      command.payload = QString::number(watts);
      return command;
    }

    if (powerAction == QStringLiteral("set-clocks")) {
      if (positional.size() != 4) {
        return invalidCommand(
            QStringLiteral("`power set-clocks` requires <core_mhz> and "
                           "<mem_mhz> arguments (e.g. 50 200)."));
      }
      bool ok1 = false, ok2 = false;
      const int core = positional.at(2).toInt(&ok1);
      const int mem = positional.at(3).toInt(&ok2);
      if (!ok1 || !ok2) {
        return invalidCommand(
            QStringLiteral("Clock offsets must be integers."));
      }
      ParsedCommand command;
      command.action = CommandAction::PowerSetClocks;
      command.payload = QStringLiteral("%1:%2").arg(core).arg(mem);
      return command;
    }

    if (powerAction == QStringLiteral("set-preset")) {
      if (positional.size() != 3) {
        return invalidCommand(QStringLiteral(
            "`power set-preset` requires a preset argument (eco, balanced, "
            "performance, custom)."));
      }
      const QString preset = positional.at(2).toLower();
      const QStringList validPresets = {
          QStringLiteral("eco"), QStringLiteral("balanced"),
          QStringLiteral("performance"), QStringLiteral("custom")};
      if (!validPresets.contains(preset)) {
        return invalidCommand(
            QStringLiteral("Invalid power preset. Choose from: eco, balanced, "
                           "performance, custom."));
      }
      ParsedCommand command;
      command.action = CommandAction::PowerSetPreset;
      command.payload = preset;
      return command;
    }

    if (powerAction == QStringLiteral("set-persistence")) {
      if (positional.size() != 3) {
        return invalidCommand(QStringLiteral(
            "`power set-persistence` requires a state argument (on, off, 1, 0, "
            "enable, disable)."));
      }
      const QString state = positional.at(2).toLower();
      if (state != QStringLiteral("on") && state != QStringLiteral("off") &&
          state != QStringLiteral("1") && state != QStringLiteral("0") &&
          state != QStringLiteral("enable") &&
          state != QStringLiteral("disable")) {
        return invalidCommand(QStringLiteral(
            "Invalid persistence state. Choose from: on, off, 1, 0."));
      }
      const bool enable =
          (state == QStringLiteral("on") || state == QStringLiteral("1") ||
           state == QStringLiteral("enable"));
      ParsedCommand command;
      command.action = CommandAction::PowerSetPersistence;
      command.payload = enable ? QStringLiteral("1") : QStringLiteral("0");
      return command;
    }

    return invalidCommand(
        QStringLiteral("Unknown `power` subcommand: %1").arg(powerAction));
  }

  if (commandName == QStringLiteral("processes")) {
    ParsedCommand command;
    command.action = json ? CommandAction::PrintProcessesJson
                          : CommandAction::PrintProcessesText;
    return command;
  }

  if (commandName == QStringLiteral("kill-process")) {
    if (positional.size() != 2) {
      return invalidCommand(
          QStringLiteral("`kill-process` requires a PID argument."));
    }
    bool ok = false;
    const int pid = positional.at(1).toInt(&ok);
    if (!ok || pid <= 1) {
      return invalidCommand(
          QStringLiteral("PID must be a valid process ID > 1."));
    }
    ParsedCommand command;
    command.action = CommandAction::KillProcess;
    command.payload = QString::number(pid);
    return command;
  }

  if (commandName == QStringLiteral("gpus")) {
    ParsedCommand command;
    command.action =
        json ? CommandAction::PrintGpusJson : CommandAction::PrintGpusText;
    return command;
  }

  if (commandName == QStringLiteral("select-gpu")) {
    if (positional.size() != 2) {
      return invalidCommand(QStringLiteral(
          "`select-gpu` requires a GPU index argument (e.g. 0)."));
    }
    bool ok = false;
    const int idx = positional.at(1).toInt(&ok);
    if (!ok || idx < 0) {
      return invalidCommand(QStringLiteral("GPU index must be >= 0."));
    }
    ParsedCommand command;
    command.action = CommandAction::SelectGpu;
    command.payload = QString::number(idx);
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
  QEventLoop updaterLoop;
  bool updaterStarted = false;
  QObject::connect(&updater, &NvidiaUpdater::busyChanged, &updater, [&]() {
    if (updater.busy()) {
      updaterStarted = true;
      return;
    }

    if (updaterStarted) {
      updaterLoop.quit();
    }
  });
  updater.checkForUpdate();
  if (updater.busy()) {
    updaterLoop.exec();
  }
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
  snapshot.gpuHotspotTemperatureC = gpuMonitor.hotspotTemperatureC();
  snapshot.gpuMemoryTemperatureC = gpuMonitor.memoryTemperatureC();
  snapshot.gpuUtilizationPercent = gpuMonitor.utilizationPercent();
  snapshot.gpuMemoryUsedMiB = gpuMonitor.memoryUsedMiB();
  snapshot.gpuMemoryTotalMiB = gpuMonitor.memoryTotalMiB();
  snapshot.gpuMemoryUsagePercent = gpuMonitor.memoryUsagePercent();
  snapshot.gpuFanSpeedPercent = gpuMonitor.fanSpeedPercent();
  snapshot.gpuCount = gpuMonitor.gpuCount();
  snapshot.gpuProcessCount = gpuMonitor.gpuProcessCount();

  FanController fanController;
  fanController.stop();
  fanController.refresh();
  snapshot.fanSupported = fanController.supported();
  snapshot.fanControlSupported = fanController.controlSupported();
  snapshot.fanCapability = fanController.capabilityString();
  snapshot.fanHardwareType = fanController.hardwareType();
  snapshot.fanMode = fanController.fanMode();
  snapshot.fanTargetSpeedPercent = fanController.targetFanSpeedPercent();
  snapshot.fanRpm = fanController.currentRpm();
  snapshot.fanSafetyOverride = fanController.safetyOverrideActive();
  snapshot.fanThermalThresholdC = fanController.thermalThresholdC();
  snapshot.fanSmoothingEnabled = fanController.smoothingEnabled();

  RamMonitor ramMonitor;
  ramMonitor.stop();
  ramMonitor.refresh();
  snapshot.ramAvailable = ramMonitor.available();
  snapshot.ramTotalMiB = ramMonitor.totalMiB();
  snapshot.ramUsedMiB = ramMonitor.usedMiB();
  snapshot.ramUsagePercent = ramMonitor.usagePercent();

  PowerController powerController;
  powerController.refresh();
  snapshot.powerSupported = powerController.supported();
  snapshot.powerControlSupported = powerController.controlSupported();
  snapshot.powerDrawW = powerController.currentPowerDrawW();
  snapshot.powerLimitW = powerController.powerLimitW();
  snapshot.minPowerLimitW = powerController.minPowerLimitW();
  snapshot.maxPowerLimitW = powerController.maxPowerLimitW();
  snapshot.defaultPowerLimitW = powerController.defaultPowerLimitW();
  snapshot.persistenceModeEnabled = powerController.persistenceModeEnabled();
  snapshot.powerPreset = powerController.powerPreset();
  snapshot.coreClockOffsetMHz = powerController.coreClockOffsetMHz();
  snapshot.memoryClockOffsetMHz = powerController.memoryClockOffsetMHz();

  return snapshot;
}

QString renderStatusText(const DiagnosticsSnapshot &snapshot) {
  QString output;
  output += QStringLiteral("application: %1\n").arg(snapshot.applicationName);
  output += QStringLiteral("version: %1\n").arg(snapshot.applicationVersion);
  output += QStringLiteral("gpu_found: %1\n").arg(boolText(snapshot.gpuFound));
  output += QStringLiteral("gpu_name: %1\n").arg(dashIfEmpty(snapshot.gpuName));
  output += QStringLiteral("gpu_count: %1\n").arg(snapshot.gpuCount);
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
  output += QStringLiteral("gpu_fan_speed_percent: %1\n")
                .arg(snapshot.gpuFanSpeedPercent);
  output += QStringLiteral("fan_control_supported: %1\n")
                .arg(boolText(snapshot.fanControlSupported));
  output += QStringLiteral("fan_mode: %1\n").arg(dashIfEmpty(snapshot.fanMode));
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
  output += QStringLiteral("gpu_hotspot_temperature_c: %1\n")
                .arg(snapshot.gpuHotspotTemperatureC);
  output += QStringLiteral("gpu_memory_temperature_c: %1\n")
                .arg(snapshot.gpuMemoryTemperatureC);
  output += QStringLiteral("gpu_utilization_percent: %1\n")
                .arg(snapshot.gpuUtilizationPercent);
  output += QStringLiteral("gpu_memory_used_mib: %1\n")
                .arg(snapshot.gpuMemoryUsedMiB);
  output += QStringLiteral("gpu_memory_total_mib: %1\n")
                .arg(snapshot.gpuMemoryTotalMiB);
  output += QStringLiteral("gpu_memory_usage_percent: %1\n")
                .arg(snapshot.gpuMemoryUsagePercent);
  output += QStringLiteral("gpu_fan_speed_percent: %1\n")
                .arg(snapshot.gpuFanSpeedPercent);
  output +=
      QStringLiteral("gpu_process_count: %1\n").arg(snapshot.gpuProcessCount);
  output += QStringLiteral("fan_supported: %1\n")
                .arg(boolText(snapshot.fanSupported));
  output += QStringLiteral("fan_control_supported: %1\n")
                .arg(boolText(snapshot.fanControlSupported));
  output += QStringLiteral("fan_capability: %1\n")
                .arg(dashIfEmpty(snapshot.fanCapability));
  output += QStringLiteral("fan_hardware_type: %1\n")
                .arg(dashIfEmpty(snapshot.fanHardwareType));
  output += QStringLiteral("fan_mode: %1\n").arg(dashIfEmpty(snapshot.fanMode));
  output += QStringLiteral("fan_target_speed_percent: %1\n")
                .arg(snapshot.fanTargetSpeedPercent);
  output += QStringLiteral("fan_rpm: %1\n").arg(snapshot.fanRpm);
  output += QStringLiteral("fan_safety_override: %1\n")
                .arg(boolText(snapshot.fanSafetyOverride));
  output += QStringLiteral("fan_thermal_threshold_c: %1\n")
                .arg(snapshot.fanThermalThresholdC);
  output += QStringLiteral("fan_smoothing_enabled: %1\n")
                .arg(boolText(snapshot.fanSmoothingEnabled));
  output += QStringLiteral("ram_available: %1\n")
                .arg(boolText(snapshot.ramAvailable));
  output += QStringLiteral("ram_total_mib: %1\n").arg(snapshot.ramTotalMiB);
  output += QStringLiteral("ram_used_mib: %1\n").arg(snapshot.ramUsedMiB);
  output +=
      QStringLiteral("ram_usage_percent: %1\n").arg(snapshot.ramUsagePercent);
  output += QStringLiteral("power_supported: %1\n")
                .arg(boolText(snapshot.powerSupported));
  output += QStringLiteral("power_control_supported: %1\n")
                .arg(boolText(snapshot.powerControlSupported));
  output +=
      QStringLiteral("power_draw_w: %1\n").arg(snapshot.powerDrawW, 0, 'f', 1);
  output += QStringLiteral("power_limit_w: %1\n")
                .arg(snapshot.powerLimitW, 0, 'f', 1);
  output += QStringLiteral("min_power_limit_w: %1\n")
                .arg(snapshot.minPowerLimitW, 0, 'f', 1);
  output += QStringLiteral("max_power_limit_w: %1\n")
                .arg(snapshot.maxPowerLimitW, 0, 'f', 1);
  output += QStringLiteral("default_power_limit_w: %1\n")
                .arg(snapshot.defaultPowerLimitW, 0, 'f', 1);
  output += QStringLiteral("persistence_mode: %1\n")
                .arg(boolText(snapshot.persistenceModeEnabled));
  output += QStringLiteral("power_preset: %1\n")
                .arg(dashIfEmpty(snapshot.powerPreset));
  output += QStringLiteral("core_clock_offset_mhz: %1\n")
                .arg(snapshot.coreClockOffsetMHz);
  output += QStringLiteral("memory_clock_offset_mhz: %1\n")
                .arg(snapshot.memoryClockOffsetMHz);

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
  object.insert(QStringLiteral("gpuCount"), snapshot.gpuCount);
  object.insert(QStringLiteral("activeDriver"), snapshot.activeDriver);
  object.insert(QStringLiteral("driverVersion"), snapshot.driverVersion);
  object.insert(QStringLiteral("sessionType"), snapshot.sessionType);
  object.insert(QStringLiteral("secureBootEnabled"),
                snapshot.secureBootEnabled);
  object.insert(QStringLiteral("updateAvailable"), snapshot.updateAvailable);
  object.insert(QStringLiteral("latestDriverVersion"),
                snapshot.latestDriverVersion);
  object.insert(QStringLiteral("gpuFanSpeedPercent"),
                snapshot.gpuFanSpeedPercent);
  object.insert(QStringLiteral("fanControlSupported"),
                snapshot.fanControlSupported);
  object.insert(QStringLiteral("fanMode"), snapshot.fanMode);
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
  object.insert(QStringLiteral("gpuHotspotTemperatureC"),
                snapshot.gpuHotspotTemperatureC);
  object.insert(QStringLiteral("gpuMemoryTemperatureC"),
                snapshot.gpuMemoryTemperatureC);
  object.insert(QStringLiteral("gpuUtilizationPercent"),
                snapshot.gpuUtilizationPercent);
  object.insert(QStringLiteral("gpuMemoryUsedMiB"), snapshot.gpuMemoryUsedMiB);
  object.insert(QStringLiteral("gpuMemoryTotalMiB"),
                snapshot.gpuMemoryTotalMiB);
  object.insert(QStringLiteral("gpuMemoryUsagePercent"),
                snapshot.gpuMemoryUsagePercent);
  object.insert(QStringLiteral("gpuFanSpeedPercent"),
                snapshot.gpuFanSpeedPercent);
  object.insert(QStringLiteral("gpuProcessCount"), snapshot.gpuProcessCount);
  object.insert(QStringLiteral("fanSupported"), snapshot.fanSupported);
  object.insert(QStringLiteral("fanControlSupported"),
                snapshot.fanControlSupported);
  object.insert(QStringLiteral("fanCapability"), snapshot.fanCapability);
  object.insert(QStringLiteral("fanHardwareType"), snapshot.fanHardwareType);
  object.insert(QStringLiteral("fanMode"), snapshot.fanMode);
  object.insert(QStringLiteral("fanTargetSpeedPercent"),
                snapshot.fanTargetSpeedPercent);
  object.insert(QStringLiteral("fanRpm"), snapshot.fanRpm);
  object.insert(QStringLiteral("fanSafetyOverride"),
                snapshot.fanSafetyOverride);
  object.insert(QStringLiteral("fanThermalThresholdC"),
                snapshot.fanThermalThresholdC);
  object.insert(QStringLiteral("fanSmoothingEnabled"),
                snapshot.fanSmoothingEnabled);
  object.insert(QStringLiteral("ramAvailable"), snapshot.ramAvailable);
  object.insert(QStringLiteral("ramTotalMiB"), snapshot.ramTotalMiB);
  object.insert(QStringLiteral("ramUsedMiB"), snapshot.ramUsedMiB);
  object.insert(QStringLiteral("ramUsagePercent"), snapshot.ramUsagePercent);
  object.insert(QStringLiteral("powerSupported"), snapshot.powerSupported);
  object.insert(QStringLiteral("powerControlSupported"),
                snapshot.powerControlSupported);
  object.insert(QStringLiteral("powerDrawW"), snapshot.powerDrawW);
  object.insert(QStringLiteral("powerLimitW"), snapshot.powerLimitW);
  object.insert(QStringLiteral("minPowerLimitW"), snapshot.minPowerLimitW);
  object.insert(QStringLiteral("maxPowerLimitW"), snapshot.maxPowerLimitW);
  object.insert(QStringLiteral("defaultPowerLimitW"),
                snapshot.defaultPowerLimitW);
  object.insert(QStringLiteral("persistenceMode"),
                snapshot.persistenceModeEnabled);
  object.insert(QStringLiteral("powerPreset"), snapshot.powerPreset);
  object.insert(QStringLiteral("coreClockOffsetMHz"),
                snapshot.coreClockOffsetMHz);
  object.insert(QStringLiteral("memoryClockOffsetMHz"),
                snapshot.memoryClockOffsetMHz);
  return object;
}

} // namespace RoControlCli
