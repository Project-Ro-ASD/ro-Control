#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include "nvidia/detector.h"

class TestDetector : public QObject {
  Q_OBJECT

private slots:
  void testConstruction() {
    NvidiaDetector detector;
    Q_UNUSED(detector);
    QVERIFY(true);
  }

  void testGpuInfoDefaults() {
    NvidiaDetector::GpuInfo info{};
    QCOMPARE(info.found, false);
    QVERIFY(info.name.isEmpty());
    QVERIFY(info.driverVersion.isEmpty());
    QCOMPARE(info.driverPackageInstalled, false);
    QCOMPARE(info.driverLoaded, false);
    QCOMPARE(info.openKernelModulesInstalled, false);
    QCOMPARE(info.secureBootEnabled, false);
  }

  void testDetectDoesNotCrash() {
    NvidiaDetector detector;
    auto info = detector.detect();
    Q_UNUSED(info);
    QVERIFY(true);
  }

  void testHasNvidiaGpu() {
    NvidiaDetector detector;
    const bool hasGpu = detector.hasNvidiaGpu();
    const auto info = detector.detect();
    QCOMPARE(hasGpu, info.found);
  }

  void testIsDriverInstalled() {
    NvidiaDetector detector;
    const bool installed = detector.isDriverInstalled();
    const auto info = detector.detect();
    QCOMPARE(installed, !detector.installedDriverVersion().isEmpty() ||
                            info.driverPackageInstalled);
  }

  void testInstalledDriverVersion() {
    NvidiaDetector detector;
    QString version = detector.installedDriverVersion();
    if (!version.isEmpty()) {
      QVERIFY(version.contains(QChar('.')));
    }
  }

  void testDetectConsistency() {
    NvidiaDetector detector;
    auto info = detector.detect();
    if (!info.found) {
      QVERIFY(info.name.isEmpty());
    }
    if (info.driverVersion.isEmpty() && !info.driverPackageInstalled) {
      QVERIFY(!detector.isDriverInstalled());
    }
  }

  // verificationReport() en azından temel güvenlik bilgisini döndürmeli.
  void testVerificationReport() {
    NvidiaDetector detector;
    detector.refresh();
    const QString report = detector.verificationReport();
    QVERIFY(!report.isEmpty());
    QVERIFY(report.contains(QStringLiteral("GPU: ")));
    QVERIFY(report.contains(QStringLiteral("Driver Version: ")));
    QVERIFY(report.contains(QStringLiteral("Secure Boot")));
  }

  void testActiveDriverStringIsNeverEmpty() {
    NvidiaDetector detector;
    detector.refresh();
    QVERIFY(!detector.activeDriver().trimmed().isEmpty());
  }

  void testOpenKernelModuleIsNotMisclassifiedAsMixed() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString rpmPath = tempDir.filePath(QStringLiteral("fake-rpm.sh"));
    QFile rpm(rpmPath);
    QVERIFY(rpm.open(QIODevice::WriteOnly | QIODevice::Text));
    rpm.write("#!/bin/sh\n"
              "printf 'akmod-nvidia-open|0:570.1-1.fc42\\n'\n"
              "printf 'xorg-x11-drv-nvidia|0:570.1-1.fc42\\n'\n");
    rpm.close();
    QVERIFY(QFile::setPermissions(rpmPath, QFileDevice::ReadOwner |
                                               QFileDevice::WriteOwner |
                                               QFileDevice::ExeOwner));

    const QString modulesPath = tempDir.filePath(QStringLiteral("modules"));
    QFile modules(modulesPath);
    QVERIFY(modules.open(QIODevice::WriteOnly | QIODevice::Text));
    modules.write("nvidia 1 0 - Live 0x0\n");
    modules.close();

    const QString versionPath =
        tempDir.filePath(QStringLiteral("nvidia-version"));
    QFile version(versionPath);
    QVERIFY(version.open(QIODevice::WriteOnly | QIODevice::Text));
    version.write("NVRM version: NVIDIA UNIX Open Kernel Module for x86_64\n");
    version.close();

    qputenv("RO_CONTROL_COMMAND_RPM", rpmPath.toUtf8());
    qputenv("RO_CONTROL_PROC_MODULES_PATH", modulesPath.toUtf8());
    qputenv("RO_CONTROL_NVIDIA_PROC_VERSION_PATH", versionPath.toUtf8());
    qputenv("RO_CONTROL_NVIDIA_OPENRM_PATH",
            tempDir.filePath(QStringLiteral("missing-openrm")).toUtf8());

    NvidiaDetector detector;
    detector.setDetectionResult(detector.detect());
    QCOMPARE(detector.installedDriverSource(), QStringLiteral("open-source"));

    qunsetenv("RO_CONTROL_COMMAND_RPM");
    qunsetenv("RO_CONTROL_PROC_MODULES_PATH");
    qunsetenv("RO_CONTROL_NVIDIA_PROC_VERSION_PATH");
    qunsetenv("RO_CONTROL_NVIDIA_OPENRM_PATH");
  }

  void testDriverSourceClassification() {
    NvidiaDetector detector;
    NvidiaDetector::GpuInfo info;

    info.closedSourceDriverInstalled = true;
    detector.setDetectionResult(info);
    QCOMPARE(detector.installedDriverSource(), QStringLiteral("closed-source"));
    QCOMPARE(detector.installedDriverSourceLabel(),
             QStringLiteral("NVIDIA Proprietary Kernel Module detected"));

    info.closedSourceDriverInstalled = false;
    info.openSourceDriverInstalled = true;
    detector.setDetectionResult(info);
    QCOMPARE(detector.installedDriverSource(), QStringLiteral("open-source"));
    QCOMPARE(detector.installedDriverSourceLabel(),
             QStringLiteral("NVIDIA Open Kernel Modules detected"));

    info.closedSourceDriverInstalled = true;
    detector.setDetectionResult(info);
    QCOMPARE(detector.installedDriverSource(), QStringLiteral("mixed"));
  }

  void testSecureBootEfivarOverride() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString efivarPath =
        tempDir.filePath(QStringLiteral("SecureBoot-test"));
    QFile file(efivarPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write(QByteArray::fromHex("0700000001")) == 5);
    file.close();

    qputenv("RO_CONTROL_SECURE_BOOT_EFIVAR_PATH", efivarPath.toUtf8());

    NvidiaDetector detector;
    const auto info = detector.detect();
    QVERIFY(info.secureBootKnown);
    QVERIFY(info.secureBootEnabled);

    qunsetenv("RO_CONTROL_SECURE_BOOT_EFIVAR_PATH");
  }

  void testSecureBootEfivarDisabledOverride() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString efivarPath =
        tempDir.filePath(QStringLiteral("SecureBoot-disabled-test"));
    QFile file(efivarPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write(QByteArray::fromHex("0700000000")) == 5);
    file.close();

    qputenv("RO_CONTROL_SECURE_BOOT_EFIVAR_PATH", efivarPath.toUtf8());

    NvidiaDetector detector;
    const auto info = detector.detect();
    QVERIFY(info.secureBootKnown);
    QVERIFY(!info.secureBootEnabled);

    qunsetenv("RO_CONTROL_SECURE_BOOT_EFIVAR_PATH");
  }

  void testCleanGpuName() {
    // 1. Bracketed NVIDIA GPU with chip code
    QCOMPARE(NvidiaDetector::cleanGpuName(
                 QStringLiteral("TU106 [GeForce RTX 2060 SUPER]")),
             QStringLiteral("NVIDIA GeForce RTX 2060 SUPER"));

    // 2. Already clean NVIDIA GPU
    QCOMPARE(NvidiaDetector::cleanGpuName(
                 QStringLiteral("NVIDIA GeForce RTX 2060 SUPER")),
             QStringLiteral("NVIDIA GeForce RTX 2060 SUPER"));

    // 3. Bracketed with revision suffix and corporation prefix
    QCOMPARE(
        NvidiaDetector::cleanGpuName(
            QStringLiteral(
                "NVIDIA Corporation TU106 [GeForce RTX 2060 SUPER] (rev a1)"),
            QStringLiteral("NVIDIA Corporation")),
        QStringLiteral("NVIDIA GeForce RTX 2060 SUPER"));

    // 4. Ada Lovelace RTX 4090
    QCOMPARE(NvidiaDetector::cleanGpuName(
                 QStringLiteral("AD102 [GeForce RTX 4090]")),
             QStringLiteral("NVIDIA GeForce RTX 4090"));

    // 5. Intel integrated graphics
    QCOMPARE(NvidiaDetector::cleanGpuName(
                 QStringLiteral("Raptor Lake-S GT1 [UHD Graphics 770]"),
                 QStringLiteral("Intel Corporation")),
             QStringLiteral("Intel UHD Graphics 770"));

    // 6. AMD Radeon GPU
    QCOMPARE(NvidiaDetector::cleanGpuName(
                 QStringLiteral("Navi 21 [Radeon RX 6800/6800 XT / 6900 XT]"),
                 QStringLiteral("Advanced Micro Devices, Inc. [AMD/ATI]")),
             QStringLiteral("AMD Radeon RX 6800/6800 XT / 6900 XT"));

    // 7. Quadro GPU
    QCOMPARE(NvidiaDetector::cleanGpuName(QStringLiteral("Quadro RTX 4000")),
             QStringLiteral("NVIDIA Quadro RTX 4000"));
  }

  void testGpuInfoEqualityOperator() {
    NvidiaDetector::GpuInfo a{};
    NvidiaDetector::GpuInfo b{};
    QVERIFY(a == b);

    a.found = true;
    a.name = QStringLiteral("NVIDIA GeForce RTX 4090");
    QVERIFY(!(a == b));

    b.found = true;
    b.name = QStringLiteral("NVIDIA GeForce RTX 4090");
    QVERIFY(a == b);

    a.driverVersion = QStringLiteral("580.126.18");
    QVERIFY(!(a == b));
    b.driverVersion = QStringLiteral("580.126.18");
    QVERIFY(a == b);
  }

  void testRefreshSuppressesSpuriousInfoChanged() {
    NvidiaDetector detector;
    QSignalSpy spy(&detector, &NvidiaDetector::infoChanged);

    detector.refresh();
    const int firstCount = spy.count();
    QVERIFY(firstCount >= 0);

    // Subsequent refresh with unchanged hardware state must not emit spurious
    // signals
    detector.refresh();
    QCOMPARE(spy.count(), firstCount);
  }

  void testSetDetectionResultSuppressesSpuriousInfoChanged() {
    NvidiaDetector detector;
    QSignalSpy spy(&detector, &NvidiaDetector::infoChanged);

    NvidiaDetector::GpuInfo info;
    info.found = true;
    info.name = QStringLiteral("RTX 4090");
    info.driverVersion = QStringLiteral("580.126.18");

    detector.setDetectionResult(info);
    QCOMPARE(spy.count(), 1);

    // Setting identical result must not re-emit
    detector.setDetectionResult(info);
    QCOMPARE(spy.count(), 1);

    // Modifying one field must emit
    info.driverLoaded = true;
    detector.setDetectionResult(info);
    QCOMPARE(spy.count(), 2);
  }
};

QTEST_MAIN(TestDetector)
#include "test_detector.moc"
