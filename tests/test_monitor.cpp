#include <QStandardPaths>
#include <QTest>

#include "monitor/cpumonitor.h"
#include "monitor/gpumonitor.h"
#include "monitor/rammonitor.h"

class TestMonitor : public QObject {
  Q_OBJECT

private slots:
  void testCpuConstruction() {
    CpuMonitor cpu;
    cpu.refresh();
    QTest::qWait(25);
    cpu.refresh();

    QVERIFY(cpu.usagePercent() >= 0.0);
    QVERIFY(cpu.usagePercent() <= 100.0);
    QVERIFY(cpu.temperatureC() >= 0);
  }

  void testGpuConstruction() {
    GpuMonitor gpu;
    gpu.refresh();

    if (QStandardPaths::findExecutable(QStringLiteral("nvidia-smi"))
            .isEmpty()) {
      QCOMPARE(gpu.available(), false);
      QCOMPARE(gpu.temperatureC(), 0);
      QCOMPARE(gpu.utilizationPercent(), 0);
      QCOMPARE(gpu.memoryUsagePercent(), 0);
      return;
    }

    QVERIFY(gpu.temperatureC() >= 0);
    QVERIFY(gpu.utilizationPercent() >= 0);
    QVERIFY(gpu.utilizationPercent() <= 100);
    QVERIFY(gpu.memoryUsagePercent() >= 0);
    QVERIFY(gpu.memoryUsagePercent() <= 100);
    QVERIFY(!gpu.gpuName().isEmpty());
  }

  void testRamConstruction() {
    RamMonitor ram;
    ram.refresh();

    QVERIFY(ram.usagePercent() >= 0);
    QVERIFY(ram.usagePercent() <= 100);
    QVERIFY(ram.totalMiB() >= 0);
    QVERIFY(ram.usedMiB() >= 0);
    QVERIFY(ram.usedMiB() <= ram.totalMiB() || ram.totalMiB() == 0);
  }
};

QTEST_MAIN(TestMonitor)
#include "test_monitor.moc"
