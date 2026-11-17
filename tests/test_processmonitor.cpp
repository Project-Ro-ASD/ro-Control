#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include <unistd.h>

#include "monitor/processmonitor.h"

// Creates a minimal fake procfs fixture so the test is independent of the
// host OS:
//
//   <tmp>/stat              -> totals line ("cpu  ...")
//   <tmp>/123/stat          -> process 123 (comm "fixture-a", utime+stime=800)
//   <tmp>/123/cmdline       -> "/usr/bin/fixture-a\0--flag\0"
//   <tmp>/123/io            -> read_bytes/write_bytes
//   <tmp>/456/stat          -> process 456 (comm "fixture-b", utime+stime=100)
//   <tmp>/456/cmdline       -> "/usr/bin/fixture-b\0"
//   <tmp>/456/io            -> read_bytes/write_bytes
class TestProcessMonitor : public QObject {
  Q_OBJECT

private slots:
  void initTestCase() {
    QVERIFY(m_procDir.isValid());

    auto writeFile = [this](const QString &relativePath,
                            const QByteArray &contents) {
      const QString fullPath = m_procDir.filePath(relativePath);
      QDir().mkpath(QFileInfo(fullPath).absolutePath());
      QFile file(fullPath);
      QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
      file.write(contents);
      file.close();
    };

    writeFile(QStringLiteral("stat"),
              QByteArray("cpu  1000 200 300 400 100 0 50 0 0 0\n"));

    writeFile(QStringLiteral("123/stat"),
              QByteArray("123 (fixture-a) S 1 123 123 0 -1 4194304 "
                         "100 0 0 0 500 300 0 0 20 0 1 0 1000 1000000 "
                         "16384 0 0\n"));
    writeFile(QStringLiteral("123/cmdline"),
              QByteArray("/usr/bin/fixture-a\0--flag\0", 25));
    writeFile(QStringLiteral("123/io"),
              QByteArray("read_bytes: 4096\nwrite_bytes: 8192\n"));

    writeFile(QStringLiteral("456/stat"),
              QByteArray("456 (fixture-b) R 123 456 456 0 -1 4194304 "
                         "50 0 0 0 60 40 0 0 20 0 2 0 2000 2000000 "
                         "4096 0 0\n"));
    writeFile(QStringLiteral("456/cmdline"),
              QByteArray("/usr/bin/fixture-b\0", 18));
    writeFile(QStringLiteral("456/io"),
              QByteArray("read_bytes: 1024\nwrite_bytes: 2048\n"));

    qputenv("RO_CONTROL_PROC_PATH", m_procDir.path().toLocal8Bit());
  }

  void cleanupTestCase() { qunsetenv("RO_CONTROL_PROC_PATH"); }

  void testConstructionAndRefresh() {
    ProcessMonitor pm;
    pm.stop();
    pm.refresh();

    QCOMPARE(pm.running(), false);
    QVERIFY(pm.available());
    QVERIFY(!pm.allProcesses().isEmpty());
    QVERIFY(pm.processCount() >= 2);
  }

  void testLifecycleAndMinimumInterval() {
    ProcessMonitor pm;
    const int initialInterval = pm.updateInterval();
    QVERIFY(initialInterval >= 250);

    pm.stop();
    QVERIFY(!pm.running());

    pm.setUpdateInterval(200); // below minimum
    QCOMPARE(pm.updateInterval(), initialInterval);

    pm.setUpdateInterval(500);
    QCOMPARE(pm.updateInterval(), 500);

    pm.start();
    QVERIFY(pm.running());
  }

  void testTopProcessesSortedDescending() {
    ProcessMonitor pm;
    pm.stop();
    pm.refresh();
    pm.refresh(); // second sample for non-zero CPU delta

    const auto cpuTop = pm.cpuTopProcesses();
    QVERIFY(!cpuTop.isEmpty());
    for (int i = 1; i < cpuTop.size(); ++i) {
      const double prev =
          cpuTop.at(i - 1).toMap().value(QStringLiteral("cpuPercent")).toDouble();
      const double cur =
          cpuTop.at(i).toMap().value(QStringLiteral("cpuPercent")).toDouble();
      QVERIFY(prev >= cur);
    }

    const auto memTop = pm.memoryTopProcesses();
    QVERIFY(!memTop.isEmpty());
    for (int i = 1; i < memTop.size(); ++i) {
      const qint64 prev =
          memTop.at(i - 1).toMap().value(QStringLiteral("memoryBytes")).toLongLong();
      const qint64 cur =
          memTop.at(i).toMap().value(QStringLiteral("memoryBytes")).toLongLong();
      QVERIFY(prev >= cur);
    }
  }

  void testKillInvalidPid() {
    ProcessMonitor pm;
    QVERIFY(!pm.killProcess(0, 15));
    QVERIFY(!pm.killProcess(-1, 15));
  }

  void testReniceBoundaryValidation() {
    ProcessMonitor pm;
    QVERIFY(!pm.setProcessPriority(1, -21)); // below range
    QVERIFY(!pm.setProcessPriority(1, 20));  // above range
    QVERIFY(!pm.setProcessPriority(0, 0));   // invalid pid
  }

  void testStatusMessageSignal() {
    ProcessMonitor pm;
    QSignalSpy spy(&pm, &ProcessMonitor::statusMessageChanged);
    QVERIFY(spy.isValid());
    pm.setProcessPriority(0, 99); // triggers status message
    QVERIFY(spy.count() >= 1 || !pm.statusMessage().isEmpty());
  }

  void testParsedFields() {
    ProcessMonitor pm;
    pm.stop();
    pm.refresh();

    QVariantList all = pm.allProcesses();
    QVERIFY(all.size() >= 2);

    QHash<int, QVariantMap> byPid;
    for (const QVariant &entry : all) {
      const QVariantMap map = entry.toMap();
      byPid.insert(map.value(QStringLiteral("pid")).toInt(), map);
    }

    QVERIFY(byPid.contains(123));
    QVERIFY(byPid.contains(456));

    const QVariantMap a = byPid.value(123);
    QCOMPARE(a.value(QStringLiteral("name")).toString(),
             QStringLiteral("fixture-a"));
    QCOMPARE(a.value(QStringLiteral("state")).toString(), QStringLiteral("S"));
    QCOMPARE(a.value(QStringLiteral("ppid")).toInt(), 1);
    QCOMPARE(a.value(QStringLiteral("memoryBytes")).toLongLong(),
             16384LL * sysconf(_SC_PAGESIZE));
    QCOMPARE(a.value(QStringLiteral("nice")).toInt(), 0);
    QCOMPARE(a.value(QStringLiteral("threadCount")).toInt(), 1);
    QVERIFY(a.value(QStringLiteral("cmdline")).toString().contains(
        QStringLiteral("fixture-a")));
    QCOMPARE(a.value(QStringLiteral("readBytes")).toLongLong(), 4096);
    QCOMPARE(a.value(QStringLiteral("writeBytes")).toLongLong(), 8192);
  }

private:
  QTemporaryDir m_procDir;
};

QTEST_MAIN(TestProcessMonitor)
#include "test_processmonitor.moc"
