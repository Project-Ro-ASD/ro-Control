#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

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
    QVERIFY(gpu.memoryUsedMiB() <= gpu.memoryTotalMiB() ||
            gpu.memoryTotalMiB() == 0);
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

  void testGpuPartialTelemetryKeepsMonitorAvailable() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString scriptPath =
        tempDir.filePath(QStringLiteral("fake-nvidia-smi.sh"));
    QFile script(scriptPath);
    QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream stream(&script);
    stream << "#!/bin/sh\n";
    stream << "printf 'NVIDIA GeForce RTX 4080, 61, N/A, 2048, 16384\\n'\n";
    script.close();
    QVERIFY(QFile::setPermissions(scriptPath, QFileDevice::ReadOwner |
                                                  QFileDevice::WriteOwner |
                                                  QFileDevice::ExeOwner));

    qputenv("RO_CONTROL_COMMAND_NVIDIA_SMI", scriptPath.toUtf8());
    GpuMonitor gpu;
    gpu.stop();
    gpu.refresh();

    QVERIFY(gpu.available());
    QCOMPARE(gpu.gpuName(), QStringLiteral("NVIDIA GeForce RTX 4080"));
    QCOMPARE(gpu.temperatureC(), 61);
    QCOMPARE(gpu.utilizationPercent(), 0);
    QCOMPARE(gpu.memoryUsedMiB(), 2048);
    QCOMPARE(gpu.memoryTotalMiB(), 16384);
    QCOMPARE(gpu.memoryUsagePercent(), 12);

    qunsetenv("RO_CONTROL_COMMAND_NVIDIA_SMI");
  }

  void testGpuPowerAndClockTelemetry() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString scriptPath =
        tempDir.filePath(QStringLiteral("fake-nvidia-smi-full.sh"));
    QFile script(scriptPath);
    QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream stream(&script);
    stream << "#!/bin/sh\n";
    stream << "printf 'NVIDIA GeForce RTX 4090, 58, 85, 6144, 24576, 45, "
              "215.50 W, 450.00 W, 2520 MHz, 10501 MHz\\n'\n";
    script.close();
    QVERIFY(QFile::setPermissions(scriptPath, QFileDevice::ReadOwner |
                                                  QFileDevice::WriteOwner |
                                                  QFileDevice::ExeOwner));

    qputenv("RO_CONTROL_COMMAND_NVIDIA_SMI", scriptPath.toUtf8());
    GpuMonitor gpu;
    gpu.stop();
    gpu.refresh();

    QVERIFY(gpu.available());
    QCOMPARE(gpu.gpuName(), QStringLiteral("NVIDIA GeForce RTX 4090"));
    QCOMPARE(gpu.temperatureC(), 58);
    QCOMPARE(gpu.utilizationPercent(), 85);
    QCOMPARE(gpu.memoryUsedMiB(), 6144);
    QCOMPARE(gpu.memoryTotalMiB(), 24576);
    QCOMPARE(gpu.fanSpeedPercent(), 45);
    QCOMPARE(gpu.powerDrawW(), 215.50);
    QCOMPARE(gpu.powerLimitW(), 450.00);
    QCOMPARE(gpu.graphicsClockMHz(), 2520);
    QCOMPARE(gpu.memoryClockMHz(), 10501);

    qunsetenv("RO_CONTROL_COMMAND_NVIDIA_SMI");
  }

  void testGpuStatusMessageWhenTelemetryUnavailable() {
    qputenv("RO_CONTROL_COMMAND_NVIDIA_SMI",
            QByteArrayLiteral("/definitely/missing/nvidia-smi"));
    qputenv("RO_CONTROL_COMMAND_SENSORS",
            QByteArrayLiteral("/definitely/missing/sensors"));
    qputenv("RO_CONTROL_COMMAND_NVIDIA_SETTINGS",
            QByteArrayLiteral("/definitely/missing/nvidia-settings"));
    qputenv("RO_CONTROL_DRM_ROOT",
            QByteArrayLiteral("/definitely/missing/drm-root"));

    GpuMonitor gpu;
    gpu.stop();
    gpu.refresh();

    QVERIFY(!gpu.available());
    QVERIFY(!gpu.statusMessage().trimmed().isEmpty());

    qunsetenv("RO_CONTROL_COMMAND_NVIDIA_SMI");
    qunsetenv("RO_CONTROL_COMMAND_SENSORS");
    qunsetenv("RO_CONTROL_COMMAND_NVIDIA_SETTINGS");
    qunsetenv("RO_CONTROL_DRM_ROOT");
  }

  void testGpuTemperatureFallsBackToSensors() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString scriptPath =
        tempDir.filePath(QStringLiteral("fake-sensors.sh"));
    QFile script(scriptPath);
    QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream stream(&script);
    stream << "#!/bin/sh\n";
    stream << "cat <<'EOF'\n";
    stream << "nouveau-pci-0100\n";
    stream << "Adapter: PCI adapter\n";
    stream << "temp1:\n";
    stream << "  temp1_input: 52.000\n";
    stream << "EOF\n";
    script.close();
    QVERIFY(QFile::setPermissions(scriptPath, QFileDevice::ReadOwner |
                                                  QFileDevice::WriteOwner |
                                                  QFileDevice::ExeOwner));

    qputenv("RO_CONTROL_COMMAND_NVIDIA_SMI",
            QByteArrayLiteral("/definitely/missing/nvidia-smi"));
    qputenv("RO_CONTROL_COMMAND_SENSORS", scriptPath.toUtf8());
    qputenv("RO_CONTROL_COMMAND_NVIDIA_SETTINGS",
            QByteArrayLiteral("/definitely/missing/nvidia-settings"));
    qputenv("RO_CONTROL_DRM_ROOT",
            QByteArrayLiteral("/definitely/missing/drm-root"));

    GpuMonitor gpu;
    gpu.stop();
    gpu.refresh();

    QVERIFY(gpu.available());
    QCOMPARE(gpu.temperatureC(), 52);
    QCOMPARE(gpu.utilizationPercent(), 0);
    QCOMPARE(gpu.memoryUsedMiB(), 0);
    QCOMPARE(gpu.memoryTotalMiB(), 0);

    qunsetenv("RO_CONTROL_COMMAND_NVIDIA_SMI");
    qunsetenv("RO_CONTROL_COMMAND_SENSORS");
    qunsetenv("RO_CONTROL_COMMAND_NVIDIA_SETTINGS");
    qunsetenv("RO_CONTROL_DRM_ROOT");
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

  void testRamUsesOverriddenMeminfoPath() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString meminfoPath = tempDir.filePath(QStringLiteral("meminfo"));
    QFile meminfo(meminfoPath);
    QVERIFY(meminfo.open(QIODevice::WriteOnly | QIODevice::Text));
    meminfo.write("MemTotal:       32768000 kB\n");
    meminfo.write("MemAvailable:   23552000 kB\n");
    meminfo.write("MemFree:         3072000 kB\n");
    meminfo.write("Buffers:          204800 kB\n");
    meminfo.write("Cached:          8192000 kB\n");
    meminfo.close();

    qputenv("RO_CONTROL_MEMINFO_PATH", meminfoPath.toUtf8());

    RamMonitor ram;
    ram.stop();
    ram.refresh();

    QVERIFY(ram.available());
    QCOMPARE(ram.totalMiB(), 32000);
    QCOMPARE(ram.usedMiB(), 9000);
    QCOMPARE(ram.usagePercent(), 28);

    qunsetenv("RO_CONTROL_MEMINFO_PATH");
  }

  void testRamReadsZramAndZswapTelemetry() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString swapsPath = tempDir.filePath(QStringLiteral("swaps"));
    QFile swaps(swapsPath);
    QVERIFY(swaps.open(QIODevice::WriteOnly | QIODevice::Text));
    swaps.write("Filename\tType\tSize\tUsed\tPriority\n");
    swaps.write("/dev/zram0 partition 2097152 524288 100\n");
    swaps.close();

    const QString zramRoot = tempDir.filePath(QStringLiteral("block"));
    QVERIFY(QDir().mkpath(zramRoot + QStringLiteral("/zram0")));
    QFile diskSize(zramRoot + QStringLiteral("/zram0/disksize"));
    QVERIFY(diskSize.open(QIODevice::WriteOnly | QIODevice::Text));
    diskSize.write("2147483648\n");
    diskSize.close();
    QFile memoryStat(zramRoot + QStringLiteral("/zram0/mm_stat"));
    QVERIFY(memoryStat.open(QIODevice::WriteOnly | QIODevice::Text));
    memoryStat.write("1073741824 450000000 536870912 0 0 0 0 0 0\n");
    memoryStat.close();

    const QString zswapPath = tempDir.filePath(QStringLiteral("zswap-enabled"));
    QFile zswap(zswapPath);
    QVERIFY(zswap.open(QIODevice::WriteOnly | QIODevice::Text));
    zswap.write("Y\n");
    zswap.close();

    qputenv("RO_CONTROL_SWAPS_PATH", swapsPath.toUtf8());
    qputenv("RO_CONTROL_ZRAM_SYSFS_ROOT", zramRoot.toUtf8());
    qputenv("RO_CONTROL_ZSWAP_ENABLED_PATH", zswapPath.toUtf8());

    RamMonitor ram;
    ram.stop();
    ram.refresh();

    QVERIFY(ram.zramAvailable());
    QCOMPARE(ram.zramTotalMiB(), 2048);
    QCOMPARE(ram.zramUsedMiB(), 512);
    QCOMPARE(ram.zramCompressionRatio(), 2.0);
    QVERIFY(ram.zswapEnabled());

    qunsetenv("RO_CONTROL_SWAPS_PATH");
    qunsetenv("RO_CONTROL_ZRAM_SYSFS_ROOT");
    qunsetenv("RO_CONTROL_ZSWAP_ENABLED_PATH");
  }

  void testGpuMultiDeviceAndProcesses() {
    GpuMonitor gpu;
    gpu.stop();
    gpu.refresh();

    QVERIFY(gpu.gpuCount() >= 1);
    QVERIFY(gpu.selectedGpuIndex() >= 0);
    QVERIFY(!gpu.gpuDevices().isEmpty());

    const auto dev = gpu.gpuDevices().first().toMap();
    QVERIFY(dev.contains(QStringLiteral("index")));
    QVERIFY(dev.contains(QStringLiteral("name")));

    gpu.setSelectedGpuIndex(0);
    QCOMPARE(gpu.selectedGpuIndex(), 0);

    QVERIFY(gpu.hotspotTemperatureC() >= 0);
    QVERIFY(gpu.memoryTemperatureC() >= 0);
    QVERIFY(gpu.gpuProcessCount() >= 0);

    // Testing killProcess with invalid PID returns false safely
    QVERIFY(!gpu.killProcess(0));
    QVERIFY(!gpu.killProcess(1));
  }

  void testRamFallbackWhenMemAvailableMissing() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString meminfoPath =
        tempDir.filePath(QStringLiteral("meminfo_fallback"));
    QFile meminfo(meminfoPath);
    QVERIFY(meminfo.open(QIODevice::WriteOnly | QIODevice::Text));
    meminfo.write("MemTotal:       16384000 kB\n");
    // No MemAvailable field
    meminfo.write("MemFree:         4096000 kB\n");
    meminfo.write("Buffers:         1024000 kB\n");
    meminfo.write("Cached:          4096000 kB\n");
    meminfo.write("SReclaimable:     512000 kB\n");
    meminfo.write("Shmem:            256000 kB\n");
    meminfo.close();

    qputenv("RO_CONTROL_MEMINFO_PATH", meminfoPath.toUtf8());

    RamMonitor ram;
    ram.stop();
    ram.refresh();

    QVERIFY(ram.available());
    QCOMPARE(ram.totalMiB(), 16000);
    // Calculated available = 4096000 + 1024000 + 4096000 + 512000 - 256000 =
    // 9472000 kB Used = 16384000 - 9472000 = 6912000 kB = 6750 MiB
    QCOMPARE(ram.usedMiB(), 6750);
    QVERIFY(ram.usagePercent() > 0);

    qunsetenv("RO_CONTROL_MEMINFO_PATH");
  }

  void testZramZeroPhysicalBytesHandlesDivisionByZeroGracefully() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString swapsPath = tempDir.filePath(QStringLiteral("swaps_empty"));
    QFile swaps(swapsPath);
    QVERIFY(swaps.open(QIODevice::WriteOnly | QIODevice::Text));
    swaps.write("Filename\tType\tSize\tUsed\tPriority\n");
    swaps.write("/dev/zram0 partition 1048576 0 100\n");
    swaps.close();

    const QString zramRoot = tempDir.filePath(QStringLiteral("block_empty"));
    QVERIFY(QDir().mkpath(zramRoot + QStringLiteral("/zram0")));
    QFile memoryStat(zramRoot + QStringLiteral("/zram0/mm_stat"));
    QVERIFY(memoryStat.open(QIODevice::WriteOnly | QIODevice::Text));
    // 0 original bytes, 0 physical bytes
    memoryStat.write("0 0 0 0 0 0 0 0 0\n");
    memoryStat.close();

    qputenv("RO_CONTROL_SWAPS_PATH", swapsPath.toUtf8());
    qputenv("RO_CONTROL_ZRAM_SYSFS_ROOT", zramRoot.toUtf8());

    RamMonitor ram;
    ram.stop();
    ram.refresh();

    QVERIFY(ram.zramAvailable());
    QCOMPARE(ram.zramCompressionRatio(), 0.0);

    qunsetenv("RO_CONTROL_SWAPS_PATH");
    qunsetenv("RO_CONTROL_ZRAM_SYSFS_ROOT");
  }

  void testZramDynamicTelemetryUpdates() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString swapsPath = tempDir.filePath(QStringLiteral("swaps_dynamic"));
    const QString zramRoot = tempDir.filePath(QStringLiteral("block_dynamic"));
    QVERIFY(QDir().mkpath(zramRoot + QStringLiteral("/zram0")));

    auto writeState = [&](qint64 usedKiB, qint64 origBytes,
                          qint64 physicalBytes) {
      QFile swaps(swapsPath);
      QVERIFY(swaps.open(QIODevice::WriteOnly | QIODevice::Text));
      swaps.write(QStringLiteral("Filename\tType\tSize\tUsed\tPriority\n"
                                 "/dev/zram0 partition 8388608 %1 100\n")
                      .arg(usedKiB)
                      .toUtf8());
      swaps.close();

      QFile disksize(zramRoot + QStringLiteral("/zram0/disksize"));
      QVERIFY(disksize.open(QIODevice::WriteOnly | QIODevice::Text));
      disksize.write("8589934592\n");
      disksize.close();

      QFile stat(zramRoot + QStringLiteral("/zram0/mm_stat"));
      QVERIFY(stat.open(QIODevice::WriteOnly | QIODevice::Text));
      stat.write(QStringLiteral("%1 0 %2 0 %2 0 0 0 0\n")
                     .arg(origBytes)
                     .arg(physicalBytes)
                     .toUtf8());
      stat.close();
    };

    // State 1: 512 MB swap used, 600 MB uncompressed, 200 MB compressed (3.0x
    // ratio)
    writeState(524288, 629145600, 209715200);

    qputenv("RO_CONTROL_SWAPS_PATH", swapsPath.toUtf8());
    qputenv("RO_CONTROL_ZRAM_SYSFS_ROOT", zramRoot.toUtf8());

    RamMonitor ram;
    ram.stop();
    ram.refresh();

    QVERIFY(ram.zramAvailable());
    QCOMPARE(ram.zramTotalMiB(), 8192);
    QCOMPARE(ram.zramUsedMiB(), 512);
    QCOMPARE(ram.zramPhysicalMiB(), 200);
    QCOMPARE(QString::number(ram.zramCompressionRatio(), 'f', 1),
             QStringLiteral("3.0"));

    // State 2: Dynamic change to 1024 MB swap used, 1500 MB uncompressed, 500
    // MB compressed (3.0x ratio)
    QSignalSpy spy(&ram, &RamMonitor::zramChanged);
    writeState(1048576, 1572864000, 524288000);
    ram.refresh();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(ram.zramUsedMiB(), 1024);
    QCOMPARE(ram.zramPhysicalMiB(), 500);

    qunsetenv("RO_CONTROL_SWAPS_PATH");
    qunsetenv("RO_CONTROL_ZRAM_SYSFS_ROOT");
  }
};

QTEST_MAIN(TestMonitor)
#include "test_monitor.moc"
