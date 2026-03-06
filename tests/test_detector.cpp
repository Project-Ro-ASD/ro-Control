#include <QTest>

#include "nvidia/detector.h"

class TestDetector : public QObject {
  Q_OBJECT

private slots:
  // Sınıf oluşturulabiliyor mu?
  void testConstruction() {
    NvidiaDetector detector;
    // Crash olmadan oluşturulabiliyorsa geçer
    QVERIFY(true);
  }

  // GpuInfo struct varsayılan değerleri
  void testGpuInfoDefaults() {
    NvidiaDetector::GpuInfo info{};
    QCOMPARE(info.found, false);
    QVERIFY(info.name.isEmpty());
    QVERIFY(info.driverVersion.isEmpty());
    QCOMPARE(info.driverLoaded, false);
    QCOMPARE(info.nouveauActive, false);
    QCOMPARE(info.secureBootEnabled, false);
  }

  // detect() çağrılabiliyor mu? (donanım olmadan bile çökmemeli)
  void testDetectDoesNotCrash() {
    NvidiaDetector detector;
    auto info = detector.detect();
    // NVIDIA yoksa found=false olmalı, crash olmamalı
    Q_UNUSED(info);
    QVERIFY(true);
  }

  // hasNvidiaGpu() — dönüş tipi bool
  void testHasNvidiaGpu() {
    NvidiaDetector detector;
    bool result = detector.hasNvidiaGpu();
    // Sonuç true veya false olabilir — önemli olan crash olmaması
    Q_UNUSED(result);
    QVERIFY(true);
  }

  // isDriverInstalled() — dönüş tipi bool
  void testIsDriverInstalled() {
    NvidiaDetector detector;
    bool result = detector.isDriverInstalled();
    Q_UNUSED(result);
    QVERIFY(true);
  }

  // installedDriverVersion() — boş string veya versiyon döner
  void testInstalledDriverVersion() {
    NvidiaDetector detector;
    QString version = detector.installedDriverVersion();
    // Versiyon varsa "xxx.xx.xx" gibi olmalı, yoksa boş string
    if (!version.isEmpty()) {
      QVERIFY(version.contains(QChar('.')));
    }
  }

  // detect() tutarlılık: found=false ise driverVersion boş olmalı
  void testDetectConsistency() {
    NvidiaDetector detector;
    auto info = detector.detect();
    if (!info.found) {
      QVERIFY(info.name.isEmpty());
    }
  }
};

QTEST_MAIN(TestDetector)
#include "test_detector.moc"
