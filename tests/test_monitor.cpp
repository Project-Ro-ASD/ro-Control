#include <QSignalSpy>
#include <QTest>

#include "monitor/cpumonitor.h"
#include "monitor/gpumonitor.h"
#include "monitor/rammonitor.h"

class TestMonitor : public QObject {
  Q_OBJECT

private slots:
  // ── CpuMonitor ──────────────────────────────────────────────────────────

  void testCpuConstruction() {
    CpuMonitor cpu;
    QCOMPARE(cpu.load(), 0);
    QCOMPARE(cpu.temperature(), 0);
  }

  void testCpuStartStop() {
    CpuMonitor cpu;
    cpu.start(500);
    // Timer çalışıyor olmalı
    QTest::qWait(100);
    cpu.stop();
    QVERIFY(true); // crash olmadı
  }

  void testCpuSignals() {
    CpuMonitor cpu;
    QSignalSpy loadSpy(&cpu, &CpuMonitor::loadChanged);
    QSignalSpy tempSpy(&cpu, &CpuMonitor::temperatureChanged);
    QVERIFY(loadSpy.isValid());
    QVERIFY(tempSpy.isValid());
  }

  void testCpuLoadRange() {
    CpuMonitor cpu;
    cpu.start(100);
    QTest::qWait(300); // birkaç poll döngüsü bekle
    cpu.stop();
    // Yük 0-100 arasında olmalı
    QVERIFY(cpu.load() >= 0 && cpu.load() <= 100);
  }

  // ── GpuMonitor ──────────────────────────────────────────────────────────

  void testGpuConstruction() {
    GpuMonitor gpu;
    QCOMPARE(gpu.temperature(), 0);
    QCOMPARE(gpu.load(), 0);
    QCOMPARE(gpu.vramUsed(), 0);
    QCOMPARE(gpu.vramTotal(), 0);
    QCOMPARE(gpu.available(), false);
  }

  void testGpuStartStop() {
    GpuMonitor gpu;
    gpu.start(500);
    QTest::qWait(100);
    gpu.stop();
    QVERIFY(true);
  }

  void testGpuSignals() {
    GpuMonitor gpu;
    QSignalSpy tempSpy(&gpu, &GpuMonitor::temperatureChanged);
    QSignalSpy loadSpy(&gpu, &GpuMonitor::loadChanged);
    QSignalSpy vramUsedSpy(&gpu, &GpuMonitor::vramUsedChanged);
    QSignalSpy vramTotalSpy(&gpu, &GpuMonitor::vramTotalChanged);
    QSignalSpy availSpy(&gpu, &GpuMonitor::availableChanged);
    QVERIFY(tempSpy.isValid());
    QVERIFY(loadSpy.isValid());
    QVERIFY(vramUsedSpy.isValid());
    QVERIFY(vramTotalSpy.isValid());
    QVERIFY(availSpy.isValid());
  }

  // ── RamMonitor ──────────────────────────────────────────────────────────

  void testRamConstruction() {
    RamMonitor ram;
    QCOMPARE(ram.totalMb(), 0);
    QCOMPARE(ram.usedMb(), 0);
    QCOMPARE(ram.usedPercent(), 0);
  }

  void testRamStartStop() {
    RamMonitor ram;
    ram.start(500);
    QTest::qWait(100);
    ram.stop();
    QVERIFY(true);
  }

  void testRamSignals() {
    RamMonitor ram;
    QSignalSpy totalSpy(&ram, &RamMonitor::totalMbChanged);
    QSignalSpy usedSpy(&ram, &RamMonitor::usedMbChanged);
    QSignalSpy pctSpy(&ram, &RamMonitor::usedPercentChanged);
    QVERIFY(totalSpy.isValid());
    QVERIFY(usedSpy.isValid());
    QVERIFY(pctSpy.isValid());
  }
};

QTEST_MAIN(TestMonitor)
#include "test_monitor.moc"
