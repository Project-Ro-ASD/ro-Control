#include "installer.h"

#include "system/commandrunner.h"
#include "system/sessionutil.h"

#include <QMetaObject>
#include <QPointer>
#include <QThread>
#include <QtGlobal>

namespace {

const QStringList kSharedNvidiaUserspacePackages = {
    QStringLiteral("xorg-x11-drv-nvidia"),
    QStringLiteral("xorg-x11-drv-nvidia-libs"),
    QStringLiteral("xorg-x11-drv-nvidia-cuda"),
    QStringLiteral("xorg-x11-drv-nvidia-cuda-libs"),
    QStringLiteral("nvidia-modprobe"),
    QStringLiteral("nvidia-persistenced"),
    QStringLiteral("nvidia-settings"),
};

const QStringList kKernelPackageCleanupTargets = {
    QStringLiteral("akmod-nvidia"),
    QStringLiteral("akmod-nvidia-open"),
    QStringLiteral("xorg-x11-drv-nvidia-kmodsrc"),
};

const char kNvidiaLicenseUrl[] =
    "https://www.nvidia.com/en-us/drivers/nvidia-license/";

QString commandError(const CommandRunner::Result &result,
                     const QString &fallback = QString()) {
  const QString stderrText = result.stderr.trimmed();
  if (!stderrText.isEmpty()) {
    return stderrText;
  }

  const QString stdoutText = result.stdout.trimmed();
  if (!stdoutText.isEmpty()) {
    return stdoutText;
  }

  return fallback;
}

QStringList buildDriverInstallTargets(const QString &kernelPackageName) {
  QStringList packages{kernelPackageName};
  packages << kSharedNvidiaUserspacePackages;
  return packages;
}

void emitProgressAsync(const QPointer<NvidiaInstaller> &guard,
                       const QString &message) {
  QMetaObject::invokeMethod(
      guard,
      [guard, message]() {
        if (guard) {
          emit guard->progressMessage(message);
        }
      },
      Qt::QueuedConnection);
}

void attachRunnerLogging(CommandRunner &runner,
                         const QPointer<NvidiaInstaller> &guard) {
  QObject::connect(
      &runner, &CommandRunner::outputLine, guard,
      [guard](const QString &message) { emitProgressAsync(guard, message); });

  QObject::connect(
      &runner, &CommandRunner::errorLine, guard,
      [guard](const QString &message) { emitProgressAsync(guard, message); });

  QObject::connect(
      &runner, &CommandRunner::commandStarted, guard,
      [guard](const QString &program, const QStringList &args, int attempt) {
        QStringList visibleArgs = args;
        if (!visibleArgs.isEmpty() &&
            visibleArgs.constFirst().contains(
                QStringLiteral("ro-control-helper"))) {
          visibleArgs.removeFirst();
        }

        const QString commandLine = QStringLiteral("$ %1 %2").arg(
            program, visibleArgs.join(QLatin1Char(' ')).trimmed());
        emitProgressAsync(
            guard, NvidiaInstaller::tr("Starting command (attempt %1): %2")
                       .arg(attempt)
                       .arg(commandLine.trimmed()));
      });

  QObject::connect(
      &runner, &CommandRunner::commandFinished, guard,
      [guard](const QString &program, int exitCode, int attempt,
              int elapsedMs) {
        emitProgressAsync(
            guard, NvidiaInstaller::tr(
                       "Command finished (attempt %1, exit %2, %3 ms): %4")
                       .arg(attempt)
                       .arg(exitCode)
                       .arg(elapsedMs)
                       .arg(program));
      });
}

} // namespace

NvidiaInstaller::NvidiaInstaller(QObject *parent) : QObject(parent) {
  refreshProprietaryAgreement();
}

void NvidiaInstaller::setBusy(bool busy) {
  if (m_busy == busy) {
    return;
  }

  m_busy = busy;
  emit busyChanged();
}

void NvidiaInstaller::runAsyncTask(const std::function<void()> &task) {
  if (m_busy) {
    emit progressMessage(tr("Another driver operation is already running."));
    return;
  }

  setBusy(true);

  QThread *thread = QThread::create(task);
  connect(thread, &QThread::finished, this, [this, thread]() {
    setBusy(false);
    thread->deleteLater();
  });
  thread->start();
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
  setProprietaryAgreement(
      true,
      tr("The proprietary NVIDIA driver is subject to NVIDIA's software "
         "license. Review the official NVIDIA license before installation: %1")
          .arg(QString::fromLatin1(kNvidiaLicenseUrl)));
}

void NvidiaInstaller::install() { installProprietary(false); }

void NvidiaInstaller::installProprietary(bool agreementAccepted) {
  refreshProprietaryAgreement();

  if (m_proprietaryAgreementRequired && !agreementAccepted) {
    emit installFinished(
        false, tr("NVIDIA license review confirmation is required before "
                  "installation."));
    return;
  }

  QPointer<NvidiaInstaller> guard(this);
  runAsyncTask([guard]() {
    if (!guard) {
      return;
    }

    CommandRunner runner;
    attachRunnerLogging(runner, guard);

    emitProgressAsync(
        guard, NvidiaInstaller::tr("Checking RPM Fusion repositories..."));

    CommandRunner rpmRunner;
    const auto fedoraResult =
        rpmRunner.run(QStringLiteral("rpm"),
                      {QStringLiteral("-E"), QStringLiteral("%fedora")});

    const QString fedoraVersion = fedoraResult.stdout.trimmed();
    if (fedoraVersion.isEmpty()) {
      QMetaObject::invokeMethod(
          guard,
          [guard]() {
            if (guard) {
              emit guard->installFinished(
                  false, NvidiaInstaller::tr(
                             "Platform version could not be detected."));
            }
          },
          Qt::QueuedConnection);
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
      const QString error =
          NvidiaInstaller::tr("Failed to enable RPM Fusion repositories: ") +
          result.stderr.trimmed();
      QMetaObject::invokeMethod(
          guard,
          [guard, error]() {
            if (guard) {
              emit guard->installFinished(false, error);
            }
          },
          Qt::QueuedConnection);
      return;
    }

    emitProgressAsync(
        guard,
        NvidiaInstaller::tr(
            "Installing the proprietary NVIDIA driver (akmod-nvidia)..."));

    QStringList installArgs{QStringLiteral("install"), QStringLiteral("-y"),
                            QStringLiteral("--refresh"),
                            QStringLiteral("--best"),
                            QStringLiteral("--allowerasing")};
    installArgs << buildDriverInstallTargets(QStringLiteral("akmod-nvidia"));
    result = runner.runAsRoot(QStringLiteral("dnf"), installArgs);

    if (!result.success()) {
      const QString error = NvidiaInstaller::tr("Installation failed: ") +
                            result.stderr.trimmed();
      QMetaObject::invokeMethod(
          guard,
          [guard, error]() {
            if (guard) {
              emit guard->installFinished(false, error);
            }
          },
          Qt::QueuedConnection);
      return;
    }

    emitProgressAsync(
        guard,
        NvidiaInstaller::tr("Building the kernel module (akmods --force)..."));
    result =
        runner.runAsRoot(QStringLiteral("akmods"), {QStringLiteral("--force")});
    if (!result.success()) {
      const QString error =
          NvidiaInstaller::tr("Kernel module build failed: ") +
          (result.stderr.trimmed().isEmpty() ? result.stdout.trimmed()
                                             : result.stderr.trimmed());
      QMetaObject::invokeMethod(
          guard,
          [guard, error]() {
            if (guard) {
              emit guard->installFinished(false, error);
            }
          },
          Qt::QueuedConnection);
      return;
    }

    const QString sessionType = SessionUtil::detectSessionType();
    QString sessionError;
    if (!guard->applySessionSpecificSetup(runner, sessionType, &sessionError)) {
      QMetaObject::invokeMethod(
          guard,
          [guard, sessionError]() {
            if (guard) {
              emit guard->installFinished(false, sessionError);
            }
          },
          Qt::QueuedConnection);
      return;
    }

    QMetaObject::invokeMethod(
        guard,
        [guard]() {
          if (guard) {
            emit guard->installFinished(
                true, NvidiaInstaller::tr(
                          "The proprietary NVIDIA driver was installed "
                          "successfully. Please restart the system."));
          }
        },
        Qt::QueuedConnection);
  });
}

void NvidiaInstaller::installOpenSource() {
  QPointer<NvidiaInstaller> guard(this);
  runAsyncTask([guard]() {
    if (!guard) {
      return;
    }

    CommandRunner runner;
    attachRunnerLogging(runner, guard);

    emitProgressAsync(guard, NvidiaInstaller::tr(
                                 "Switching to NVIDIA open kernel modules..."));

    auto result = runner.runAsRoot(
        QStringLiteral("dnf"),
        QStringList{QStringLiteral("remove"), QStringLiteral("-y")} +
            kKernelPackageCleanupTargets);

    if (!result.success()) {
      const QString error =
          NvidiaInstaller::tr(
              "Failed to remove conflicting NVIDIA kernel packages: ") +
          commandError(result);
      QMetaObject::invokeMethod(
          guard,
          [guard, error]() {
            if (guard) {
              emit guard->installFinished(false, error);
            }
          },
          Qt::QueuedConnection);
      return;
    }

    QStringList installArgs{QStringLiteral("install"), QStringLiteral("-y"),
                            QStringLiteral("--refresh"),
                            QStringLiteral("--best"),
                            QStringLiteral("--allowerasing")};
    installArgs << buildDriverInstallTargets(
        QStringLiteral("akmod-nvidia-open"));
    result = runner.runAsRoot(QStringLiteral("dnf"), installArgs);

    if (!result.success()) {
      const QString error =
          NvidiaInstaller::tr(
              "Open NVIDIA kernel module installation failed: ") +
          commandError(result);
      QMetaObject::invokeMethod(
          guard,
          [guard, error]() {
            if (guard) {
              emit guard->installFinished(false, error);
            }
          },
          Qt::QueuedConnection);
      return;
    }

    emitProgressAsync(
        guard,
        NvidiaInstaller::tr("Building the kernel module (akmods --force)..."));
    result =
        runner.runAsRoot(QStringLiteral("akmods"), {QStringLiteral("--force")});
    if (!result.success()) {
      const QString error =
          NvidiaInstaller::tr("Kernel module build failed: ") +
          commandError(result, NvidiaInstaller::tr("unknown error"));
      QMetaObject::invokeMethod(
          guard,
          [guard, error]() {
            if (guard) {
              emit guard->installFinished(false, error);
            }
          },
          Qt::QueuedConnection);
      return;
    }

    QString sessionError;
    const QString sessionType = SessionUtil::detectSessionType();
    if (!guard->applySessionSpecificSetup(runner, sessionType, &sessionError)) {
      QMetaObject::invokeMethod(
          guard,
          [guard, sessionError]() {
            if (guard) {
              emit guard->installFinished(false, sessionError);
            }
          },
          Qt::QueuedConnection);
      return;
    }

    runner.runAsRoot(QStringLiteral("dracut"), {QStringLiteral("--force")});

    QMetaObject::invokeMethod(
        guard,
        [guard]() {
          if (guard) {
            emit guard->installFinished(
                true, NvidiaInstaller::tr(
                          "NVIDIA open kernel modules were installed "
                          "successfully. Please restart the system."));
          }
        },
        Qt::QueuedConnection);
  });
}

void NvidiaInstaller::remove() {
  QPointer<NvidiaInstaller> guard(this);
  runAsyncTask([guard]() {
    if (!guard) {
      return;
    }

    CommandRunner runner;
    attachRunnerLogging(runner, guard);

    emitProgressAsync(guard,
                      NvidiaInstaller::tr("Removing the NVIDIA driver..."));

    const auto result = runner.runAsRoot(
        QStringLiteral("dnf"),
        {QStringLiteral("remove"), QStringLiteral("-y"),
         QStringLiteral("akmod-nvidia"), QStringLiteral("akmod-nvidia-open"),
         QStringLiteral("xorg-x11-drv-nvidia*")});

    const bool success = result.success();
    const QString message =
        success
            ? NvidiaInstaller::tr("Driver removed successfully.")
            : NvidiaInstaller::tr("Removal failed: ") + result.stderr.trimmed();
    QMetaObject::invokeMethod(
        guard,
        [guard, success, message]() {
          if (guard) {
            emit guard->removeFinished(success, message);
          }
        },
        Qt::QueuedConnection);
  });
}

void NvidiaInstaller::deepClean() {
  QPointer<NvidiaInstaller> guard(this);
  runAsyncTask([guard]() {
    if (!guard) {
      return;
    }

    CommandRunner runner;
    attachRunnerLogging(runner, guard);

    emitProgressAsync(
        guard, NvidiaInstaller::tr("Cleaning legacy driver leftovers..."));

    const auto removeResult =
        runner.runAsRoot(QStringLiteral("dnf"),
                         {QStringLiteral("remove"), QStringLiteral("-y"),
                          QStringLiteral("*nvidia*"), QStringLiteral("*akmod*"),
                          QStringLiteral("*nvidia-open*")});

    if (!removeResult.success()) {
      const QString error = NvidiaInstaller::tr("Deep clean failed: ") +
                            removeResult.stderr.trimmed();
      QMetaObject::invokeMethod(
          guard,
          [guard, error]() {
            if (guard) {
              emit guard->removeFinished(false, error);
            }
          },
          Qt::QueuedConnection);
      return;
    }

    const auto cleanResult =
        runner.runAsRoot(QStringLiteral("dnf"),
                         {QStringLiteral("clean"), QStringLiteral("all")});
    if (!cleanResult.success()) {
      const QString error = NvidiaInstaller::tr("DNF cache cleanup failed: ") +
                            cleanResult.stderr.trimmed();
      QMetaObject::invokeMethod(
          guard,
          [guard, error]() {
            if (guard) {
              emit guard->removeFinished(false, error);
            }
          },
          Qt::QueuedConnection);
      return;
    }

    QMetaObject::invokeMethod(
        guard,
        [guard]() {
          if (guard) {
            emit guard->progressMessage(
                NvidiaInstaller::tr("Deep clean completed."));
            emit guard->removeFinished(
                true, NvidiaInstaller::tr("Legacy NVIDIA cleanup completed."));
          }
        },
        Qt::QueuedConnection);
  });
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
