#include "updater.h"
#include "detector.h"
#include "system/commandrunner.h"

#include <QRegularExpression>

NvidiaUpdater::NvidiaUpdater(QObject *parent)
    : QObject(parent)
{}

void NvidiaUpdater::checkForUpdate()
{
    // Mevcut kurulu sürücü versiyonu
    NvidiaDetector detector;
    const QString current = detector.installedDriverVersion();

    if (current != m_currentVersion) {
        m_currentVersion = current;
        emit currentVersionChanged();
    }

    if (m_currentVersion.isEmpty()) {
        // Sürücü kurulu değil — güncelleme kontrolü anlamsız
        if (m_updateAvailable) {
            m_updateAvailable = false;
            emit updateAvailableChanged();
        }
        return;
    }

    // DNF'den güncelleme bilgisi al
    CommandRunner runner;

    const auto result = runner.run(
        QStringLiteral("dnf"),
        { QStringLiteral("check-update"), QStringLiteral("akmod-nvidia") }
    );

    // dnf check-update: exit code 100 = güncelleme var, 0 = yok
    if (result.exitCode == 100) {
        // Çıktıdan versiyon numarasını parse et
        // Format: "akmod-nvidia.x86_64    3:560.35.03-1.fc41    rpmfusion-nonfree-updates"
        static const QRegularExpression re(
            QStringLiteral(R"(akmod-nvidia\S*\s+\S*:?(\d+\.\d+[\.\d]*)\S*)")
        );

        const auto match = re.match(result.stdout);
        const QString latest = match.hasMatch() ? match.captured(1) : QString();

        if (!latest.isEmpty() && latest != m_latestVersion) {
            m_latestVersion = latest;
            emit latestVersionChanged();
        }

        if (!m_updateAvailable) {
            m_updateAvailable = true;
            emit updateAvailableChanged();
        }
    } else {
        if (m_updateAvailable) {
            m_updateAvailable = false;
            emit updateAvailableChanged();
        }
    }
}

void NvidiaUpdater::applyUpdate()
{
    CommandRunner runner;

    connect(&runner, &CommandRunner::outputLine,
            this, &NvidiaUpdater::progressMessage);

    emit progressMessage(QStringLiteral("NVIDIA sürücüsü güncelleniyor..."));

    const auto result = runner.runAsRoot(
        QStringLiteral("dnf"),
        { QStringLiteral("update"), QStringLiteral("-y"), QStringLiteral("akmod-nvidia") }
    );

    if (!result.success()) {
        emit updateFinished(false, QStringLiteral("Güncelleme başarısız: ") + result.stderr);
        return;
    }

    emit progressMessage(QStringLiteral("Kernel modülü yeniden derleniyor..."));

    runner.runAsRoot(QStringLiteral("akmods"), { QStringLiteral("--force") });

    // Güncelleme sonrası durumu yenile
    m_updateAvailable = false;
    emit updateAvailableChanged();

    checkForUpdate();

    emit updateFinished(true, QStringLiteral("Sürücü başarıyla güncellendi. Lütfen sistemi yeniden başlatın."));
}
