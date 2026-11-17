#include <QSignalSpy>
#include <QTest>

#include "monitor/diskmonitor.h"

class TestDiskMonitor : public QObject {
  Q_OBJECT

private slots:
  void testConstructionAndRefresh() {
    DiskMonitor dm;
    dm.refresh();

    // The host CI machine may not have mounted block devices in some
    // environments; at minimum we expect refresh() to return without crashing
    // and the signals to be emitted.
    QCOMPARE(dm.running(), true);
  }

  void testLifecycleAndMinimumInterval() {
    DiskMonitor dm;
    const int initialInterval = dm.updateInterval();
    QVERIFY(initialInterval >= 1000);

    dm.stop();
    QVERIFY(!dm.running());

    dm.setUpdateInterval(500);
    QCOMPARE(dm.updateInterval(), initialInterval);

    dm.setUpdateInterval(1500);
    QCOMPARE(dm.updateInterval(), 1500);

    dm.start();
    QVERIFY(dm.running());
  }

  void testDiskStatsFormat() {
    DiskMonitor dm;
    dm.refresh();

    const auto stats = dm.diskStats();
    for (const auto &entry : stats) {
      const auto map = entry.toMap();
      QVERIFY(map.contains(QStringLiteral("device")));
      QVERIFY(map.contains(QStringLiteral("mountPoint")));
      QVERIFY(map.contains(QStringLiteral("fsType")));
      QVERIFY(map.contains(QStringLiteral("totalBytes")));
      QVERIFY(map.contains(QStringLiteral("usedBytes")));
      QVERIFY(map.contains(QStringLiteral("usagePercent")));
    }
  }

  void testAvailableSignalEmission() {
    DiskMonitor dm;
    dm.refresh();
    QSignalSpy spy(&dm, &DiskMonitor::diskStatsChanged);
    QVERIFY(spy.isValid());
    dm.refresh();
    QVERIFY(spy.count() >= 1);
  }
};

QTEST_MAIN(TestDiskMonitor)
#include "test_diskmonitor.moc"
