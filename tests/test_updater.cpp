#include <QTest>

#define private public
#include "nvidia/updater.h"
#undef private
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
        {QStringLiteral("akmod-nvidia"), QStringLiteral("xorg-x11-drv-nvidia")},
        QStringLiteral("3:570.153.02-1.fc42"));

    QCOMPARE(specs.size(), 2);
    QCOMPARE(specs.at(0), QStringLiteral("akmod-nvidia-3:570.153.02-1.fc42"));
    QCOMPARE(specs.at(1),
             QStringLiteral("xorg-x11-drv-nvidia-3:570.153.02-1.fc42"));
  }

  void testBuildTransactionArgumentsForFreshInstallStaysScoped() {
    NvidiaUpdater updater;
    updater.m_latestVersion = QStringLiteral("3:570.153.02-1.fc42");

    const QStringList args =
        updater.buildTransactionArguments(QString(), QString(), QString(),
                                         QStringLiteral("akmod-nvidia"));

    QCOMPARE(args.value(0), QStringLiteral("install"));
    QVERIFY(args.contains(QStringLiteral("--refresh")));
    QVERIFY(args.contains(QStringLiteral("--best")));
    QVERIFY(!args.contains(QStringLiteral("update")));
    QVERIFY(!args.contains(QStringLiteral("upgrade")));
    QVERIFY(!args.contains(QStringLiteral("system-upgrade")));
    QVERIFY(args.contains(QStringLiteral("akmod-nvidia-3:570.153.02-1.fc42")));
  }

  void testBuildTransactionArgumentsForInstalledDriverAvoidsBroadUpdate() {
    NvidiaUpdater updater;
    updater.m_latestVersion = QStringLiteral("3:570.153.02-1.fc42");

    const QStringList args = updater.buildTransactionArguments(
        QString(), QStringLiteral("3:565.77-1.fc42"), QString(),
        QStringLiteral("akmod-nvidia"));

    QCOMPARE(args.value(0), QStringLiteral("distro-sync"));
    QVERIFY(args.contains(QStringLiteral("--allowerasing")));
    QVERIFY(!args.contains(QStringLiteral("update")));
    QVERIFY(!args.contains(QStringLiteral("upgrade")));
    QVERIFY(!args.contains(QStringLiteral("system-upgrade")));
    QVERIFY(args.contains(QStringLiteral("akmod-nvidia-3:570.153.02-1.fc42")));
  }

  void testTransactionChangedReturnsFalseForNoopOutput() {
    NvidiaUpdater updater;
    CommandRunner::Result result{
        .exitCode = 0,
        .stdout = QStringLiteral("Last metadata expiration check: 0:00:12 ago.\nNothing to do.\n"),
        .stderr = QString(),
        .attempt = 1,
    };

    QVERIFY(!updater.transactionChanged(result));
  }

  void testTransactionChangedReturnsTrueForRealPackageTransaction() {
    NvidiaUpdater updater;
    CommandRunner::Result result{
        .exitCode = 0,
        .stdout = QStringLiteral("Installing:\nakmod-nvidia.x86_64 3:570.153.02-1.fc42\nComplete!\n"),
        .stderr = QString(),
        .attempt = 1,
    };

    QVERIFY(updater.transactionChanged(result));
  }

  void testBuildTransactionArgumentsForOpenKernelModules() {
    NvidiaUpdater updater;
    updater.m_latestVersion = QStringLiteral("3:570.153.02-1.fc42");

    const QStringList args = updater.buildTransactionArguments(
        QString(), QStringLiteral("3:565.77-1.fc42"), QString(),
        QStringLiteral("akmod-nvidia-open"));

    QVERIFY(args.contains(
        QStringLiteral("akmod-nvidia-open-3:570.153.02-1.fc42")));
  }
};

QTEST_MAIN(TestUpdater)
#include "test_updater.moc"
