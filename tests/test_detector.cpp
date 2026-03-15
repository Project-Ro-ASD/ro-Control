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
    bool result = detector.hasNvidiaGpu();
    Q_UNUSED(result);
    QVERIFY(true);
  }

  void testIsDriverInstalled() {
    NvidiaDetector detector;
    bool result = detector.isDriverInstalled();
    Q_UNUSED(result);
    QVERIFY(true);
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
  }

  // verificationReport() en azından temel güvenlik bilgisini döndürmeli.
  void testVerificationReport() {
    NvidiaDetector detector;
    detector.refresh();
    const QString report = detector.verificationReport();
    QVERIFY(!report.isEmpty());
    QVERIFY(report.contains(QStringLiteral("Secure Boot")));
  }
};

QTEST_MAIN(TestDetector)
#include "test_detector.moc"
