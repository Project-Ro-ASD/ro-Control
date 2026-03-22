#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTextStream>
#include <QTranslator>
#include <QVariant>

#include "backend/monitor/cpumonitor.h"
#include "backend/monitor/gpumonitor.h"
#include "backend/monitor/rammonitor.h"
#include "backend/nvidia/detector.h"
#include "backend/nvidia/installer.h"
#include "backend/nvidia/updater.h"
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

    QObject::connect(&installer, &NvidiaInstaller::installFinished, &installer,
                     [&](bool ok, const QString &message) {
                       finished = true;
                       success = ok;
                       finalMessage = message;
                     });
    QObject::connect(&installer, &NvidiaInstaller::removeFinished, &installer,
                     [&](bool ok, const QString &message) {
                       finished = true;
                       success = ok;
                       finalMessage = message;
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
      finished = true;
      success = true;
      finalMessage = QStringLiteral("Legacy NVIDIA cleanup completed.");
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

    QObject::connect(&updater, &NvidiaUpdater::updateFinished, &updater,
                     [&](bool ok, const QString &message) {
                       finished = true;
                       success = ok;
                       finalMessage = message;
                     });

    updater.applyUpdate();

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

} // namespace

int main(int argc, char *argv[]) {
  constexpr auto kApplicationName = "ro-control";
  constexpr auto kDisplayName = "ro-Control";
  constexpr auto kApplicationVersion = "0.1.0";
  const QString applicationDescription =
      QStringLiteral("ro-Control GPU driver manager and diagnostics CLI.");

  {
    QCoreApplication cliApp(argc, argv);
    cliApp.setApplicationName(QString::fromLatin1(kApplicationName));
    cliApp.setApplicationVersion(QString::fromLatin1(kApplicationVersion));

    const auto command = RoControlCli::parseArguments(
        cliApp.arguments(), cliApp.applicationName(),
        cliApp.applicationVersion(), applicationDescription);

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

    if (command.action != RoControlCli::CommandAction::LaunchGui) {
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
  const QString localeName = QLocale::system().name();
  const QString baseLanguage =
      localeName.section(QLatin1Char('_'), 0, 0).toLower();

  if (translator.load(
          QStringLiteral(":/i18n/ro-control_%1.qm").arg(localeName)) ||
      translator.load(
          QStringLiteral(":/i18n/ro-control_%1.qm").arg(baseLanguage))) {
    app.installTranslator(&translator);
  }

  NvidiaDetector detector;
  NvidiaInstaller installer;
  NvidiaUpdater updater;
  CpuMonitor cpuMonitor;
  GpuMonitor gpuMonitor;
  RamMonitor ramMonitor;

  QQmlApplicationEngine engine;

  // Backend nesnelerini tüm QML dosyalarına global olarak aç
  engine.rootContext()->setContextProperty("nvidiaDetector", &detector);
  engine.rootContext()->setContextProperty("nvidiaInstaller", &installer);
  engine.rootContext()->setContextProperty("nvidiaUpdater", &updater);
  engine.rootContext()->setContextProperty("cpuMonitor", &cpuMonitor);
  engine.rootContext()->setContextProperty("gpuMonitor", &gpuMonitor);
  engine.rootContext()->setContextProperty("ramMonitor", &ramMonitor);

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  engine.loadFromModule("rocontrol", "Main");

  return app.exec();
}
