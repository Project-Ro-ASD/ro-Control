#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

// CommandRunner: Tüm backend modüllerinin kullandığı shell komut çalıştırıcı.
// Hiçbir modül doğrudan sistem çağrısı yapmaz — hepsi bu sınıfı kullanır.
class CommandRunner : public QObject
{
    Q_OBJECT

public:
    struct Result {
        int     exitCode;
        QString stdout;
        QString stderr;
        bool success() const { return exitCode == 0; }
    };

    explicit CommandRunner(QObject *parent = nullptr);

    // Bloklayan komut — sonuç dönene kadar bekler
    Result run(const QString &program, const QStringList &args = {});

    // Root gerektiren komut — pkexec ile çalıştırır
    Result runAsRoot(const QString &program, const QStringList &args = {});

signals:
    // Uzun işlemler için anlık çıktı (DNF install vb.)
    void outputLine(const QString &line);
};
