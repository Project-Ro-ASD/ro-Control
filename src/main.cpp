#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>

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

    // QML motorunu başlat
    QQmlApplicationEngine engine;

    // Ana QML dosyasını yükle
    const QUrl url(u"qrc:/rocontrol/src/qml/Main.qml"_qs);

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
