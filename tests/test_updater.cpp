#include <QTest>

#include "nvidia/updater.h"
#include "nvidia/versionparser.h"

class TestUpdater : public QObject {
  Q_OBJECT

private slots:
  void testParseAvailablePackageVersions() {
    const QString sample =
        QStringLiteral("Installed Packages\n"
                       "akmod-nvidia.x86_64 3:570.153.02-1.fc42 @updates\n"
                       "Available Packages\n"
                       "akmod-nvidia.x86_64 3:565.77-1.fc42 rpmfusion\n"
                       "akmod-nvidia.x86_64 3:570.124.04-1.fc42 rpmfusion\n"
                       "akmod-nvidia.x86_64 3:570.153.02-1.fc42 updates\n");

    const QStringList versions =
        NvidiaVersionParser::parseAvailablePackageVersions(
            sample, QStringLiteral("akmod-nvidia"));

    QCOMPARE(versions.size(), 3);
    QCOMPARE(versions.at(0), QStringLiteral("3:570.153.02-1.fc42"));
    QCOMPARE(versions.at(1), QStringLiteral("3:565.77-1.fc42"));
    QCOMPARE(versions.at(2), QStringLiteral("3:570.124.04-1.fc42"));
  }

  void testParseCheckUpdateVersion() {
    const QString sample =
        QStringLiteral("akmod-nvidia.x86_64 3:570.153.02-1.fc42 updates\n");

    QCOMPARE(NvidiaVersionParser::parseCheckUpdateVersion(
                 sample, QStringLiteral("akmod-nvidia")),
             QStringLiteral("3:570.153.02-1.fc42"));
  }

  void testPackageSpecForVersion() {
    QCOMPARE(NvidiaVersionParser::packageSpecForVersion(
                 QStringLiteral("akmod-nvidia"),
                 QStringLiteral("3:570.153.02-1.fc42")),
             QStringLiteral("akmod-nvidia-3:570.153.02-1.fc42"));
  }

  void testBuildVersionedPackageSpecs() {
    const QStringList specs = NvidiaVersionParser::buildVersionedPackageSpecs(
        {QStringLiteral("akmod-nvidia"), QStringLiteral("nvidia-settings")},
        QStringLiteral("3:570.153.02-1.fc42"));

    QCOMPARE(specs.size(), 2);
    QCOMPARE(specs.at(0), QStringLiteral("akmod-nvidia-3:570.153.02-1.fc42"));
    QCOMPARE(specs.at(1),
             QStringLiteral("nvidia-settings-3:570.153.02-1.fc42"));
  }

  void testBuildTransactionArgumentsForFreshInstallStaysScoped() {
    NvidiaUpdater updater;
    updater.setLatestPackageVersion(QStringLiteral("3:570.153.02-1.fc42"));

    const QStringList args = updater.buildTransactionArguments(
        QString(), QString(), QStringLiteral("wayland"),
        QStringLiteral("akmod-nvidia"));

    QCOMPARE(args.value(0), QStringLiteral("install"));
    QVERIFY(args.contains(QStringLiteral("--refresh")));
    QVERIFY(args.contains(QStringLiteral("--best")));
    QVERIFY(!args.contains(QStringLiteral("update")));
    QVERIFY(!args.contains(QStringLiteral("upgrade")));
    QVERIFY(!args.contains(QStringLiteral("system-upgrade")));
    QVERIFY(args.contains(QStringLiteral("akmod-nvidia-3:570.153.02-1.fc42")));
    QVERIFY(!args.contains(QStringLiteral("xorg-x11-drv-nvidia")));
  }

  void testBuildTransactionArgumentsForInstalledDriverAvoidsBroadUpdate() {
    NvidiaUpdater updater;
    updater.setLatestPackageVersion(QStringLiteral("3:570.153.02-1.fc42"));

    const QStringList args = updater.buildTransactionArguments(
        QString(), QStringLiteral("3:565.77-1.fc42"), QStringLiteral("wayland"),
        QStringLiteral("akmod-nvidia"));

    QCOMPARE(args.value(0), QStringLiteral("distro-sync"));
    QVERIFY(args.contains(QStringLiteral("--allowerasing")));
    QVERIFY(!args.contains(QStringLiteral("update")));
    QVERIFY(!args.contains(QStringLiteral("upgrade")));
    QVERIFY(!args.contains(QStringLiteral("system-upgrade")));
    QVERIFY(args.contains(QStringLiteral("akmod-nvidia-3:570.153.02-1.fc42")));
    QVERIFY(!args.contains(QStringLiteral("xorg-x11-drv-nvidia")));
  }

  void testTransactionChangedReturnsFalseForNoopOutput() {
    NvidiaUpdater updater;
    CommandRunner::Result result{
        .exitCode = 0,
        .stdout = QStringLiteral(
            "Last metadata expiration check: 0:00:12 ago.\nNothing to do.\n"),
        .stderr = QString(),
        .attempt = 1,
    };

    QVERIFY(!updater.transactionChanged(result));
  }

  void testTransactionChangedReturnsTrueForRealPackageTransaction() {
    NvidiaUpdater updater;
    CommandRunner::Result result{
        .exitCode = 0,
        .stdout = QStringLiteral("Installing:\nakmod-nvidia.x86_64 "
                                 "3:570.153.02-1.fc42\nComplete!\n"),
        .stderr = QString(),
        .attempt = 1,
    };

    QVERIFY(updater.transactionChanged(result));
  }

  void testBuildTransactionArgumentsForOpenKernelModules() {
    NvidiaUpdater updater;
    updater.setLatestPackageVersion(QStringLiteral("3:570.153.02-1.fc42"));

    const QStringList args = updater.buildTransactionArguments(
        QString(), QStringLiteral("3:565.77-1.fc42"), QStringLiteral("wayland"),
        QStringLiteral("akmod-nvidia-open"));

    QVERIFY(
        args.contains(QStringLiteral("akmod-nvidia-open-3:570.153.02-1.fc42")));
  }

  void testParseOfficialUnixDriverVersions() {
    const QString sample = QStringLiteral(
        "<html><body>Linux x86_64/AMD64/EM64T "
        "Latest Production Branch Version: <a>595.71.05</a> "
        "Latest New Feature Branch Version: <a>590.48.01</a> "
        "Latest Beta Version: <a>595.45.04</a> "
        "Latest Legacy GPU version (470.xx series): <a>470.256.02</a> "
        "Linux aarch64 Latest Production Branch Version: <a>595.71.05</a>"
        "</body></html>");

    const QStringList versions =
        NvidiaVersionParser::parseOfficialUnixDriverVersions(
            sample, QStringLiteral("x86_64"));

    QCOMPARE(versions.size(), 4);
    QCOMPARE(versions.at(0), QStringLiteral("595.71.05"));
    QCOMPARE(versions.at(1), QStringLiteral("590.48.01"));
    QCOMPARE(versions.at(2), QStringLiteral("595.45.04"));
    QCOMPARE(versions.at(3), QStringLiteral("470.256.02"));
  }

  void testNormalizedDriverVersion() {
    QCOMPARE(NvidiaVersionParser::normalizedDriverVersion(
                 QStringLiteral("3:570.153.02-1.fc42")),
             QStringLiteral("570.153.02"));
    QCOMPARE(NvidiaVersionParser::normalizedDriverVersion(
                 QStringLiteral("595.71.05")),
             QStringLiteral("595.71.05"));
  }
};

QTEST_MAIN(TestUpdater)
#include "test_updater.moc"
