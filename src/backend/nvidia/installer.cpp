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
        true, tr("You must accept the NVIDIA proprietary driver license terms "
                 "before installation. Detected license: %1")
                  .arg(licenseLine.isEmpty() ? tr("Unknown") : licenseLine));
    return;
  }

  setProprietaryAgreement(false, QString());
}

void NvidiaInstaller::install() { installProprietary(false); }

void NvidiaInstaller::installProprietary(bool agreementAccepted) {
  refreshProprietaryAgreement();

  if (m_proprietaryAgreementRequired && !agreementAccepted) {
    emit installFinished(false,
                         tr("License agreement acceptance is required before "
                            "installation."));
    return;
  }

  CommandRunner runner;

  connect(&runner, &CommandRunner::outputLine, this,
          &NvidiaInstaller::progressMessage);

  emit progressMessage(tr("Checking RPM Fusion repositories..."));

  CommandRunner rpmRunner;
  const auto fedoraResult = rpmRunner.run(
      QStringLiteral("rpm"), {QStringLiteral("-E"), QStringLiteral("%fedora")});

  const QString fedoraVersion = fedoraResult.stdout.trimmed();
  if (fedoraVersion.isEmpty()) {
    emit installFinished(false, tr("Platform version could not be detected."));
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
    emit installFinished(false,
                         tr("Failed to enable RPM Fusion repositories: ") +
                             result.stderr);
    return;
  }

  emit progressMessage(
      tr("Installing the proprietary NVIDIA driver (akmod-nvidia)..."));

  result = runner.runAsRoot(QStringLiteral("dnf"),
                            {QStringLiteral("install"), QStringLiteral("-y"),
                             QStringLiteral("akmod-nvidia")});

  if (!result.success()) {
    emit installFinished(false, tr("Installation failed: ") + result.stderr);
    return;
  }

  emit progressMessage(tr("Building the kernel module (akmods --force)..."));
  runner.runAsRoot(QStringLiteral("akmods"), {QStringLiteral("--force")});

  const QString sessionType = detectSessionType();
  QString sessionError;
  if (!applySessionSpecificSetup(runner, sessionType, &sessionError)) {
    emit installFinished(false, sessionError);
    return;
  }

  emit installFinished(
      true,
      tr("The proprietary NVIDIA driver was installed successfully. Please "
         "restart the system."));
}

void NvidiaInstaller::installOpenSource() {
  CommandRunner runner;

  connect(&runner, &CommandRunner::outputLine, this,
          &NvidiaInstaller::progressMessage);

  emit progressMessage(tr("Switching to the open-source driver..."));

  // Once kapali kaynak paketleri kaldir.
  auto result = runner.runAsRoot(
      QStringLiteral("dnf"),
      {QStringLiteral("remove"), QStringLiteral("-y"),
       QStringLiteral("akmod-nvidia"), QStringLiteral("xorg-x11-drv-nvidia*")});

  if (!result.success()) {
    emit installFinished(false, tr("Failed to remove proprietary packages: ") +
                                    result.stderr);
    return;
  }

  // Nouveau ve temel Mesa paketlerini garanti altina al.
  result = runner.runAsRoot(QStringLiteral("dnf"),
                            {QStringLiteral("install"), QStringLiteral("-y"),
                             QStringLiteral("xorg-x11-drv-nouveau"),
                             QStringLiteral("mesa-dri-drivers")});

  if (!result.success()) {
    emit installFinished(false, tr("Open-source driver installation failed: ") +
                                    result.stderr);
    return;
  }

  runner.runAsRoot(QStringLiteral("dracut"), {QStringLiteral("--force")});

  emit installFinished(true,
                       tr("The open-source driver (Nouveau) was installed. "
                          "Please restart the system."));
}

void NvidiaInstaller::remove() {
  CommandRunner runner;
  connect(&runner, &CommandRunner::outputLine, this,
          &NvidiaInstaller::progressMessage);

  emit progressMessage(tr("Removing the NVIDIA driver..."));

  const auto result = runner.runAsRoot(
      QStringLiteral("dnf"),
      {QStringLiteral("remove"), QStringLiteral("-y"),
       QStringLiteral("akmod-nvidia"), QStringLiteral("xorg-x11-drv-nvidia*")});

  emit removeFinished(result.success(),
                      result.success()
                          ? tr("Driver removed successfully.")
                          : tr("Removal failed: ") + result.stderr);
}

void NvidiaInstaller::deepClean() {
  CommandRunner runner;
  connect(&runner, &CommandRunner::outputLine, this,
          &NvidiaInstaller::progressMessage);

  emit progressMessage(tr("Cleaning legacy driver leftovers..."));

  runner.runAsRoot(QStringLiteral("dnf"),
                   {QStringLiteral("remove"), QStringLiteral("-y"),
                    QStringLiteral("*nvidia*"), QStringLiteral("*akmod*")});

  runner.runAsRoot(QStringLiteral("dnf"),
                   {QStringLiteral("clean"), QStringLiteral("all")});

  emit progressMessage(tr("Deep clean completed."));
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
    emit progressMessage(
        QStringLiteral("Wayland detected: applying nvidia-drm.modeset=1..."));

    const auto result =
        runner.runAsRoot(QStringLiteral("grubby"),
                         {QStringLiteral("--update-kernel=ALL"),
                          QStringLiteral("--args=nvidia-drm.modeset=1")});

    if (!result.success()) {
      if (errorMessage) {
        *errorMessage = tr("Failed to apply the Wayland kernel parameter: ") +
                        result.stderr;
      }
      return false;
    }
    return true;
  }

  if (sessionType == QStringLiteral("x11")) {
    emit progressMessage(
        tr("X11 detected: checking NVIDIA userspace packages..."));

    const auto result = runner.runAsRoot(
        QStringLiteral("dnf"), {QStringLiteral("install"), QStringLiteral("-y"),
                                QStringLiteral("xorg-x11-drv-nvidia")});

    if (!result.success()) {
      if (errorMessage) {
        *errorMessage =
            tr("Failed to install the X11 NVIDIA package: ") + result.stderr;
      }
      return false;
    }
  }

  return true;
}
