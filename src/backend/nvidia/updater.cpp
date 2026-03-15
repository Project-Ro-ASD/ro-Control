#include "updater.h"
#include "detector.h"
#include "system/commandrunner.h"

#include <QRegularExpression>
#include <QStandardPaths>
#include <QtGlobal>

namespace {

QString commandError(const CommandRunner::Result &result,
                     const QString &fallback) {
  const QString stderrText = result.stderr.trimmed();
  const QString stdoutText = result.stdout.trimmed();

  if (!stderrText.isEmpty()) {
    return stderrText;
  }

  if (!stdoutText.isEmpty()) {
    return stdoutText;
  }

  return fallback;
}

} // namespace

NvidiaUpdater::NvidiaUpdater(QObject *parent) : QObject(parent) {}

void NvidiaUpdater::checkForUpdate() {
  // TR: Her kontrol denemesinde UI'ye gorunur bir baslangic mesaji gonder.
  // EN: Always emit a visible start message for each check request.
  emit progressMessage(QStringLiteral("Guncelleme kontrolu baslatildi..."));

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
    emit progressMessage(QStringLiteral("Kurulu NVIDIA surucusu bulunamadi."));
    return;
  }

  // TR: DNF cikis kodlari: 100=guncelleme var, 0=yok, digeri=hata.
  // EN: DNF exit codes: 100=updates available, 0=none, others=error.
  CommandRunner runner;
  if (QStandardPaths::findExecutable(QStringLiteral("dnf")).isEmpty()) {
    if (m_updateAvailable) {
      m_updateAvailable = false;
      emit updateAvailableChanged();
    }
    if (!m_latestVersion.isEmpty()) {
      m_latestVersion.clear();
      emit latestVersionChanged();
    }
    emit progressMessage(QStringLiteral("dnf bulunamadi."));
    return;
  }

  const auto result =
      runner.run(QStringLiteral("dnf"), {QStringLiteral("check-update"),
                                         QStringLiteral("akmod-nvidia")});

  // dnf check-update: exit code 100 = güncelleme var, 0 = yok
  if (result.exitCode == 100) {
    // Çıktıdan versiyon numarasını parse et
    // Format: "akmod-nvidia.x86_64    3:560.35.03-1.fc41
    // rpmfusion-nonfree-updates"
    static const QRegularExpression re(
        QStringLiteral(R"(akmod-nvidia\S*\s+\S*:?(\d+\.\d+[\.\d]*)\S*)"));

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

    if (!m_latestVersion.isEmpty()) {
      emit progressMessage(
          QStringLiteral("Guncelleme bulundu: %1").arg(m_latestVersion));
    } else {
      emit progressMessage(
          QStringLiteral("Guncelleme bulundu (surum ayrintisi alinamadi)."));
    }
  } else if (result.exitCode == 0) {
    if (m_updateAvailable) {
      m_updateAvailable = false;
      emit updateAvailableChanged();
    }
    if (!m_latestVersion.isEmpty()) {
      m_latestVersion.clear();
      emit latestVersionChanged();
    }

    emit progressMessage(
        QStringLiteral("Surucu guncel. Yeni surum bulunamadi."));
  } else {
    if (m_updateAvailable) {
      m_updateAvailable = false;
      emit updateAvailableChanged();
    }

    emit progressMessage(QStringLiteral("Guncelleme kontrolu basarisiz: %1")
                             .arg(result.stderr.trimmed().isEmpty()
                                      ? result.stdout.trimmed()
                                      : result.stderr.trimmed()));
  }
}

void NvidiaUpdater::applyUpdate() {
  CommandRunner runner;

  connect(&runner, &CommandRunner::outputLine, this,
          &NvidiaUpdater::progressMessage);

  // TR: Uzun surebilecek adimlar oncesinde kullaniciya ilerleme bilgisi ver.
  // EN: Emit progress updates before long-running operations.
  if (QStandardPaths::findExecutable(QStringLiteral("dnf")).isEmpty()) {
    emit updateFinished(false, QStringLiteral("dnf bulunamadi."));
    return;
  }

  emit progressMessage(QStringLiteral("NVIDIA sürücüsü güncelleniyor..."));

  auto result = runner.runAsRoot(
      QStringLiteral("dnf"), {QStringLiteral("update"), QStringLiteral("-y"),
                              QStringLiteral("akmod-nvidia")});

  if (!result.success()) {
    emit updateFinished(
        false, QStringLiteral("Guncelleme basarisiz: ") +
                   commandError(result, QStringLiteral("bilinmeyen hata")));
    return;
  }

  emit progressMessage(QStringLiteral("Kernel modülü yeniden derleniyor..."));

  result =
      runner.runAsRoot(QStringLiteral("akmods"), {QStringLiteral("--force")});
  if (!result.success()) {
    emit updateFinished(
        false, QStringLiteral("Kernel modulu derlenemedi: ") +
                   commandError(result, QStringLiteral("bilinmeyen hata")));
    return;
  }

  const QString sessionType =
      qEnvironmentVariable("XDG_SESSION_TYPE").trimmed().toLower();
  if (sessionType == QStringLiteral("wayland")) {
    emit progressMessage(QStringLiteral(
        "Wayland tespit edildi: nvidia-drm.modeset=1 ayari guncelleniyor..."));
    result = runner.runAsRoot(QStringLiteral("grubby"),
                              {QStringLiteral("--update-kernel=ALL"),
                               QStringLiteral("--args=nvidia-drm.modeset=1")});
    if (!result.success()) {
      emit updateFinished(
          false, QStringLiteral("Wayland kernel parametresi guncellenemedi: ") +
                     commandError(result, QStringLiteral("bilinmeyen hata")));
      return;
    }
  }

  // Güncelleme sonrası durumu yenile
  m_updateAvailable = false;
  emit updateAvailableChanged();

  checkForUpdate();

  emit updateFinished(
      true,
      QStringLiteral(
          "Sürücü başarıyla güncellendi. Lütfen sistemi yeniden başlatın."));
}
