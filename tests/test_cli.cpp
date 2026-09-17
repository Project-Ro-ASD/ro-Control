#include <QJsonObject>
#include <QTest>

#include "cli/cli.h"

namespace {

const QString kAppVersion = QStringLiteral("1.3.0");

}

class TestCli : public QObject {
  Q_OBJECT

private slots:
  void testHelpOption() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("--help")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintHelp);
    QVERIFY(command.payload.contains(QStringLiteral("driver install")));
    QVERIFY(command.payload.contains(QStringLiteral("status [--json]")));
  }

  void testVersionOption() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("--version")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintVersion);
    QCOMPARE(command.payload, kAppVersion);
  }

  void testJsonRequiresDiagnostics() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("--json")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::Invalid);
    QVERIFY(command.payload.contains(QStringLiteral("--json")));
  }

  void testStatusTextCommand() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("status")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintStatusText);
  }

  void testStatusJsonCommand() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("status"),
         QStringLiteral("--json")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintStatusJson);
  }

  void testDiagnosticsTextOption() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("diagnostics")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintDiagnosticsText);
  }

  void testDiagnosticsJsonOption() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("diagnostics"),
         QStringLiteral("--json")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintDiagnosticsJson);
  }

  void testLegacyDiagnosticsOptionStillWorks() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("--diagnostics"),
         QStringLiteral("--json")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintDiagnosticsJson);
  }

  void testDriverInstallDefaultsToProprietary() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("driver"),
         QStringLiteral("install")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action,
             RoControlCli::CommandAction::InstallProprietaryDriver);
    QCOMPARE(command.acceptLicense, false);
  }

  void testDriverInstallOpenSource() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("driver"),
         QStringLiteral("install"), QStringLiteral("--open-source")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action,
             RoControlCli::CommandAction::InstallOpenSourceDriver);
  }

  void testDriverInstallAcceptLicense() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("driver"),
         QStringLiteral("install"), QStringLiteral("--proprietary"),
         QStringLiteral("--accept-license")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action,
             RoControlCli::CommandAction::InstallProprietaryDriver);
    QCOMPARE(command.acceptLicense, true);
  }

  void testDriverUpdateCommand() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("driver"),
         QStringLiteral("update")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::UpdateDriver);
  }

  void testDriverInstallRejectsJson() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("driver"),
         QStringLiteral("install"), QStringLiteral("--json")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::Invalid);
    QVERIFY(command.payload.contains(QStringLiteral("--json")));
  }

  void testDriverInstallRejectsConflictingModes() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("driver"),
         QStringLiteral("install"), QStringLiteral("--proprietary"),
         QStringLiteral("--open-source")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::Invalid);
    QVERIFY(
        command.payload.contains(QStringLiteral("cannot be used together")));
  }

  void testRenderDiagnosticsText() {
    RoControlCli::DiagnosticsSnapshot snapshot;
    snapshot.applicationName = QStringLiteral("ro-control");
    snapshot.applicationVersion = kAppVersion;
    snapshot.locale = QStringLiteral("en_US");
    snapshot.gpuFound = true;
    snapshot.gpuName = QStringLiteral("Example GPU");
    snapshot.driverVersion = QStringLiteral("1.2.3");
    snapshot.activeDriver = QStringLiteral("Proprietary");
    snapshot.verificationReport = QStringLiteral("GPU: Example GPU");

    const QString text = RoControlCli::renderDiagnosticsText(snapshot);
    QVERIFY(text.contains(QStringLiteral("application: ro-control")));
    QVERIFY(text.contains(QStringLiteral("driver_version: 1.2.3")));
    QVERIFY(text.contains(QStringLiteral("verification_report:")));
  }

  void testRenderStatusText() {
    RoControlCli::DiagnosticsSnapshot snapshot;
    snapshot.applicationName = QStringLiteral("ro-control");
    snapshot.applicationVersion = kAppVersion;
    snapshot.activeDriver = QStringLiteral("Proprietary");
    snapshot.updateAvailable = true;

    const QString text = RoControlCli::renderStatusText(snapshot);
    QVERIFY(text.contains(QStringLiteral("application: ro-control")));
    QVERIFY(text.contains(QStringLiteral("active_driver: Proprietary")));
    QVERIFY(text.contains(QStringLiteral("update_available: yes")));
  }

  void testRenderDiagnosticsJsonObject() {
    RoControlCli::DiagnosticsSnapshot snapshot;
    snapshot.applicationName = QStringLiteral("ro-control");
    snapshot.applicationVersion = kAppVersion;
    snapshot.gpuFound = true;
    snapshot.ramUsagePercent = 42;

    const QJsonObject object =
        RoControlCli::renderDiagnosticsJsonObject(snapshot);
    QCOMPARE(object.value(QStringLiteral("application")).toString(),
             QStringLiteral("ro-control"));
    QCOMPARE(object.value(QStringLiteral("gpuFound")).toBool(), true);
    QCOMPARE(object.value(QStringLiteral("ramUsagePercent")).toInt(), 42);
  }

  void testRenderStatusJsonObject() {
    RoControlCli::DiagnosticsSnapshot snapshot;
    snapshot.applicationName = QStringLiteral("ro-control");
    snapshot.applicationVersion = kAppVersion;
    snapshot.updateAvailable = true;

    const QJsonObject object = RoControlCli::renderStatusJsonObject(snapshot);
    QCOMPARE(object.value(QStringLiteral("command")).toString(),
             QStringLiteral("status"));
    QCOMPARE(object.value(QStringLiteral("application")).toString(),
             QStringLiteral("ro-control"));
    QCOMPARE(object.value(QStringLiteral("updateAvailable")).toBool(), true);
  }

  void testFanStatusCliCommand() {
    const auto commandText = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("fan"),
         QStringLiteral("status")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(commandText.action,
             RoControlCli::CommandAction::PrintFanStatusText);

    const auto commandJson = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("fan"),
         QStringLiteral("status"), QStringLiteral("--json")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(commandJson.action,
             RoControlCli::CommandAction::PrintFanStatusJson);
  }

  void testFanSetSpeedCliCommand() {
    const auto validCommand = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("fan"),
         QStringLiteral("set-speed"), QStringLiteral("75")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(validCommand.action, RoControlCli::CommandAction::FanSetSpeed);
    QCOMPARE(validCommand.payload, QStringLiteral("75"));

    const auto invalidCommand = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("fan"),
         QStringLiteral("set-speed"), QStringLiteral("150")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(invalidCommand.action, RoControlCli::CommandAction::Invalid);
  }

  void testFanSetModeCliCommand() {
    const auto validCommand = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("fan"),
         QStringLiteral("set-mode"), QStringLiteral("silent")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(validCommand.action, RoControlCli::CommandAction::FanSetMode);
    QCOMPARE(validCommand.payload, QStringLiteral("silent"));

    const auto invalidCommand = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("fan"),
         QStringLiteral("set-mode"),
         QStringLiteral("turbo-extreme-unsupported")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(invalidCommand.action, RoControlCli::CommandAction::Invalid);
  }

  void testFanSetSpeedValidation() {
    const auto negCommand = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("fan"),
         QStringLiteral("set-speed"), QStringLiteral("-10")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(negCommand.action, RoControlCli::CommandAction::Invalid);

    const auto nonNumCommand = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("fan"),
         QStringLiteral("set-speed"), QStringLiteral("fast")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(nonNumCommand.action, RoControlCli::CommandAction::Invalid);
  }

  void testFanResetCliCommand() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("fan"),
         QStringLiteral("reset")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::FanReset);
  }

  void testPowerStatusCliCommand() {
    const auto commandText = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("power"),
         QStringLiteral("status")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(commandText.action,
             RoControlCli::CommandAction::PrintPowerStatusText);

    const auto commandJson = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("power"),
         QStringLiteral("status"), QStringLiteral("--json")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(commandJson.action,
             RoControlCli::CommandAction::PrintPowerStatusJson);
  }

  void testPowerSetLimitCliCommand() {
    const auto validCommand = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("power"),
         QStringLiteral("set-limit"), QStringLiteral("175")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(validCommand.action, RoControlCli::CommandAction::PowerSetLimit);
    QCOMPARE(validCommand.payload, QStringLiteral("175"));

    const auto invalidCommand = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("power"),
         QStringLiteral("set-limit"), QStringLiteral("-50")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(invalidCommand.action, RoControlCli::CommandAction::Invalid);
  }

  void testPowerSetPresetCliCommand() {
    const auto validCommand = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("power"),
         QStringLiteral("set-preset"), QStringLiteral("eco")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(validCommand.action, RoControlCli::CommandAction::PowerSetPreset);
    QCOMPARE(validCommand.payload, QStringLiteral("eco"));
  }

  void testPowerSetPersistenceCliCommand() {
    const auto validCommand = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("power"),
         QStringLiteral("set-persistence"), QStringLiteral("on")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(validCommand.action,
             RoControlCli::CommandAction::PowerSetPersistence);
    QCOMPARE(validCommand.payload, QStringLiteral("1"));
  }

  void testDaemonFlagCliCommand() {
    const auto command = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("--daemon")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::RunDaemon);
  }

  void testProcessesAndGpusCliCommands() {
    const auto procText = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("processes")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(procText.action, RoControlCli::CommandAction::PrintProcessesText);

    const auto procJson = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("processes"),
         QStringLiteral("--json")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(procJson.action, RoControlCli::CommandAction::PrintProcessesJson);

    const auto killCmd = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("kill-process"),
         QStringLiteral("1234")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(killCmd.action, RoControlCli::CommandAction::KillProcess);
    QCOMPARE(killCmd.payload, QStringLiteral("1234"));

    const auto gpusText = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("gpus")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(gpusText.action, RoControlCli::CommandAction::PrintGpusText);

    const auto selectGpu = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("select-gpu"),
         QStringLiteral("1")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(selectGpu.action, RoControlCli::CommandAction::SelectGpu);
    QCOMPARE(selectGpu.payload, QStringLiteral("1"));
  }

  void testPowerSetClocksAndFanSmoothingCli() {
    const auto clockCmd = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("power"),
         QStringLiteral("set-clocks"), QStringLiteral("50"),
         QStringLiteral("200")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(clockCmd.action, RoControlCli::CommandAction::PowerSetClocks);
    QCOMPARE(clockCmd.payload, QStringLiteral("50:200"));

    const auto smoothCmd = RoControlCli::parseArguments(
        {QStringLiteral("ro-control"), QStringLiteral("fan"),
         QStringLiteral("set-smoothing"), QStringLiteral("on"),
         QStringLiteral("25"), QStringLiteral("10"), QStringLiteral("3")},
        QStringLiteral("ro-control"), kAppVersion, QStringLiteral("CLI test"));
    QCOMPARE(smoothCmd.action, RoControlCli::CommandAction::FanSetSmoothing);
    QCOMPARE(smoothCmd.payload, QStringLiteral("1:25:10:3"));
  }
};

QTEST_MAIN(TestCli)
#include "test_cli.moc"
