#pragma once

#include <QObject>
#include <QString>

// CommandRunner: Shell komutlarını çalıştırır, stdout/stderr döner.
// Tüm backend modülleri bu sınıfı kullanır — doğrudan sistem() çağrısı yapılmaz.
class CommandRunner : public QObject
{
    Q_OBJECT

public:
    struct Result {
        int     exitCode;   // 0 = başarılı
        QString stdout;     // Komut çıktısı
        QString stderr;     // Hata çıktısı
        bool    success() const { return exitCode == 0; }
    };

    explicit CommandRunner(QObject *parent = nullptr);

    // Komutu çalıştır ve sonucu döndür (bloklayan)
    Result run(const QString &command, const QStringList &args = {});

signals:
    // Uzun süren işlemler için anlık çıktı satırı
    void outputLine(const QString &line);
};
