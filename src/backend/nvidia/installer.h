#pragma once

#include <QObject>
#include <QString>

// NvidiaInstaller: DNF üzerinden NVIDIA sürücü kurulum/kaldırma işlemleri.
// Tüm işlemler root gerektirir — polkit üzerinden yetki alınır.
class NvidiaInstaller : public QObject
{
    Q_OBJECT

public:
    explicit NvidiaInstaller(QObject *parent = nullptr);

    // Sürücüyü kur (akmod-nvidia)
    Q_INVOKABLE void install();

    // Sürücüyü kaldır
    Q_INVOKABLE void remove();

    // Eski sürücü kalıntılarını temizle
    Q_INVOKABLE void deepClean();

signals:
    // İşlem adımları — QML'e ilerleme göstermek için
    void progressMessage(const QString &message);

    // İşlem tamamlandı
    void installFinished(bool success, const QString &message);
    void removeFinished(bool success, const QString &message);
};
