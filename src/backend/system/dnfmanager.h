#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class CommandRunner;

// DnfManager: DNF paket yöneticisi üzerinden kurulum, kaldırma ve sorgu işlemleri.
// Tüm DNF komutları CommandRunner üzerinden çalıştırılır.
class DnfManager : public QObject
{
    Q_OBJECT

public:
    struct PackageInfo {
        QString name;
        QString version;
        QString summary;
    };

    explicit DnfManager(QObject *parent = nullptr);

    // Paket kurulu mu?
    bool isInstalled(const QString &packageName) const;

    // Paket kur (root gerektirir)
    bool install(const QString &packageName);
    bool install(const QStringList &packages);

    // Paket kaldır (root gerektirir)
    bool remove(const QString &packageName);
    bool remove(const QStringList &packages);

    // Repo etkinleştir
    bool enableRepo(const QString &repoUrl);

    // Mevcut paket bilgisini al
    PackageInfo queryPackage(const QString &packageName) const;

    // DNF cache temizle (root gerektirir)
    bool cleanCache();

signals:
    void outputLine(const QString &line);
};
