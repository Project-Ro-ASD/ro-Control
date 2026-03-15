#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QVariant>

#include "backend/monitor/cpumonitor.h"
#include "backend/monitor/gpumonitor.h"
#include "backend/monitor/rammonitor.h"
#include "backend/nvidia/detector.h"
#include "backend/nvidia/installer.h"
#include "backend/nvidia/updater.h"

int main(int argc, char *argv[]) {
  // TR: QApplication, Qt Widgets tabanli uygulama omurgasini baslatir.
  // EN: QApplication bootstraps the Qt Widgets application runtime.
  QApplication app(argc, argv);

  // TR: Bu meta bilgiler masaustu entegrasyonu ve UI kimligi icin kullanilir.
  // EN: These metadata values are used for desktop integration and app
  // identity.
  app.setApplicationName("ro-control");
  app.setApplicationDisplayName("ro-Control");
  app.setApplicationVersion("0.1.0");
  app.setOrganizationName("Project-Ro-ASD");
  app.setOrganizationDomain("github.com/Project-Ro-ASD");
  app.setWindowIcon(QIcon::fromTheme(
      "ro-control", QIcon(":/qt/qml/rocontrol/assets/ro-control-logo.svg")));

  NvidiaDetector detector;
  NvidiaInstaller installer;
  NvidiaUpdater updater;
  CpuMonitor cpuMonitor;
  GpuMonitor gpuMonitor;
  RamMonitor ramMonitor;

  // TR: QML motoru, arayuz ve bagli context nesnelerini yukler.
  // EN: The QML engine loads the UI and injected context objects.
  QQmlApplicationEngine engine;

  // TR: Ana pencerenin gerekli backend baglantilarini baslangicta enjekte et.
  // EN: Inject required backend bindings into the main QML root object.
  engine.setInitialProperties({
      {"nvidiaDetector", QVariant::fromValue(&detector)},
      {"nvidiaInstaller", QVariant::fromValue(&installer)},
      {"nvidiaUpdater", QVariant::fromValue(&updater)},
      {"cpuMonitor", QVariant::fromValue(&cpuMonitor)},
      {"gpuMonitor", QVariant::fromValue(&gpuMonitor)},
      {"ramMonitor", QVariant::fromValue(&ramMonitor)},
  });

  // TR: Ana bileşen olusmazsa uygulamayi kontrollu sekilde sonlandir.
  // EN: Exit gracefully if the root QML component cannot be created.
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  // TR: Modulden yukleme, qrc yol/prefix farklarina karsi daha dayaniklidir.
  // EN: Module-based loading is resilient to qrc path/prefix differences.
  engine.loadFromModule("rocontrol", "Main");

  return app.exec();
}
