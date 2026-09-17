#include <QActionGroup>
#include <QApplication>
#include <QCoreApplication>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QStringList>
#include <QTextStream>
#include <QTranslator>
#include <QVariant>
#include <QtConcurrent>
#include <QtQuickControls2/QQuickStyle>

#include <functional>

#include <QMenu>
#include <QSystemTrayIcon>

#include "backend/fan/fancontroller.h"
#include "backend/monitor/cpumonitor.h"
#include "backend/monitor/gpumonitor.h"
#include "backend/monitor/rammonitor.h"
#include "backend/nvidia/detector.h"
#include "backend/nvidia/installer.h"
#include "backend/nvidia/updater.h"
#include "backend/power/powercontroller.h"
#include "backend/system/dbusservice.h"
#include "backend/system/healthguard.h"
#include "backend/system/languagemanager.h"
#include "backend/system/systeminfoprovider.h"
#include "backend/system/uipreferencesmanager.h"
#include "cli/cli.h"

namespace {

struct CliExecutionResult {
  int exitCode = 0;
  QString stdoutText;
  QString stderrText;
};

CliExecutionResult executeCliCommand(const RoControlCli::ParsedCommand &command,
                                     const QString &applicationName,
                                     const QString &applicationVersion) {
  CliExecutionResult result;

  if (command.action == RoControlCli::CommandAction::PrintStatusText ||
      command.action == RoControlCli::CommandAction::PrintStatusJson ||
      command.action == RoControlCli::CommandAction::PrintDiagnosticsText ||
      command.action == RoControlCli::CommandAction::PrintDiagnosticsJson) {
    const auto snapshot =
        RoControlCli::collectDiagnostics(applicationName, applicationVersion);

    if (command.action == RoControlCli::CommandAction::PrintStatusJson) {
      result.stdoutText = QString::fromUtf8(
          QJsonDocument(RoControlCli::renderStatusJsonObject(snapshot))
              .toJson(QJsonDocument::Indented));
    } else if (command.action == RoControlCli::CommandAction::PrintStatusText) {
      result.stdoutText = RoControlCli::renderStatusText(snapshot);
    } else if (command.action ==
               RoControlCli::CommandAction::PrintDiagnosticsJson) {
      result.stdoutText = QString::fromUtf8(
          QJsonDocument(RoControlCli::renderDiagnosticsJsonObject(snapshot))
              .toJson(QJsonDocument::Indented));
    } else {
      result.stdoutText = RoControlCli::renderDiagnosticsText(snapshot);
    }

    return result;
  }

  if (command.action == RoControlCli::CommandAction::PrintFanStatusText ||
      command.action == RoControlCli::CommandAction::PrintFanStatusJson) {
    const auto snapshot =
        RoControlCli::collectDiagnostics(applicationName, applicationVersion);
    if (command.action == RoControlCli::CommandAction::PrintFanStatusJson) {
      QJsonObject obj;
      obj.insert(QStringLiteral("command"), QStringLiteral("fan-status"));
      obj.insert(QStringLiteral("supported"), snapshot.fanSupported);
      obj.insert(QStringLiteral("controlSupported"),
                 snapshot.fanControlSupported);
      obj.insert(QStringLiteral("capability"), snapshot.fanCapability);
      obj.insert(QStringLiteral("hardwareType"), snapshot.fanHardwareType);
      obj.insert(QStringLiteral("gpuFanSpeedPercent"),
                 snapshot.gpuFanSpeedPercent);
      obj.insert(QStringLiteral("fanRpm"), snapshot.fanRpm);
      obj.insert(QStringLiteral("mode"), snapshot.fanMode);
      obj.insert(QStringLiteral("targetFanSpeedPercent"),
                 snapshot.fanTargetSpeedPercent);
      obj.insert(QStringLiteral("safetyOverrideActive"),
                 snapshot.fanSafetyOverride);
      obj.insert(QStringLiteral("thermalThresholdC"),
                 snapshot.fanThermalThresholdC);
      obj.insert(QStringLiteral("gpuTemperatureC"), snapshot.gpuTemperatureC);
      result.stdoutText =
          QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    } else {
      result.stdoutText =
          QStringLiteral("fan_supported: %1\n"
                         "fan_control_supported: %2\n"
                         "fan_capability: %3\n"
                         "fan_hardware_type: %4\n"
                         "gpu_fan_speed_percent: %5%\n"
                         "fan_rpm: %6\n"
                         "fan_mode: %7\n"
                         "fan_target_speed_percent: %8%\n"
                         "gpu_temperature_c: %9 C\n"
                         "thermal_threshold_c: %10 C\n"
                         "safety_override: %11\n")
              .arg(snapshot.fanSupported ? QStringLiteral("yes")
                                         : QStringLiteral("no"))
              .arg(snapshot.fanControlSupported ? QStringLiteral("yes")
                                                : QStringLiteral("no"))
              .arg(snapshot.fanCapability.isEmpty()
                       ? QStringLiteral("unsupported")
                       : snapshot.fanCapability)
              .arg(snapshot.fanHardwareType.isEmpty()
                       ? QStringLiteral("None")
                       : snapshot.fanHardwareType)
              .arg(snapshot.gpuFanSpeedPercent)
              .arg(snapshot.fanRpm)
              .arg(snapshot.fanMode.isEmpty() ? QStringLiteral("auto")
                                              : snapshot.fanMode)
              .arg(snapshot.fanTargetSpeedPercent)
              .arg(snapshot.gpuTemperatureC)
              .arg(snapshot.fanThermalThresholdC)
              .arg(snapshot.fanSafetyOverride ? QStringLiteral("active")
                                              : QStringLiteral("inactive"));
    }
    return result;
  }

  if (command.action == RoControlCli::CommandAction::FanSetSpeed) {
    FanController fanController;
    fanController.stop();
    if (!fanController.controlSupported()) {
      result.stderrText = QStringLiteral(
          "Error: Fan control is unsupported or read-only on this hardware.\n");
      result.exitCode = 1;
      return result;
    }
    const int speed = command.payload.toInt();
    fanController.setFanMode(QStringLiteral("manual"));
    fanController.setManualFanSpeedPercent(speed);
    result.stdoutText =
        QStringLiteral("Fan speed set to %1% (manual mode).\n").arg(speed);
    result.exitCode = 0;
    return result;
  }

  if (command.action == RoControlCli::CommandAction::FanSetMode) {
    FanController fanController;
    fanController.stop();
    if (command.payload != QStringLiteral("auto") &&
        !fanController.controlSupported()) {
      result.stderrText = QStringLiteral(
          "Error: Fan control is unsupported or read-only on this hardware.\n");
      result.exitCode = 1;
      return result;
    }
    fanController.setFanMode(command.payload);
    result.stdoutText =
        QStringLiteral("Fan mode set to '%1'.\n").arg(command.payload);
    result.exitCode = 0;
    return result;
  }

  if (command.action == RoControlCli::CommandAction::FanReset) {
    FanController fanController;
    fanController.stop();
    fanController.resetToAuto();
    result.stdoutText =
        QStringLiteral("Fan control reset to automatic driver/VBIOS mode.\n");
    result.exitCode = 0;
    return result;
  }

  if (command.action == RoControlCli::CommandAction::PrintPowerStatusText ||
      command.action == RoControlCli::CommandAction::PrintPowerStatusJson) {
    const auto snapshot =
        RoControlCli::collectDiagnostics(applicationName, applicationVersion);
    if (command.action == RoControlCli::CommandAction::PrintPowerStatusJson) {
      QJsonObject obj;
      obj.insert(QStringLiteral("command"), QStringLiteral("power-status"));
      obj.insert(QStringLiteral("supported"), snapshot.powerSupported);
      obj.insert(QStringLiteral("controlSupported"),
                 snapshot.powerControlSupported);
      obj.insert(QStringLiteral("powerDrawW"), snapshot.powerDrawW);
      obj.insert(QStringLiteral("powerLimitW"), snapshot.powerLimitW);
      obj.insert(QStringLiteral("minPowerLimitW"), snapshot.minPowerLimitW);
      obj.insert(QStringLiteral("maxPowerLimitW"), snapshot.maxPowerLimitW);
      obj.insert(QStringLiteral("defaultPowerLimitW"),
                 snapshot.defaultPowerLimitW);
      obj.insert(QStringLiteral("persistenceMode"),
                 snapshot.persistenceModeEnabled);
      obj.insert(QStringLiteral("powerPreset"), snapshot.powerPreset);
      result.stdoutText =
          QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    } else {
      result.stdoutText =
          QStringLiteral("power_supported: %1\n"
                         "power_control_supported: %2\n"
                         "power_draw_w: %3 W\n"
                         "power_limit_w: %4 W\n"
                         "min_power_limit_w: %5 W\n"
                         "max_power_limit_w: %6 W\n"
                         "default_power_limit_w: %7 W\n"
                         "persistence_mode: %8\n"
                         "power_preset: %9\n")
              .arg(snapshot.powerSupported ? QStringLiteral("yes")
                                           : QStringLiteral("no"))
              .arg(snapshot.powerControlSupported ? QStringLiteral("yes")
                                                  : QStringLiteral("no"))
              .arg(snapshot.powerDrawW, 0, 'f', 1)
              .arg(snapshot.powerLimitW, 0, 'f', 1)
              .arg(snapshot.minPowerLimitW, 0, 'f', 1)
              .arg(snapshot.maxPowerLimitW, 0, 'f', 1)
              .arg(snapshot.defaultPowerLimitW, 0, 'f', 1)
              .arg(snapshot.persistenceModeEnabled ? QStringLiteral("enabled")
                                                   : QStringLiteral("disabled"))
              .arg(snapshot.powerPreset.isEmpty() ? QStringLiteral("balanced")
                                                  : snapshot.powerPreset);
    }
    return result;
  }

  if (command.action == RoControlCli::CommandAction::PowerSetLimit) {
    PowerController powerController;
    powerController.refresh();
    if (!powerController.controlSupported()) {
      result.stderrText =
          QStringLiteral("Error: GPU power control is unsupported or read-only "
                         "on this hardware.\n");
      result.exitCode = 1;
      return result;
    }
    const double watts = command.payload.toDouble();
    if (powerController.setPowerLimit(watts)) {
      result.stdoutText = QStringLiteral("GPU power limit set to %1 W.\n")
                              .arg(watts, 0, 'f', 0);
      result.exitCode = 0;
    } else {
      result.stderrText =
          QStringLiteral("Error: %1\n").arg(powerController.statusMessage());
      result.exitCode = 1;
    }
    return result;
  }

  if (command.action == RoControlCli::CommandAction::PowerSetPreset) {
    PowerController powerController;
    powerController.refresh();
    if (powerController.applyPowerPreset(command.payload)) {
      result.stdoutText =
          QStringLiteral("Power preset set to '%1'.\n").arg(command.payload);
      result.exitCode = 0;
    } else {
      result.stderrText =
          QStringLiteral("Error: %1\n").arg(powerController.statusMessage());
      result.exitCode = 1;
    }
    return result;
  }

  if (command.action == RoControlCli::CommandAction::PowerSetPersistence) {
    PowerController powerController;
    powerController.refresh();
    const bool enable = (command.payload == QStringLiteral("1"));
    if (powerController.setPersistenceMode(enable)) {
      result.stdoutText = QStringLiteral("Persistence mode set to %1.\n")
                              .arg(enable ? QStringLiteral("enabled")
                                          : QStringLiteral("disabled"));
      result.exitCode = 0;
    } else {
      result.stderrText =
          QStringLiteral("Error: %1\n").arg(powerController.statusMessage());
      result.exitCode = 1;
    }
    return result;
  }

  if (command.action == RoControlCli::CommandAction::PowerSetClocks) {
    PowerController powerController;
    powerController.refresh();
    if (!powerController.controlSupported()) {
      result.stderrText = QStringLiteral(
          "Error: GPU clock control is unsupported or read-only.\n");
      result.exitCode = 1;
      return result;
    }
    const QStringList parts = command.payload.split(QLatin1Char(':'));
    const int core = parts.value(0).toInt();
    const int mem = parts.value(1).toInt();
    if (powerController.setClockOffsets(core, mem)) {
      const QString coreSign = core >= 0 ? QStringLiteral("+") : QString();
      const QString memSign = mem >= 0 ? QStringLiteral("+") : QString();
      result.stdoutText =
          QStringLiteral(
              "GPU clock offsets set to Core: %1%2 MHz, Memory: %3%4 MHz.\n")
              .arg(coreSign)
              .arg(core)
              .arg(memSign)
              .arg(mem);
      result.exitCode = 0;
    } else {
      result.stderrText =
          QStringLiteral("Error: %1\n").arg(powerController.statusMessage());
      result.exitCode = 1;
    }
    return result;
  }

  if (command.action == RoControlCli::CommandAction::FanSetSmoothing) {
    FanController fanController;
    fanController.stop();
    const QStringList parts = command.payload.split(QLatin1Char(':'));
    const bool on = (parts.value(0) == QStringLiteral("1"));
    fanController.setSmoothingEnabled(on);
    if (parts.size() >= 2 && !parts.at(1).isEmpty()) {
      fanController.setRampUpRatePercent(parts.at(1).toInt());
    }
    if (parts.size() >= 3 && !parts.at(2).isEmpty()) {
      fanController.setRampDownRatePercent(parts.at(2).toInt());
    }
    if (parts.size() >= 4 && !parts.at(3).isEmpty()) {
      fanController.setHysteresisTempC(parts.at(3).toInt());
    }
    result.stdoutText =
        QStringLiteral("Fan smoothing %1 (RampUp: %2%/s, RampDown: %3%/s, "
                       "Hysteresis: %4 C).\n")
            .arg(on ? QStringLiteral("enabled") : QStringLiteral("disabled"))
            .arg(fanController.rampUpRatePercent())
            .arg(fanController.rampDownRatePercent())
            .arg(fanController.hysteresisTempC());
    result.exitCode = 0;
    return result;
  }

  if (command.action == RoControlCli::CommandAction::PrintProcessesText ||
      command.action == RoControlCli::CommandAction::PrintProcessesJson) {
    GpuMonitor gpuMonitor;
    gpuMonitor.stop();
    gpuMonitor.refresh();
    const auto procs = gpuMonitor.gpuProcesses();
    if (command.action == RoControlCli::CommandAction::PrintProcessesJson) {
      QJsonObject obj;
      QJsonArray arr;
      for (const auto &p : procs) {
        arr.append(QJsonObject::fromVariantMap(p.toMap()));
      }
      obj.insert(QStringLiteral("command"), QStringLiteral("processes"));
      obj.insert(QStringLiteral("count"), procs.size());
      obj.insert(QStringLiteral("processes"), arr);
      result.stdoutText =
          QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    } else {
      if (procs.isEmpty()) {
        result.stdoutText =
            QStringLiteral("No active GPU processes detected.\n");
      } else {
        result.stdoutText = QStringLiteral("PID\tNAME\t\t\tTYPE\t\t\tVRAM\n");
        result.stdoutText +=
            QStringLiteral("───────────────────────────────────────────────────"
                           "────────────────\n");
        for (const auto &p : procs) {
          const auto m = p.toMap();
          result.stdoutText +=
              QStringLiteral("%1\t%2\t%3\t%4 MiB\n")
                  .arg(m.value(QStringLiteral("pid")).toInt(), -6)
                  .arg(m.value(QStringLiteral("name")).toString(), -20)
                  .arg(m.value(QStringLiteral("type")).toString(), -20)
                  .arg(m.value(QStringLiteral("vramMiB")).toInt());
        }
      }
    }
    return result;
  }

  if (command.action == RoControlCli::CommandAction::KillProcess) {
    GpuMonitor gpuMonitor;
    gpuMonitor.stop();
    const int pid = command.payload.toInt();
    if (gpuMonitor.killProcess(pid)) {
      result.stdoutText =
          QStringLiteral("Process %1 terminated successfully.\n").arg(pid);
      result.exitCode = 0;
    } else {
      result.stderrText =
          QStringLiteral("Failed to terminate process %1.\n").arg(pid);
      result.exitCode = 1;
    }
    return result;
  }

  if (command.action == RoControlCli::CommandAction::PrintGpusText ||
      command.action == RoControlCli::CommandAction::PrintGpusJson) {
    GpuMonitor gpuMonitor;
    gpuMonitor.stop();
    gpuMonitor.refresh();
    const auto gpus = gpuMonitor.gpuDevices();
    if (command.action == RoControlCli::CommandAction::PrintGpusJson) {
      QJsonObject obj;
      QJsonArray arr;
      for (const auto &g : gpus) {
        arr.append(QJsonObject::fromVariantMap(g.toMap()));
      }
      obj.insert(QStringLiteral("command"), QStringLiteral("gpus"));
      obj.insert(QStringLiteral("count"), gpus.size());
      obj.insert(QStringLiteral("selectedIndex"),
                 gpuMonitor.selectedGpuIndex());
      obj.insert(QStringLiteral("devices"), arr);
      result.stdoutText =
          QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    } else {
      result.stdoutText =
          QStringLiteral("INDEX\tNAME\t\t\t\tPCI BUS ID\t\tUUID\n");
      result.stdoutText +=
          QStringLiteral("─────────────────────────────────────────────────────"
                         "────────────────────────\n");
      for (const auto &g : gpus) {
        const auto m = g.toMap();
        const int idx = m.value(QStringLiteral("index")).toInt();
        const QString star = (idx == gpuMonitor.selectedGpuIndex())
                                 ? QStringLiteral("*")
                                 : QStringLiteral(" ");
        result.stdoutText +=
            QStringLiteral("%1%2\t%3\t%4\t%5\n")
                .arg(star)
                .arg(idx, -4)
                .arg(m.value(QStringLiteral("name")).toString(), -30)
                .arg(m.value(QStringLiteral("pciBusId")).toString(), -16)
                .arg(m.value(QStringLiteral("uuid")).toString());
      }
    }
    return result;
  }

  if (command.action == RoControlCli::CommandAction::SelectGpu) {
    GpuMonitor gpuMonitor;
    gpuMonitor.stop();
    const int idx = command.payload.toInt();
    gpuMonitor.setSelectedGpuIndex(idx);
    result.stdoutText =
        QStringLiteral("Active GPU set to index %1.\n").arg(idx);
    result.exitCode = 0;
    return result;
  }

  QTextStream progressStream(&result.stdoutText);
  auto appendProgress = [&](const QString &message) {
    if (!message.trimmed().isEmpty()) {
      progressStream << message.trimmed() << '\n';
    }
  };

  if (command.action == RoControlCli::CommandAction::InstallProprietaryDriver ||
      command.action == RoControlCli::CommandAction::InstallOpenSourceDriver ||
      command.action == RoControlCli::CommandAction::RemoveDriver ||
      command.action == RoControlCli::CommandAction::DeepCleanDriver) {
    NvidiaInstaller installer;
    QObject::connect(&installer, &NvidiaInstaller::progressMessage, &installer,
                     appendProgress);

    bool finished = false;
    bool success = false;
    QString finalMessage;
    QEventLoop loop;

    QObject::connect(&installer, &NvidiaInstaller::installFinished, &installer,
                     [&](bool ok, const QString &message) {
                       finished = true;
                       success = ok;
                       finalMessage = message;
                       loop.quit();
                     });
    QObject::connect(&installer, &NvidiaInstaller::removeFinished, &installer,
                     [&](bool ok, const QString &message) {
                       finished = true;
                       success = ok;
                       finalMessage = message;
                       loop.quit();
                     });

    if (command.action ==
        RoControlCli::CommandAction::InstallProprietaryDriver) {
      installer.installProprietary(command.acceptLicense);
    } else if (command.action ==
               RoControlCli::CommandAction::InstallOpenSourceDriver) {
      installer.installOpenSource();
    } else if (command.action == RoControlCli::CommandAction::RemoveDriver) {
      installer.remove();
    } else {
      installer.deepClean();
    }

    if (!finished) {
      loop.exec();
    }

    if (!finalMessage.isEmpty()) {
      if (success) {
        appendProgress(finalMessage);
      } else {
        result.stderrText = finalMessage;
      }
    }

    result.exitCode = finished && success ? 0 : 1;
    return result;
  }

  if (command.action == RoControlCli::CommandAction::UpdateDriver) {
    NvidiaUpdater updater;
    QObject::connect(&updater, &NvidiaUpdater::progressMessage, &updater,
                     appendProgress);

    bool finished = false;
    bool success = false;
    QString finalMessage;
    QEventLoop loop;

    QObject::connect(&updater, &NvidiaUpdater::updateFinished, &updater,
                     [&](bool ok, const QString &message) {
                       finished = true;
                       success = ok;
                       finalMessage = message;
                       loop.quit();
                     });

    updater.applyUpdate();

    if (!finished) {
      loop.exec();
    }

    if (!finalMessage.isEmpty()) {
      if (success) {
        appendProgress(finalMessage);
      } else {
        result.stderrText = finalMessage;
      }
    }

    result.exitCode = finished && success ? 0 : 1;
    return result;
  }

  result.exitCode = 2;
  result.stderrText = QStringLiteral("Unsupported CLI command.");
  return result;
}

void configureGuiGraphicsEnvironment() {
#if defined(Q_OS_LINUX)
  // ro-Control is a driver management tool, not a GPU-accelerated UI. Using
  // Qt Quick's software renderer on Linux avoids EGL/DRI startup warnings and
  // keeps the app usable while the graphics driver stack is broken or changing.
  if (qEnvironmentVariableIsEmpty("RO_CONTROL_USE_HARDWARE_RENDER")) {
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
      qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("wayland"));
    }
    qputenv("QT_QUICK_BACKEND", QByteArrayLiteral("software"));
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
    return;
  }

  // Keep an explicit escape hatch for hosts that still need software rendering.
  if (!qEnvironmentVariableIsEmpty("RO_CONTROL_FORCE_SOFTWARE_RENDER")) {
    qputenv("QT_QUICK_BACKEND", QByteArrayLiteral("software"));
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
  }
#endif
}

} // namespace

int main(int argc, char *argv[]) {
  constexpr auto kApplicationName = "ro-control";
  constexpr auto kDisplayName = "ro-Control";
  constexpr auto kApplicationVersion = RO_CONTROL_APP_VERSION;
  const QString applicationDescription =
      QStringLiteral("ro-Control GPU driver manager and diagnostics CLI.");

  QStringList arguments;
  arguments.reserve(argc);
  for (int i = 0; i < argc; ++i) {
    arguments << QString::fromLocal8Bit(argv[i]);
  }

  const auto command = RoControlCli::parseArguments(
      arguments, QString::fromLatin1(kApplicationName),
      QString::fromLatin1(kApplicationVersion), applicationDescription);

  QTextStream out(stdout);
  QTextStream err(stderr);

  if (command.action == RoControlCli::CommandAction::PrintHelp ||
      command.action == RoControlCli::CommandAction::PrintVersion) {
    out << command.payload;
    if (!command.payload.endsWith(QLatin1Char('\n'))) {
      out << Qt::endl;
    }
    return 0;
  }

  if (command.action == RoControlCli::CommandAction::Invalid) {
    err << command.payload << Qt::endl;
    err << "Run `ro-control --help` for usage." << Qt::endl;
    return 2;
  }

  if (command.action == RoControlCli::CommandAction::RunDaemon) {
    QCoreApplication daemonApp(argc, argv);
    daemonApp.setApplicationName(QString::fromLatin1(kApplicationName));
    daemonApp.setApplicationVersion(QString::fromLatin1(kApplicationVersion));

    CpuMonitor cpuMonitor;
    GpuMonitor gpuMonitor;
    RamMonitor ramMonitor;
    FanController fanController;
    PowerController powerController;
    HealthGuard healthGuard;

    QObject::connect(
        &gpuMonitor, &GpuMonitor::temperatureCChanged, &fanController,
        [&]() { fanController.updateTemperature(gpuMonitor.temperatureC()); });
    QObject::connect(
        &cpuMonitor, &CpuMonitor::temperatureCChanged, &fanController, [&]() {
          fanController.updateCpuTemperature(cpuMonitor.temperatureC());
        });
    QObject::connect(
        &gpuMonitor, &GpuMonitor::temperatureCChanged, &healthGuard,
        [&]() { healthGuard.updateGpuTemperature(gpuMonitor.temperatureC()); });
    QObject::connect(
        &cpuMonitor, &CpuMonitor::temperatureCChanged, &healthGuard,
        [&]() { healthGuard.updateCpuTemperature(cpuMonitor.temperatureC()); });
    QObject::connect(
        &gpuMonitor, &GpuMonitor::powerDrawWChanged, &powerController,
        [&]() { powerController.updatePowerDraw(gpuMonitor.powerDrawW()); });

    RoControlDBusService dbusService(&daemonApp, &cpuMonitor, &gpuMonitor,
                                     &ramMonitor, &fanController,
                                     &powerController, &healthGuard);

    out << "ro-Control daemon active. D-Bus interface: "
           "io.github.ProjectRoASD.rocontrol"
        << Qt::endl;
    return daemonApp.exec();
  }

  if (command.action != RoControlCli::CommandAction::LaunchGui) {
    QCoreApplication cliApp(argc, argv);
    cliApp.setApplicationName(QString::fromLatin1(kApplicationName));
    cliApp.setApplicationVersion(QString::fromLatin1(kApplicationVersion));

    const auto result = executeCliCommand(command, cliApp.applicationName(),
                                          cliApp.applicationVersion());
    if (!result.stdoutText.isEmpty()) {
      out << result.stdoutText;
      if (!result.stdoutText.endsWith(QLatin1Char('\n'))) {
        out << Qt::endl;
      }
    }
    if (!result.stderrText.isEmpty()) {
      err << result.stderrText;
      if (!result.stderrText.endsWith(QLatin1Char('\n'))) {
        err << Qt::endl;
      }
    }
    return result.exitCode;
  }

  configureGuiGraphicsEnvironment();

  if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
    QQuickStyle::setStyle(QStringLiteral("Basic"));
  }

  QApplication app(argc, argv);

  app.setApplicationName(QString::fromLatin1(kApplicationName));
  app.setApplicationDisplayName(QString::fromLatin1(kDisplayName));
  app.setApplicationVersion(QString::fromLatin1(kApplicationVersion));
  app.setOrganizationName("Project-Ro-ASD");
  app.setOrganizationDomain("github.com/Project-Ro-ASD");
  app.setWindowIcon(QIcon::fromTheme(
      "ro-control", QIcon(":/qt/qml/rocontrol/assets/ro-control-logo.svg")));

  QTranslator translator;
  NvidiaDetector detector;
  NvidiaInstaller installer;
  NvidiaUpdater updater;
  CpuMonitor cpuMonitor;
  GpuMonitor gpuMonitor;
  RamMonitor ramMonitor;
  FanController fanController;
  PowerController powerController;
  HealthGuard healthGuard;
  SystemInfoProvider systemInfo;

  RoControlDBusService dbusService(&app, &cpuMonitor, &gpuMonitor, &ramMonitor,
                                   &fanController, &powerController,
                                   &healthGuard);

  QObject::connect(
      &gpuMonitor, &GpuMonitor::temperatureCChanged, &fanController,
      [&]() { fanController.updateTemperature(gpuMonitor.temperatureC()); });

  QObject::connect(
      &cpuMonitor, &CpuMonitor::temperatureCChanged, &fanController,
      [&]() { fanController.updateCpuTemperature(cpuMonitor.temperatureC()); });

  QObject::connect(
      &gpuMonitor, &GpuMonitor::powerDrawWChanged, &powerController,
      [&]() { powerController.updatePowerDraw(gpuMonitor.powerDrawW()); });

  QObject::connect(
      &gpuMonitor, &GpuMonitor::fanSpeedPercentChanged, &fanController,
      [&]() { fanController.updateGpuFanSpeed(gpuMonitor.fanSpeedPercent()); });

  QObject::connect(&gpuMonitor, &GpuMonitor::thermalLimitTemperatureCChanged,
                   &fanController, [&]() {
                     fanController.updateGpuThermalLimit(
                         gpuMonitor.thermalLimitTemperatureC());
                   });

  QObject::connect(
      &gpuMonitor, &GpuMonitor::temperatureCChanged, &healthGuard,
      [&]() { healthGuard.updateGpuTemperature(gpuMonitor.temperatureC()); });

  QObject::connect(
      &cpuMonitor, &CpuMonitor::temperatureCChanged, &healthGuard,
      [&]() { healthGuard.updateCpuTemperature(cpuMonitor.temperatureC()); });

  auto *detectionWatcher = new QFutureWatcher<NvidiaDetector::GpuInfo>(&app);
  detectionWatcher->setFuture(
      QtConcurrent::run([&detector] { return detector.detect(); }));
  QObject::connect(detectionWatcher,
                   &QFutureWatcher<NvidiaDetector::GpuInfo>::finished,
                   detectionWatcher, [&detector, detectionWatcher]() {
                     detector.setDetectionResult(detectionWatcher->result());
                     detectionWatcher->deleteLater();
                   });

  QQmlApplicationEngine engine;
  LanguageManager languageManager(&app, &engine, &translator);
  UiPreferencesManager uiPreferencesManager;

  // System Tray Setup
  QSystemTrayIcon trayIcon(
      QIcon::fromTheme(QStringLiteral("ro-control"),
                       QIcon(QStringLiteral(
                           ":/qt/qml/rocontrol/assets/ro-control-logo.svg"))),
      &app);

  auto updateTrayTooltip = [&]() {
    const QString tooltip =
        QStringLiteral("ro-Control\nGPU: %1°C | Fan: %2 RPM\nPower: %3 W")
            .arg(gpuMonitor.temperatureC())
            .arg(fanController.currentRpm())
            .arg(gpuMonitor.powerDrawW(), 0, 'f', 1);
    trayIcon.setToolTip(tooltip);
  };

  QObject::connect(&gpuMonitor, &GpuMonitor::temperatureCChanged, &trayIcon,
                   updateTrayTooltip);
  QObject::connect(&fanController, &FanController::currentRpmChanged, &trayIcon,
                   updateTrayTooltip);

  QMenu trayMenu;
  auto *restoreAction = trayMenu.addAction(
      QCoreApplication::translate("Main", "Open ro-Control"));
  trayMenu.addSeparator();

  auto *fanMenu =
      trayMenu.addMenu(QCoreApplication::translate("Main", "Fan Profile"));
  auto *autoFanAction =
      fanMenu->addAction(QCoreApplication::translate("Main", "Auto"));
  auto *silentFanAction =
      fanMenu->addAction(QCoreApplication::translate("Main", "Silent"));
  auto *balancedFanAction =
      fanMenu->addAction(QCoreApplication::translate("Main", "Balanced"));
  auto *perfFanAction =
      fanMenu->addAction(QCoreApplication::translate("Main", "Performance"));
  auto *fanActionGroup = new QActionGroup(&trayMenu);
  fanActionGroup->setExclusive(true);
  for (QAction *action :
       {autoFanAction, silentFanAction, balancedFanAction, perfFanAction}) {
    action->setCheckable(true);
    fanActionGroup->addAction(action);
  }

  QObject::connect(autoFanAction, &QAction::triggered, &fanController,
                   [&]() { fanController.setFanMode(QStringLiteral("auto")); });
  QObject::connect(silentFanAction, &QAction::triggered, &fanController, [&]() {
    fanController.setFanMode(QStringLiteral("silent"));
  });
  QObject::connect(
      balancedFanAction, &QAction::triggered, &fanController,
      [&]() { fanController.setFanMode(QStringLiteral("balanced")); });
  QObject::connect(perfFanAction, &QAction::triggered, &fanController, [&]() {
    fanController.setFanMode(QStringLiteral("performance"));
  });

  auto *powerMenu =
      trayMenu.addMenu(QCoreApplication::translate("Main", "Power Preset"));
  auto *ecoPowerAction =
      powerMenu->addAction(QCoreApplication::translate("Main", "Eco"));
  auto *balancedPowerAction =
      powerMenu->addAction(QCoreApplication::translate("Main", "Balanced"));
  auto *perfPowerAction =
      powerMenu->addAction(QCoreApplication::translate("Main", "Performance"));
  auto *powerActionGroup = new QActionGroup(&trayMenu);
  powerActionGroup->setExclusive(true);
  for (QAction *action :
       {ecoPowerAction, balancedPowerAction, perfPowerAction}) {
    action->setCheckable(true);
    powerActionGroup->addAction(action);
  }

  QObject::connect(
      ecoPowerAction, &QAction::triggered, &powerController,
      [&]() { powerController.applyPowerPreset(QStringLiteral("eco")); });
  QObject::connect(
      balancedPowerAction, &QAction::triggered, &powerController,
      [&]() { powerController.applyPowerPreset(QStringLiteral("balanced")); });
  QObject::connect(
      perfPowerAction, &QAction::triggered, &powerController, [&]() {
        powerController.applyPowerPreset(QStringLiteral("performance"));
      });

  auto syncTrayProfiles = [&]() {
    const QString fanMode = fanController.fanMode();
    autoFanAction->setChecked(fanMode == QStringLiteral("auto"));
    silentFanAction->setChecked(fanMode == QStringLiteral("silent"));
    balancedFanAction->setChecked(fanMode == QStringLiteral("balanced"));
    perfFanAction->setChecked(fanMode == QStringLiteral("performance"));

    const QString powerPreset = powerController.powerPreset();
    ecoPowerAction->setChecked(powerPreset == QStringLiteral("eco"));
    balancedPowerAction->setChecked(powerPreset == QStringLiteral("balanced"));
    perfPowerAction->setChecked(powerPreset == QStringLiteral("performance"));
    fanMenu->setTitle(QCoreApplication::translate("Main", "Fan Profile: %1")
                          .arg(fanMode.toUpper()));
    powerMenu->setTitle(QCoreApplication::translate("Main", "Power Preset: %1")
                            .arg(powerPreset.toUpper()));
  };
  QObject::connect(&fanController, &FanController::fanModeChanged, &trayMenu,
                   syncTrayProfiles);
  QObject::connect(&powerController, &PowerController::powerPresetChanged,
                   &trayMenu, syncTrayProfiles);
  syncTrayProfiles();

  trayMenu.addSeparator();
  auto *quitAction =
      trayMenu.addAction(QCoreApplication::translate("Main", "Quit"));
  QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);

  trayIcon.setContextMenu(&trayMenu);

  QObject::connect(restoreAction, &QAction::triggered, [&engine]() {
    const auto rootObjects = engine.rootObjects();
    if (!rootObjects.isEmpty()) {
      if (auto *window = qobject_cast<QQuickWindow *>(rootObjects.first())) {
        window->show();
        window->raise();
        window->requestActivate();
      }
    }
  });

  QObject::connect(&trayIcon, &QSystemTrayIcon::activated,
                   [&engine](QSystemTrayIcon::ActivationReason reason) {
                     if (reason == QSystemTrayIcon::Trigger ||
                         reason == QSystemTrayIcon::DoubleClick) {
                       const auto rootObjects = engine.rootObjects();
                       if (!rootObjects.isEmpty()) {
                         if (auto *window = qobject_cast<QQuickWindow *>(
                                 rootObjects.first())) {
                           if (window->isVisible()) {
                             window->raise();
                             window->requestActivate();
                           } else {
                             window->show();
                           }
                         }
                       }
                     }
                   });

  QObject::connect(&healthGuard, &HealthGuard::thermalAlertTriggered, &trayIcon,
                   [&](const QString &title, const QString &message,
                       const QString &severity) {
                     const auto iconType =
                         (severity == QStringLiteral("critical"))
                             ? QSystemTrayIcon::Critical
                             : QSystemTrayIcon::Warning;
                     trayIcon.showMessage(title, message, iconType, 5000);
                   });

  if (QSystemTrayIcon::isSystemTrayAvailable()) {
    trayIcon.show();
  }

  QVariantMap initialProperties;
  initialProperties.insert(QStringLiteral("nvidiaDetector"),
                           QVariant::fromValue(&detector));
  initialProperties.insert(QStringLiteral("nvidiaInstaller"),
                           QVariant::fromValue(&installer));
  initialProperties.insert(QStringLiteral("nvidiaUpdater"),
                           QVariant::fromValue(&updater));
  initialProperties.insert(QStringLiteral("cpuMonitor"),
                           QVariant::fromValue(&cpuMonitor));
  initialProperties.insert(QStringLiteral("gpuMonitor"),
                           QVariant::fromValue(&gpuMonitor));
  initialProperties.insert(QStringLiteral("ramMonitor"),
                           QVariant::fromValue(&ramMonitor));
  initialProperties.insert(QStringLiteral("fanController"),
                           QVariant::fromValue(&fanController));
  initialProperties.insert(QStringLiteral("powerController"),
                           QVariant::fromValue(&powerController));
  initialProperties.insert(QStringLiteral("healthGuard"),
                           QVariant::fromValue(&healthGuard));
  initialProperties.insert(QStringLiteral("systemInfo"),
                           QVariant::fromValue(&systemInfo));
  initialProperties.insert(QStringLiteral("languageManager"),
                           QVariant::fromValue(&languageManager));
  initialProperties.insert(QStringLiteral("uiPreferences"),
                           QVariant::fromValue(&uiPreferencesManager));
  engine.setInitialProperties(initialProperties);

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  engine.loadFromModule("rocontrol", "Main");

  const auto rootObjects = engine.rootObjects();
  if (!rootObjects.isEmpty()) {
    if (auto *window = qobject_cast<QQuickWindow *>(rootObjects.first())) {
      // Desynchronize the 1s telemetry timers so they never fire back-to-back
      // inside the same event-loop iteration / QML frame.
      cpuMonitor.stop();
      gpuMonitor.stop();
      ramMonitor.stop();
      QTimer::singleShot(0, &gpuMonitor, &GpuMonitor::start);
      QTimer::singleShot(333, &cpuMonitor, &CpuMonitor::start);
      QTimer::singleShot(666, &ramMonitor, &RamMonitor::start);

      // Throttle telemetry to 5s when the window is hidden (tray mode); the
      // daemon path never touches these monitors, so D-Bus clients keep a
      // full 1s cadence.
      const QList<std::function<void(int)>> throttlers{
          [&](int ms) { cpuMonitor.setUpdateInterval(ms); },
          [&](int ms) { gpuMonitor.setUpdateInterval(ms); },
          [&](int ms) { ramMonitor.setUpdateInterval(ms); }};
      const auto applyVisibility = [throttlers](bool visible) {
        for (const auto &throttler : throttlers) {
          throttler(visible ? 1000 : 5000);
        }
      };
      QObject::connect(
          window, &QQuickWindow::visibleChanged, window,
          [applyVisibility](bool visible) { applyVisibility(visible); });
      applyVisibility(window->isVisible());
    }
  }

  return app.exec();
}
