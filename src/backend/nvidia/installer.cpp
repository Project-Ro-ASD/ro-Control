#include "installer.h"

#include "system/commandrunner.h"

#include <QtGlobal>

NvidiaInstaller::NvidiaInstaller(QObject *parent) : QObject(parent) {
  refreshProprietaryAgreement();
}

void NvidiaInstaller::setProprietaryAgreement(bool required,
                                              const QString &text) {
  if (m_proprietaryAgreementRequired == required &&
      m_proprietaryAgreementText == text) {
    return;
  }

  m_proprietaryAgreementRequired = required;
  m_proprietaryAgreementText = text;
  emit proprietaryAgreementChanged();
}

void NvidiaInstaller::refreshProprietaryAgreement() {
  CommandRunner runner;
  const auto info =
      runner.run(QStringLiteral("dnf"),
                 {QStringLiteral("info"), QStringLiteral("akmod-nvidia")});

  if (!info.success()) {
    setProprietaryAgreement(false, QString());
    return;
  }

  QString licenseLine;
  const QStringList lines = info.stdout.split(QLatin1Char('\n'));
  for (const QString &line : lines) {
    if (line.startsWith(QStringLiteral("License"), Qt::CaseInsensitive)) {
      licenseLine = line;
      break;
    }
  }

  const QString lowered = licenseLine.toLower();
  const bool requiresAgreement =
      lowered.contains(QStringLiteral("eula")) ||
      lowered.contains(QStringLiteral("proprietary")) ||
      lowered.contains(QStringLiteral("nvidia"));

  if (requiresAgreement) {
    setProprietaryAgreement(
        true, QStringLiteral(
                  "Kapali kaynak NVIDIA surucusunu kurmadan once lisans "
                  "kosullarini kabul etmeniz gerekir. Tespit edilen lisans: %1")
                  .arg(licenseLine.isEmpty() ? QStringLiteral("Bilinmiyor")
                                             : licenseLine));
    return;
  }

  setProprietaryAgreement(false, QString());
}

void NvidiaInstaller::install() { installProprietary(false); }

void NvidiaInstaller::installProprietary(bool agreementAccepted) {
  refreshProprietaryAgreement();

  if (m_proprietaryAgreementRequired && !agreementAccepted) {
    emit installFinished(
        false,
        QStringLiteral("Kurulumdan once lisans/sozlesme onayi gereklidir."));
    return;
  }

  CommandRunner runner;

  connect(&runner, &CommandRunner::outputLine, this,
          &NvidiaInstaller::progressMessage);

  emit progressMessage(QStringLiteral("RPM Fusion deposu kontrol ediliyor..."));

  CommandRunner rpmRunner;
  const auto fedoraResult = rpmRunner.run(
      QStringLiteral("rpm"), {QStringLiteral("-E"), QStringLiteral("%fedora")});

  const QString fedoraVersion = fedoraResult.stdout.trimmed();
  if (fedoraVersion.isEmpty()) {
    emit installFinished(false,
                         QStringLiteral("Fedora surumu tespit edilemedi."));
    return;
  }

  auto result = runner.runAsRoot(
      QStringLiteral("dnf"),
      {QStringLiteral("install"), QStringLiteral("-y"),
       QStringLiteral("https://mirrors.rpmfusion.org/free/fedora/"
                      "rpmfusion-free-release-%1.noarch.rpm")
           .arg(fedoraVersion),
       QStringLiteral("https://mirrors.rpmfusion.org/nonfree/fedora/"
                      "rpmfusion-nonfree-release-%1.noarch.rpm")
           .arg(fedoraVersion)});

  if (!result.success()) {
    emit installFinished(false, QStringLiteral("RPM Fusion repo eklenemedi: ") +
                                    result.stderr);
    return;
  }

  emit progressMessage(QStringLiteral(
      "Kapali kaynak NVIDIA surucusu kuruluyor (akmod-nvidia)..."));

  result = runner.runAsRoot(QStringLiteral("dnf"),
                            {QStringLiteral("install"), QStringLiteral("-y"),
                             QStringLiteral("akmod-nvidia")});

  if (!result.success()) {
    emit installFinished(false,
                         QStringLiteral("Kurulum basarisiz: ") + result.stderr);
    return;
  }

  emit progressMessage(
      QStringLiteral("Kernel modulu derleniyor (akmods --force)..."));
  runner.runAsRoot(QStringLiteral("akmods"), {QStringLiteral("--force")});

  const QString sessionType = detectSessionType();
  QString sessionError;
  if (!applySessionSpecificSetup(runner, sessionType, &sessionError)) {
    emit installFinished(false, sessionError);
    return;
  }

  emit installFinished(
      true, QStringLiteral("Kapali kaynak NVIDIA surucusu basariyla kuruldu. "
                           "Lutfen sistemi yeniden baslatin."));
}

void NvidiaInstaller::installOpenSource() {
  CommandRunner runner;

  connect(&runner, &CommandRunner::outputLine, this,
          &NvidiaInstaller::progressMessage);

  emit progressMessage(
      QStringLiteral("Acik kaynak surucuye gecis baslatiliyor..."));

  // Once kapali kaynak paketleri kaldir.
  auto result = runner.runAsRoot(
      QStringLiteral("dnf"),
      {QStringLiteral("remove"), QStringLiteral("-y"),
       QStringLiteral("akmod-nvidia"), QStringLiteral("xorg-x11-drv-nvidia*")});

  if (!result.success()) {
    emit installFinished(
        false, QStringLiteral("Kapali kaynak paket kaldirma basarisiz: ") +
                   result.stderr);
    return;
  }

  // Nouveau ve temel Mesa paketlerini garanti altina al.
  result = runner.runAsRoot(QStringLiteral("dnf"),
                            {QStringLiteral("install"), QStringLiteral("-y"),
                             QStringLiteral("xorg-x11-drv-nouveau"),
                             QStringLiteral("mesa-dri-drivers")});

  if (!result.success()) {
    emit installFinished(
        false, QStringLiteral("Acik kaynak surucu kurulumu basarisiz: ") +
                   result.stderr);
    return;
  }

  runner.runAsRoot(QStringLiteral("dracut"), {QStringLiteral("--force")});

  emit installFinished(true,
                       QStringLiteral("Acik kaynak surucu (Nouveau) kuruldu. "
                                      "Lutfen sistemi yeniden baslatin."));
}

void NvidiaInstaller::remove() {
  CommandRunner runner;
  connect(&runner, &CommandRunner::outputLine, this,
          &NvidiaInstaller::progressMessage);

  emit progressMessage(QStringLiteral("NVIDIA surucusu kaldiriliyor..."));

  const auto result = runner.runAsRoot(
      QStringLiteral("dnf"),
      {QStringLiteral("remove"), QStringLiteral("-y"),
       QStringLiteral("akmod-nvidia"), QStringLiteral("xorg-x11-drv-nvidia*")});

  emit removeFinished(result.success(),
                      result.success()
                          ? QStringLiteral("Surucu basariyla kaldirildi.")
                          : QStringLiteral("Kaldirma basarisiz: ") +
                                result.stderr);
}

void NvidiaInstaller::deepClean() {
  CommandRunner runner;
  connect(&runner, &CommandRunner::outputLine, this,
          &NvidiaInstaller::progressMessage);

  emit progressMessage(
      QStringLiteral("Eski surucu kalintilari temizleniyor..."));

  runner.runAsRoot(QStringLiteral("dnf"),
                   {QStringLiteral("remove"), QStringLiteral("-y"),
                    QStringLiteral("*nvidia*"), QStringLiteral("*akmod*")});

  runner.runAsRoot(QStringLiteral("dnf"),
                   {QStringLiteral("clean"), QStringLiteral("all")});

  emit progressMessage(QStringLiteral("Deep clean tamamlandi."));
}

QString NvidiaInstaller::detectSessionType() const {
  const QString envType =
      qEnvironmentVariable("XDG_SESSION_TYPE").trimmed().toLower();
  if (!envType.isEmpty())
    return envType;

  CommandRunner runner;
  const auto loginctl =
      runner.run(QStringLiteral("loginctl"),
                 {QStringLiteral("show-session"),
                  qEnvironmentVariable("XDG_SESSION_ID"), QStringLiteral("-p"),
                  QStringLiteral("Type"), QStringLiteral("--value")});

  if (loginctl.success()) {
    const QString type = loginctl.stdout.trimmed().toLower();
    if (!type.isEmpty())
      return type;
  }

  return QStringLiteral("unknown");
}

bool NvidiaInstaller::applySessionSpecificSetup(CommandRunner &runner,
                                                const QString &sessionType,
                                                QString *errorMessage) {
  if (sessionType == QStringLiteral("wayland")) {
    emit progressMessage(QStringLiteral(
        "Wayland tespit edildi: nvidia-drm.modeset=1 ayari uygulaniyor..."));

    const auto result =
        runner.runAsRoot(QStringLiteral("grubby"),
                         {QStringLiteral("--update-kernel=ALL"),
                          QStringLiteral("--args=nvidia-drm.modeset=1")});

    if (!result.success()) {
      if (errorMessage) {
        *errorMessage =
            QStringLiteral("Wayland icin kernel parametresi uygulanamadi: ") +
            result.stderr;
      }
      return false;
    }
    return true;
  }

  if (sessionType == QStringLiteral("x11")) {
    emit progressMessage(
        QStringLiteral("X11 tespit edildi: X11 NVIDIA userspace paketleri "
                       "kontrol ediliyor..."));

    const auto result = runner.runAsRoot(
        QStringLiteral("dnf"), {QStringLiteral("install"), QStringLiteral("-y"),
                                QStringLiteral("xorg-x11-drv-nvidia")});

    if (!result.success()) {
      if (errorMessage) {
        *errorMessage = QStringLiteral("X11 NVIDIA paketi kurulurken hata: ") +
                        result.stderr;
      }
      return false;
    }
  }

  return true;
}
