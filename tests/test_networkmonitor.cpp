#include <QSignalSpy>
#include <QTest>

#include "monitor/networkmonitor.h"

class TestNetworkMonitor : public QObject {
  Q_OBJECT

private slots:
  void testConstructionAndRefresh() {
    NetworkMonitor nm;
    nm.refresh();

    QCOMPARE(nm.running(), true);
  }

  void testLifecycleAndMinimumInterval() {
    NetworkMonitor nm;
    const int initialInterval = nm.updateInterval();
    QVERIFY(initialInterval >= 1000);

    nm.stop();
    QVERIFY(!nm.running());

    nm.setUpdateInterval(500);
    QCOMPARE(nm.updateInterval(), initialInterval);

    nm.setUpdateInterval(1500);
    QCOMPARE(nm.updateInterval(), 1500);

    nm.start();
    QVERIFY(nm.running());
  }

  void testInterfaceStructure() {
    NetworkMonitor nm;
    nm.refresh();

    const auto ifaces = nm.interfaces();
    for (const auto &entry : ifaces) {
      const auto map = entry.toMap();
      QVERIFY(map.contains(QStringLiteral("name")));
      QVERIFY(map.contains(QStringLiteral("rxBytes")));
      QVERIFY(map.contains(QStringLiteral("txBytes")));
      QVERIFY(map.contains(QStringLiteral("rxBytesPerSec")));
      QVERIFY(map.contains(QStringLiteral("txBytesPerSec")));
      QVERIFY(map.contains(QStringLiteral("rxPackets")));
      QVERIFY(map.contains(QStringLiteral("txPackets")));
      QVERIFY(map.contains(QStringLiteral("rxErrors")));
      QVERIFY(map.contains(QStringLiteral("txErrors")));
      QVERIFY(map.contains(QStringLiteral("rxDropped")));
      QVERIFY(map.contains(QStringLiteral("txDropped")));
    }
  }

  void testTotalCountersNonNegative() {
    NetworkMonitor nm;
    nm.refresh();

    QVERIFY(nm.totalRxBytesPerSec() >= 0);
    QVERIFY(nm.totalTxBytesPerSec() >= 0);
    QVERIFY(nm.interfaceCount() >= 0);
  }

  void testStatsChangedSignal() {
    NetworkMonitor nm;
    QSignalSpy spy(&nm, &NetworkMonitor::networkStatsChanged);
    QVERIFY(spy.isValid());
    nm.refresh();
    QVERIFY(spy.count() >= 1);
  }
};

QTEST_MAIN(TestNetworkMonitor)
#include "test_networkmonitor.moc"
