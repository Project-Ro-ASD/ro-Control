#include <QTest>

#include "monitor/cpumonitor.h"
#include "monitor/gpumonitor.h"
#include "monitor/rammonitor.h"

class TestMonitor : public QObject {
  Q_OBJECT

private slots:
  void testCpuConstruction() {
    CpuMonitor cpu;
    QVERIFY(cpu.running());
    cpu.refresh();
    QTest::qWait(25);
    cpu.refresh();

    QVERIFY(cpu.usagePercent() >= 0.0);
    QVERIFY(cpu.usagePercent() <= 100.0);
    QVERIFY(cpu.temperatureC() >= 0);
  }

  void testCpuLifecycleAndInterval() {
    CpuMonitor cpu;
    const int initialInterval = cpu.updateInterval();
    QVERIFY(initialInterval >= 250);

    cpu.stop();
    QVERIFY(!cpu.running());

    cpu.setUpdateInterval(200);
    QCOMPARE(cpu.updateInterval(), initialInterval);

    cpu.setUpdateInterval(750);
    QCOMPARE(cpu.updateInterval(), 750);

    cpu.start();
    QVERIFY(cpu.running());
  }

  void testGpuConstruction() {
    GpuMonitor gpu;
    QVERIFY(gpu.running());
    gpu.refresh();

    QVERIFY(gpu.temperatureC() >= 0);
    QVERIFY(gpu.utilizationPercent() >= 0);
    QVERIFY(gpu.utilizationPercent() <= 100);
    QVERIFY(gpu.memoryUsagePercent() >= 0);
    QVERIFY(gpu.memoryUsagePercent() <= 100);
    QVERIFY(gpu.memoryTotalMiB() >= 0);
    QVERIFY(gpu.memoryUsedMiB() >= 0);
    QVERIFY(gpu.memoryUsedMiB() <= gpu.memoryTotalMiB() || gpu.memoryTotalMiB() == 0);
  }

  void testGpuLifecycleAndInterval() {
    GpuMonitor gpu;
    const int initialInterval = gpu.updateInterval();
    QVERIFY(initialInterval >= 250);

    gpu.stop();
    QVERIFY(!gpu.running());

    gpu.setUpdateInterval(200);
    QCOMPARE(gpu.updateInterval(), initialInterval);

    gpu.setUpdateInterval(900);
    QCOMPARE(gpu.updateInterval(), 900);

    gpu.start();
    QVERIFY(gpu.running());
  }

  void testRamConstruction() {
    RamMonitor ram;
    QVERIFY(ram.running());
    ram.refresh();

    QVERIFY(ram.usagePercent() >= 0);
    QVERIFY(ram.usagePercent() <= 100);
    QVERIFY(ram.totalMiB() >= 0);
    QVERIFY(ram.usedMiB() >= 0);
    QVERIFY(ram.usedMiB() <= ram.totalMiB() || ram.totalMiB() == 0);
  }

  void testRamLifecycleAndInterval() {
    RamMonitor ram;
    const int initialInterval = ram.updateInterval();
    QVERIFY(initialInterval >= 250);

    ram.stop();
    QVERIFY(!ram.running());

    ram.setUpdateInterval(200);
    QCOMPARE(ram.updateInterval(), initialInterval);

    ram.setUpdateInterval(1250);
    QCOMPARE(ram.updateInterval(), 1250);

    ram.start();
    QVERIFY(ram.running());
  }
};

QTEST_MAIN(TestMonitor)
#include "test_monitor.moc"
