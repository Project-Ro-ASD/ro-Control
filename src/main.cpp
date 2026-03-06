#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "backend/monitor/cpumonitor.h"
#include "backend/monitor/gpumonitor.h"
#include "backend/monitor/rammonitor.h"
#include "backend/nvidia/detector.h"
#include "backend/nvidia/installer.h"
#include "backend/nvidia/updater.h"

int main(int argc, char *argv[]) {
  // QApplication: Widgets backend (pencere, sistem tray vb.) için gerekli
  QApplication app(argc, argv);

  // Uygulama meta bilgileri — Q_PROPERTY ve sistem entegrasyonunda kullanılır
  app.setApplicationName("ro-control");
  app.setApplicationDisplayName("ro-Control");
  app.setApplicationVersion("0.1.0");
  app.setOrganizationName("Acik-Kaynak-Gelistirme-Toplulugu");
  app.setOrganizationDomain("github.com/Acik-Kaynak-Gelistirme-Toplulugu");
  app.setWindowIcon(QIcon::fromTheme("ro-control"));

  NvidiaDetector detector;
  NvidiaInstaller installer;
  NvidiaUpdater updater;
  CpuMonitor cpuMonitor;
  GpuMonitor gpuMonitor;
  RamMonitor ramMonitor;

  // QML motorunu başlat
  QQmlApplicationEngine engine;

  engine.rootContext()->setContextProperty("nvidiaDetector", &detector);
  engine.rootContext()->setContextProperty("nvidiaInstaller", &installer);
  engine.rootContext()->setContextProperty("nvidiaUpdater", &updater);
  engine.rootContext()->setContextProperty("cpuMonitor", &cpuMonitor);
  engine.rootContext()->setContextProperty("gpuMonitor", &gpuMonitor);
  engine.rootContext()->setContextProperty("ramMonitor", &ramMonitor);

  // QML yüklenemezse uygulamayı kapat
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  // QML modülünden yüklemek, qrc prefix/policy farklarından etkilenmez.
  engine.loadFromModule("rocontrol", "Main");

  return app.exec();
}
