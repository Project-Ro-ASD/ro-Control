#include <QJsonObject>
#include <QTest>

#include "cli/cli.h"

class TestCli : public QObject {
  Q_OBJECT

private slots:
  void testHelpOption() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("--help")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintHelp);
    QVERIFY(command.payload.contains(QStringLiteral("driver install")));
    QVERIFY(command.payload.contains(QStringLiteral("status [--json]")));
  }

  void testVersionOption() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("--version")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintVersion);
    QCOMPARE(command.payload, QStringLiteral("0.2.1"));
  }

  void testJsonRequiresDiagnostics() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("--json")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::Invalid);
    QVERIFY(command.payload.contains(QStringLiteral("--json")));
  }

  void testStatusTextCommand() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("status")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintStatusText);
  }

  void testStatusJsonCommand() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("status"),
                                      QStringLiteral("--json")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintStatusJson);
  }

  void testDiagnosticsTextOption() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("diagnostics")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintDiagnosticsText);
  }

  void testDiagnosticsJsonOption() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("diagnostics"),
                                      QStringLiteral("--json")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintDiagnosticsJson);
  }

  void testLegacyDiagnosticsOptionStillWorks() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("--diagnostics"),
                                      QStringLiteral("--json")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::PrintDiagnosticsJson);
  }

  void testDriverInstallDefaultsToProprietary() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("driver"),
                                      QStringLiteral("install")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action,
             RoControlCli::CommandAction::InstallProprietaryDriver);
    QCOMPARE(command.acceptLicense, false);
  }

  void testDriverInstallOpenSource() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("driver"),
                                      QStringLiteral("install"),
                                      QStringLiteral("--open-source")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action,
             RoControlCli::CommandAction::InstallOpenSourceDriver);
  }

  void testDriverInstallAcceptLicense() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("driver"),
                                      QStringLiteral("install"),
                                      QStringLiteral("--proprietary"),
                                      QStringLiteral("--accept-license")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action,
             RoControlCli::CommandAction::InstallProprietaryDriver);
    QCOMPARE(command.acceptLicense, true);
  }

  void testDriverUpdateCommand() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("driver"),
                                      QStringLiteral("update")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::UpdateDriver);
  }

  void testDriverInstallRejectsJson() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("driver"),
                                      QStringLiteral("install"),
                                      QStringLiteral("--json")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::Invalid);
    QVERIFY(command.payload.contains(QStringLiteral("--json")));
  }

  void testDriverInstallRejectsConflictingModes() {
    const auto command =
        RoControlCli::parseArguments({QStringLiteral("ro-control"),
                                      QStringLiteral("driver"),
                                      QStringLiteral("install"),
                                      QStringLiteral("--proprietary"),
                                      QStringLiteral("--open-source")},
                                     QStringLiteral("ro-control"),
                                     QStringLiteral("0.2.1"),
                                     QStringLiteral("CLI test"));
    QCOMPARE(command.action, RoControlCli::CommandAction::Invalid);
    QVERIFY(command.payload.contains(QStringLiteral("cannot be used together")));
  }

  void testRenderDiagnosticsText() {
    RoControlCli::DiagnosticsSnapshot snapshot;
    snapshot.applicationName = QStringLiteral("ro-control");
    snapshot.applicationVersion = QStringLiteral("0.2.1");
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
    snapshot.applicationVersion = QStringLiteral("0.2.1");
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
    snapshot.applicationVersion = QStringLiteral("0.2.1");
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
    snapshot.applicationVersion = QStringLiteral("0.2.1");
    snapshot.updateAvailable = true;

    const QJsonObject object = RoControlCli::renderStatusJsonObject(snapshot);
    QCOMPARE(object.value(QStringLiteral("command")).toString(),
             QStringLiteral("status"));
    QCOMPARE(object.value(QStringLiteral("application")).toString(),
             QStringLiteral("ro-control"));
    QCOMPARE(object.value(QStringLiteral("updateAvailable")).toBool(), true);
  }
};

QTEST_MAIN(TestCli)
#include "test_cli.moc"
