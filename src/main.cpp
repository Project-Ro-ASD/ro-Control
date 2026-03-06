#include <QApplication>\n#include <QQmlApplicationEngine>\n#include <QQmlContext>\n#include <QIcon>

#include "backend/nvidia/detector.h"
#include "backend/nvidia/installer.h"
#include "backend/nvidia/updater.h"

int main(int argc, char *argv[])
{
    // QApplication: Widgets backend (pencere, sistem tray vb.) için gerekli
    QApplication app(argc, argv);

    // Uygulama meta bilgileri — Q_PROPERTY ve sistem entegrasyonunda kullanılır
    app.setApplicationName("ro-control");
    app.setApplicationDisplayName("ro-Control");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("Acik-Kaynak-Gelistirme-Toplulugu");
    app.setOrganizationDomain("github.com/Acik-Kaynak-Gelistirme-Toplulugu");
    app.setWindowIcon(QIcon::fromTheme("ro-control"));

    // Backend nesneleri
    NvidiaDetector detector;
    NvidiaInstaller installer;
    NvidiaUpdater updater;

    // QML motorunu başlat
    QQmlApplicationEngine engine;

    // Backend'i QML'e aç
    engine.rootContext()->setContextProperty("nvidiaDetector", &detector);
    engine.rootContext()->setContextProperty("nvidiaInstaller", &installer);
    engine.rootContext()->setContextProperty("nvidiaUpdater", &updater);

    // Ana QML dosyasını yükle
    using namespace Qt::StringLiterals;
    const QUrl url(u"qrc:/rocontrol/src/qml/Main.qml"_s);

    // QML yüklenemezse uygulamayı kapat
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
    );

    engine.load(url);

    return app.exec();
}
