#include "installer.h"

#include "system/commandrunner.h"

#include <QMetaObject>
#include <QPointer>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QThread>
#include <QtGlobal>

namespace {

const QStringList kManagedNvidiaPackages = {
    QStringLiteral("akmod-nvidia"),
    QStringLiteral("xorg-x11-drv-nvidia"),
    QStringLiteral("xorg-x11-drv-nvidia-libs"),
    QStringLiteral("xorg-x11-drv-nvidia-cuda"),
    QStringLiteral("xorg-x11-drv-nvidia-cuda-libs"),
    QStringLiteral("nvidia-modprobe"),
    QStringLiteral("nvidia-persistenced"),
    QStringLiteral("nvidia-settings"),
};

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

bool hasExecutable(const QString &program) {
  return !QStandardPaths::findExecutable(program).isEmpty();
}

QStringList buildLatestDriverTargets(const QString &sessionType) {
  QStringList targets = kManagedNvidiaPackages;
  if (sessionType != QStringLiteral("x11")) {
    targets.removeAll(QStringLiteral("xorg-x11-drv-nvidia"));
  }

  return targets;
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
    emit progressMessage(
        QStringLiteral("Baska bir surucu islemi zaten calisiyor."));
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
  if (!hasExecutable(QStringLiteral("dnf"))) {
    setProprietaryAgreement(false, QString());
    return;
  }

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

  QPointer<NvidiaInstaller> guard(this);
  runAsyncTask([guard]() {
    if (!guard) {
      return;
    }

    CommandRunner runner;
    QObject::connect(&runner, &CommandRunner::outputLine, guard,
                     [guard](const QString &message) {
                       if (!guard) {
                         return;
                       }
                       QMetaObject::invokeMethod(
                           guard,
                           [guard, message]() {
                             if (guard) {
                               emit guard->progressMessage(message);
                             }
                           },
                           Qt::QueuedConnection);
                     });

    QMetaObject::invokeMethod(
        guard,
        [guard]() {
          if (guard) {
            emit guard->progressMessage(
                QStringLiteral("RPM Fusion deposu kontrol ediliyor..."));
          }
        },
        Qt::QueuedConnection);

    CommandRunner rpmRunner;
    if (!hasExecutable(QStringLiteral("dnf")) ||
        !hasExecutable(QStringLiteral("rpm"))) {
      QMetaObject::invokeMethod(
          guard,
          [guard]() {
            if (guard) {
              emit guard->installFinished(
                  false,
                  QStringLiteral(
                      "Kurulum icin gerekli sistem araclari eksik (dnf/rpm)."));
            }
          },
          Qt::QueuedConnection);
      return;
    }

    const auto fedoraResult =
        rpmRunner.run(QStringLiteral("rpm"),
                      {QStringLiteral("-E"), QStringLiteral("%fedora")});
    const QString fedoraVersion = fedoraResult.stdout.trimmed();
    static const QRegularExpression fedoraVersionPattern(
        QStringLiteral("^\\d+$"));
    if (!fedoraVersionPattern.match(fedoraVersion).hasMatch()) {
      QMetaObject::invokeMethod(
          guard,
          [guard]() {
            if (guard) {
              emit guard->installFinished(
                  false, QStringLiteral("Fedora surumu tespit edilemedi."));
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
          QStringLiteral("RPM Fusion repo eklenemedi: ") +
          commandError(result, QStringLiteral("bilinmeyen hata"));
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

    QMetaObject::invokeMethod(
        guard,
        [guard]() {
          if (guard) {
            emit guard->progressMessage(QStringLiteral(
                "Kapali kaynak NVIDIA surucusu kuruluyor (akmod-nvidia)..."));
          }
        },
        Qt::QueuedConnection);

    const QString sessionType = guard->detectSessionType();
    result = runner.runAsRoot(
        QStringLiteral("dnf"),
        QStringList{QStringLiteral("install"), QStringLiteral("-y"),
                    QStringLiteral("--allowerasing")} +
            buildLatestDriverTargets(sessionType));
    if (!result.success()) {
      const QString error =
          QStringLiteral("Kurulum basarisiz: ") +
          commandError(result, QStringLiteral("bilinmeyen hata"));
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

    QMetaObject::invokeMethod(
        guard,
        [guard]() {
          if (guard) {
            emit guard->progressMessage(
                QStringLiteral("Kernel modulu derleniyor (akmods --force)..."));
          }
        },
        Qt::QueuedConnection);

    result =
        runner.runAsRoot(QStringLiteral("akmods"), {QStringLiteral("--force")});
    if (!result.success()) {
      const QString error =
          QStringLiteral("Kernel modulu derlenemedi: ") +
          commandError(result, QStringLiteral("bilinmeyen hata"));
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
                true, QStringLiteral(
                          "Kapali kaynak NVIDIA surucusu basariyla kuruldu. "
                          "Lutfen sistemi yeniden baslatin."));
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
    QObject::connect(&runner, &CommandRunner::outputLine, guard,
                     [guard](const QString &message) {
                       if (!guard) {
                         return;
                       }
                       QMetaObject::invokeMethod(
                           guard,
                           [guard, message]() {
                             if (guard) {
                               emit guard->progressMessage(message);
                             }
                           },
                           Qt::QueuedConnection);
                     });

    QMetaObject::invokeMethod(
        guard,
        [guard]() {
          if (guard) {
            emit guard->progressMessage(
                QStringLiteral("Acik kaynak surucuye gecis baslatiliyor..."));
          }
        },
        Qt::QueuedConnection);

    auto result = runner.runAsRoot(
        QStringLiteral("dnf"),
        QStringList{QStringLiteral("remove"), QStringLiteral("-y")} +
            kManagedNvidiaPackages);
    if (!result.success()) {
      const QString error =
          QStringLiteral("Kapali kaynak paket kaldirma basarisiz: ") +
          commandError(result, QStringLiteral("bilinmeyen hata"));
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

    result = runner.runAsRoot(QStringLiteral("dnf"),
                              {QStringLiteral("install"), QStringLiteral("-y"),
                               QStringLiteral("xorg-x11-drv-nouveau"),
                               QStringLiteral("mesa-dri-drivers")});
    if (!result.success()) {
      const QString error =
          QStringLiteral("Acik kaynak surucu kurulumu basarisiz: ") +
          commandError(result, QStringLiteral("bilinmeyen hata"));
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

    result =
        runner.runAsRoot(QStringLiteral("dracut"), {QStringLiteral("--force")});
    if (!result.success()) {
      const QString error =
          QStringLiteral("Initramfs guncellenemedi: ") +
          commandError(result, QStringLiteral("bilinmeyen hata"));
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

    QMetaObject::invokeMethod(
        guard,
        [guard]() {
          if (guard) {
            emit guard->installFinished(
                true,
                QStringLiteral("Acik kaynak surucu (Nouveau) kuruldu. Lutfen "
                               "sistemi yeniden baslatin."));
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
    QObject::connect(&runner, &CommandRunner::outputLine, guard,
                     [guard](const QString &message) {
                       if (!guard) {
                         return;
                       }
                       QMetaObject::invokeMethod(
                           guard,
                           [guard, message]() {
                             if (guard) {
                               emit guard->progressMessage(message);
                             }
                           },
                           Qt::QueuedConnection);
                     });

    QMetaObject::invokeMethod(
        guard,
        [guard]() {
          if (guard) {
            emit guard->progressMessage(
                QStringLiteral("NVIDIA surucusu kaldiriliyor..."));
          }
        },
        Qt::QueuedConnection);

    const auto result = runner.runAsRoot(
        QStringLiteral("dnf"),
        QStringList{QStringLiteral("remove"), QStringLiteral("-y")} +
            kManagedNvidiaPackages);
    const bool success = result.success();
    const QString message =
        success ? QStringLiteral("Surucu basariyla kaldirildi.")
                : QStringLiteral("Kaldirma basarisiz: ") +
                      commandError(result, QStringLiteral("bilinmeyen hata"));
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
    QObject::connect(&runner, &CommandRunner::outputLine, guard,
                     [guard](const QString &message) {
                       if (!guard) {
                         return;
                       }
                       QMetaObject::invokeMethod(
                           guard,
                           [guard, message]() {
                             if (guard) {
                               emit guard->progressMessage(message);
                             }
                           },
                           Qt::QueuedConnection);
                     });

    QMetaObject::invokeMethod(
        guard,
        [guard]() {
          if (guard) {
            emit guard->progressMessage(
                QStringLiteral("Eski surucu kalintilari temizleniyor..."));
          }
        },
        Qt::QueuedConnection);

    const auto removeResult = runner.runAsRoot(
        QStringLiteral("dnf"),
        QStringList{QStringLiteral("remove"), QStringLiteral("-y")} +
            kManagedNvidiaPackages);
    if (!removeResult.success()) {
      const QString error =
          QStringLiteral("Deep clean kaldirma adimi hata verdi: ") +
          commandError(removeResult, QStringLiteral("bilinmeyen hata"));
      QMetaObject::invokeMethod(
          guard,
          [guard, error]() {
            if (guard) {
              emit guard->progressMessage(error);
            }
          },
          Qt::QueuedConnection);
    }

    const auto cleanResult =
        runner.runAsRoot(QStringLiteral("dnf"),
                         {QStringLiteral("clean"), QStringLiteral("all")});
    if (!cleanResult.success()) {
      const QString error =
          QStringLiteral("DNF cache temizligi hata verdi: ") +
          commandError(cleanResult, QStringLiteral("bilinmeyen hata"));
      QMetaObject::invokeMethod(
          guard,
          [guard, error]() {
            if (guard) {
              emit guard->progressMessage(error);
            }
          },
          Qt::QueuedConnection);
    }

    QMetaObject::invokeMethod(
        guard,
        [guard]() {
          if (guard) {
            emit guard->progressMessage(
                QStringLiteral("Deep clean tamamlandi."));
          }
        },
        Qt::QueuedConnection);
  });
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
            commandError(result, QStringLiteral("bilinmeyen hata"));
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
                        commandError(result, QStringLiteral("bilinmeyen hata"));
      }
      return false;
    }
  }

  return true;
}
