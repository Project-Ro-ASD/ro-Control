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
    QCOMPARE(info.driverLoaded, false);
    QCOMPARE(info.nouveauActive, false);
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
    QCOMPARE(installed, !detector.installedDriverVersion().isEmpty());
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
    if (info.driverVersion.isEmpty()) {
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
};

QTEST_MAIN(TestDetector)
#include "test_detector.moc"
