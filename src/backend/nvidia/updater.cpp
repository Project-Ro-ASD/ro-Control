#include "updater.h"
#include "detector.h"
#include "system/commandrunner.h"
#include "versionparser.h"

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

void NvidiaUpdater::setLatestVersion(const QString &version) {
  if (m_latestVersion == version) {
    return;
  }

  m_latestVersion = version;
  emit latestVersionChanged();
}

void NvidiaUpdater::setAvailableVersions(const QStringList &versions) {
  if (m_availableVersions == versions) {
    return;
  }

  m_availableVersions = versions;
  emit availableVersionsChanged();
}

QString NvidiaUpdater::detectSessionType() const {
  const QString envType =
      qEnvironmentVariable("XDG_SESSION_TYPE").trimmed().toLower();
  if (!envType.isEmpty()) {
    return envType;
  }

  CommandRunner runner;
  const auto loginctl =
      runner.run(QStringLiteral("loginctl"),
                 {QStringLiteral("show-session"),
                  qEnvironmentVariable("XDG_SESSION_ID"), QStringLiteral("-p"),
                  QStringLiteral("Type"), QStringLiteral("--value")});

  if (loginctl.success()) {
    const QString type = loginctl.stdout.trimmed().toLower();
    if (!type.isEmpty()) {
      return type;
    }
  }

  return QStringLiteral("unknown");
}

QStringList
NvidiaUpdater::buildDriverTargets(const QString &version,
                                  const QString &sessionType) const {
  QStringList targets;
  targets << NvidiaVersionParser::packageSpecForVersion(
      QStringLiteral("akmod-nvidia"), version);

  if (sessionType == QStringLiteral("x11")) {
    targets << NvidiaVersionParser::packageSpecForVersion(
        QStringLiteral("xorg-x11-drv-nvidia"), version);
  }

  return targets;
}

bool NvidiaUpdater::finalizeDriverChange(CommandRunner &runner,
                                         const QString &sessionType,
                                         QString *errorMessage) {
  auto result =
      runner.runAsRoot(QStringLiteral("akmods"), {QStringLiteral("--force")});
  if (!result.success()) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("Kernel modulu derlenemedi: ") +
                      commandError(result, QStringLiteral("bilinmeyen hata"));
    }
    return false;
  }

  if (sessionType == QStringLiteral("wayland")) {
    emit progressMessage(QStringLiteral(
        "Wayland tespit edildi: nvidia-drm.modeset=1 ayari guncelleniyor..."));
    result = runner.runAsRoot(QStringLiteral("grubby"),
                              {QStringLiteral("--update-kernel=ALL"),
                               QStringLiteral("--args=nvidia-drm.modeset=1")});
    if (!result.success()) {
      if (errorMessage != nullptr) {
        *errorMessage =
            QStringLiteral("Wayland kernel parametresi guncellenemedi: ") +
            commandError(result, QStringLiteral("bilinmeyen hata"));
      }
      return false;
    }
  }

  return true;
}

void NvidiaUpdater::refreshAvailableVersions() {
  if (QStandardPaths::findExecutable(QStringLiteral("dnf")).isEmpty()) {
    setAvailableVersions({});
    emit progressMessage(QStringLiteral("dnf bulunamadi."));
    return;
  }

  CommandRunner runner;
  const auto result =
      runner.run(QStringLiteral("dnf"),
                 {QStringLiteral("list"), QStringLiteral("--showduplicates"),
                  QStringLiteral("akmod-nvidia")});

  if (!result.success()) {
    setAvailableVersions({});
    emit progressMessage(
        QStringLiteral("Surum listesi alinamadi: %1")
            .arg(commandError(result, QStringLiteral("bilinmeyen hata"))));
    return;
  }

  const QStringList versions =
      NvidiaVersionParser::parseAvailablePackageVersions(
          result.stdout, QStringLiteral("akmod-nvidia"));
  setAvailableVersions(versions);

  if (!versions.isEmpty()) {
    emit progressMessage(
        QStringLiteral("Kullanilabilir surum sayisi: %1").arg(versions.size()));
  } else {
    emit progressMessage(QStringLiteral("Kullanilabilir surum bulunamadi."));
  }
}

void NvidiaUpdater::checkForUpdate() {
  // TR: Her kontrol denemesinde UI'ye gorunur bir baslangic mesaji gonder.
  // EN: Always emit a visible start message for each check request.
  emit progressMessage(QStringLiteral("Guncelleme kontrolu baslatildi..."));

  refreshAvailableVersions();

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
    setLatestVersion({});
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
    setLatestVersion({});
    emit progressMessage(QStringLiteral("dnf bulunamadi."));
    return;
  }

  const auto result =
      runner.run(QStringLiteral("dnf"), {QStringLiteral("check-update"),
                                         QStringLiteral("akmod-nvidia")});

  // dnf check-update: exit code 100 = güncelleme var, 0 = yok
  if (result.exitCode == 100) {
    const QString latest = NvidiaVersionParser::parseCheckUpdateVersion(
        result.stdout, QStringLiteral("akmod-nvidia"));
    setLatestVersion(latest);

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
    setLatestVersion({});

    emit progressMessage(
        QStringLiteral("Surucu guncel. Yeni surum bulunamadi."));
  } else {
    if (m_updateAvailable) {
      m_updateAvailable = false;
      emit updateAvailableChanged();
    }
    setLatestVersion({});

    emit progressMessage(QStringLiteral("Guncelleme kontrolu basarisiz: %1")
                             .arg(result.stderr.trimmed().isEmpty()
                                      ? result.stdout.trimmed()
                                      : result.stderr.trimmed()));
  }
}

void NvidiaUpdater::applyUpdate() { applyVersion(QString()); }

void NvidiaUpdater::applyVersion(const QString &version) {
  CommandRunner runner;

  connect(&runner, &CommandRunner::outputLine, this,
          &NvidiaUpdater::progressMessage);

  // TR: Uzun surebilecek adimlar oncesinde kullaniciya ilerleme bilgisi ver.
  // EN: Emit progress updates before long-running operations.
  if (QStandardPaths::findExecutable(QStringLiteral("dnf")).isEmpty()) {
    emit updateFinished(false, QStringLiteral("dnf bulunamadi."));
    return;
  }

  NvidiaDetector detector;
  const QString installedVersion = detector.installedDriverVersion();
  const QString sessionType = detectSessionType();
  const QString trimmedVersion = version.trimmed();
  const QStringList packageTargets =
      buildDriverTargets(trimmedVersion, sessionType);

  if (!trimmedVersion.isEmpty() &&
      !m_availableVersions.contains(trimmedVersion)) {
    emit updateFinished(
        false, QStringLiteral("Secilen surum repo listesinde bulunamadi."));
    return;
  }

  emit progressMessage(
      trimmedVersion.isEmpty()
          ? QStringLiteral("NVIDIA surucusu en son surume guncelleniyor...")
          : QStringLiteral("NVIDIA surucusu secilen surume geciriliyor: %1")
                .arg(trimmedVersion));

  auto args =
      QStringList{(trimmedVersion.isEmpty() && !installedVersion.isEmpty())
                      ? QStringLiteral("update")
                      : QStringLiteral("install"),
                  QStringLiteral("-y")};
  args << packageTargets;

  auto result = runner.runAsRoot(QStringLiteral("dnf"), args);

  if (!result.success()) {
    emit updateFinished(
        false, QStringLiteral("Guncelleme basarisiz: ") +
                   commandError(result, QStringLiteral("bilinmeyen hata")));
    return;
  }

  emit progressMessage(QStringLiteral("Kernel modülü yeniden derleniyor..."));

  QString finalizeError;
  if (!finalizeDriverChange(runner, sessionType, &finalizeError)) {
    emit updateFinished(false, finalizeError);
    return;
  }

  // Güncelleme sonrası durumu yenile
  m_updateAvailable = false;
  emit updateAvailableChanged();

  checkForUpdate();

  emit updateFinished(
      true, trimmedVersion.isEmpty()
                ? (installedVersion.isEmpty()
                       ? QStringLiteral("En son surum basariyla kuruldu. "
                                        "Lutfen sistemi yeniden baslatin.")
                       : QStringLiteral("Surucu basariyla guncellendi. Lutfen "
                                        "sistemi yeniden baslatin."))
                : QStringLiteral("Secilen surum basariyla uygulandi. Lutfen "
                                 "sistemi yeniden baslatin."));
}
