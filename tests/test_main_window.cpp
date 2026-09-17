#include <QCoreApplication>
#include <QGuiApplication>
#include <QSignalSpy>
#include <QTest>
#include <QWindow>

#include <functional>

#include "monitor/cpumonitor.h"
#include "monitor/gpumonitor.h"
#include "monitor/rammonitor.h"

class MainWindowThrottleTest : public QObject {
  Q_OBJECT

private slots:
  void visibilityThrottleSurvivesScope();
};

// Mirrors the wiring in main(): a local throttler list is captured by the
// visibility lambda that outlives its block scope. Regression test for a
// startup/close segfault where the lambda dereferenced the destroyed local
// list when the window emitted visibleChanged during its destroy().
static QWindow *wireThrottle(CpuMonitor &cpu, GpuMonitor &gpu,
                             RamMonitor &ram) {
  auto *window = new QWindow();

  const QList<std::function<void(int)>> throttlers{
      [&](int ms) { cpu.setUpdateInterval(ms); },
      [&](int ms) { gpu.setUpdateInterval(ms); },
      [&](int ms) { ram.setUpdateInterval(ms); }};

  const auto applyVisibility = [throttlers](bool visible) {
    for (const auto &throttler : throttlers) {
      throttler(visible ? 1000 : 5000);
    }
  };

  QObject::connect(
      window, &QWindow::visibleChanged, window,
      [applyVisibility](bool visible) { applyVisibility(visible); });

  return window;
}

void MainWindowThrottleTest::visibilityThrottleSurvivesScope() {
  CpuMonitor cpu;
  GpuMonitor gpu;
  RamMonitor ram;

  QWindow *window = wireThrottle(cpu, gpu, ram);

  QSignalSpy visibleSpy(window, &QWindow::visibleChanged);

  window->show();
  QCoreApplication::processEvents();
  window->hide();
  QCoreApplication::processEvents();

  // Destroying the window is what exercised the dangling capture in the
  // original crash: QWindowPrivate::destroy() emits visibleChanged.
  window->visibleChanged(false);
  QCoreApplication::processEvents();
  delete window;
  QCoreApplication::processEvents();

  QVERIFY(cpu.updateInterval() == 1000 || cpu.updateInterval() == 5000);
  QVERIFY(gpu.updateInterval() == 1000 || gpu.updateInterval() == 5000);
  QVERIFY(ram.updateInterval() == 1000 || ram.updateInterval() == 5000);
}

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  MainWindowThrottleTest test;
  QTEST_SET_MAIN_SOURCE_PATH
  return QTest::qExec(&test, argc, argv);
}

#include "test_main_window.moc"