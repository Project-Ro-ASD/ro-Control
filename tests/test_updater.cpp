#include <QTest>

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
};

QTEST_MAIN(TestUpdater)
#include "test_updater.moc"
