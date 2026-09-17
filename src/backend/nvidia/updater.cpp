#include "updater.h"
#include "asyncrunner.h"
#include "detector.h"
#include "system/capabilityprobe.h"
#include "system/commandrunner.h"
#include "system/sessionutil.h"
#include "versionparser.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QThread>
#include <QVersionNumber>
#include <QtGlobal>

namespace {

QStringList commonVersionLockedDriverPackages() { return {}; }

QStringList floatingDriverPackages() {
  return {
      QStringLiteral("nvidia-modprobe"),
      QStringLiteral("nvidia-persistenced"),
      QStringLiteral("nvidia-settings"),
  };
}

QStringList nvidiaKernelModules() {
  return {
      QStringLiteral("nvidia"),
      QStringLiteral("nvidia_modeset"),
      QStringLiteral("nvidia_uvm"),
      QStringLiteral("nvidia_drm"),
  };
}

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

bool commandCanceled(const CommandRunner::Result &result) {
  return result.exitCode == -3;
}

QString missingNvidiaHardwareMessage() {
  NvidiaDetector detector;
  if (detector.hasNvidiaGpu() || detector.isDriverInstalled()) {
    return {};
  }

  return NvidiaUpdater::tr(
      "No NVIDIA GPU or installed NVIDIA driver was detected. In a virtual "
      "machine, attach or passthrough an NVIDIA GPU before starting driver "
      "updates.");
}

QString normalizedTransactionOutput(const CommandRunner::Result &result) {
  return (result.stdout + QLatin1Char('\n') + result.stderr).toLower();
}

QString quotedList(const QStringList &values) {
  QStringList quoted;
  quoted.reserve(values.size());
  for (const QString &value : values) {
    quoted << QStringLiteral("`%1`").arg(value);
  }
  return quoted.join(QStringLiteral(", "));
}

QList<CommandRunner::RootCommand>
buildSessionSpecificRootCommands(const QString &sessionType) {
  Q_UNUSED(sessionType);
  QList<CommandRunner::RootCommand> commands;
  commands.append({QStringLiteral("dracut"),
                   {QStringLiteral("--force"), QStringLiteral("--add-drivers"),
                    nvidiaKernelModules().join(QLatin1Char(' '))}});

  commands.append({QStringLiteral("env"),
                   {QStringLiteral("LANG=C"), QStringLiteral("dnf"),
                    QStringLiteral("install"), QStringLiteral("-y"),
                    QStringLiteral("egl-wayland")}});
  commands.append({QStringLiteral("grubby"),
                   {QStringLiteral("--update-kernel=ALL"),
                    QStringLiteral("--args=nvidia-drm.modeset=1 "
                                   "nvidia-drm.fbdev=1")}});

  return commands;
}

struct UpdateStatusSnapshot {
  QString currentVersion;
  QString latestVersion;
  QString latestPackageVersion;
  QStringList availableVersions;
  bool remoteCatalogAvailable = false;
  bool updateAvailable = false;
  QString message;
};

QString firstNonEmptyLine(const QString &text) {
  const QStringList lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
  for (const QString &line : lines) {
    const QString trimmedLine = line.trimmed();
    if (!trimmedLine.isEmpty()) {
      return trimmedLine;
    }
  }

  return {};
}

QString detectInstalledKernelPackageName() {
  if (!CapabilityProbe::isToolAvailable(QStringLiteral("rpm"))) {
    return QStringLiteral("akmod-nvidia");
  }

  CommandRunner runner;
  const auto openResult =
      runner.run(QStringLiteral("rpm"),
                 {QStringLiteral("-q"), QStringLiteral("akmod-nvidia-open")});
  if (openResult.success()) {
    return QStringLiteral("akmod-nvidia-open");
  }

  return QStringLiteral("akmod-nvidia");
}

QString queryLatestRemotePackageVersion(CommandRunner &runner,
                                        const QString &kernelPackageName) {
  const auto result = runner.run(
      QStringLiteral("dnf"),
      {QStringLiteral("--refresh"), QStringLiteral("repoquery"),
       QStringLiteral("--latest-limit"), QStringLiteral("1"),
       QStringLiteral("--qf"), QStringLiteral("%{epoch}:%{version}-%{release}"),
       kernelPackageName});

  if (!result.success()) {
    return {};
  }

  return firstNonEmptyLine(result.stdout);
}

QString fetchTextFromUrl(CommandRunner &runner, const QString &url) {
  CommandRunner::RunOptions options;
  options.timeoutMs = 10000;

  const QString userAgent = QStringLiteral("ro-Control/%1")
                                .arg(QCoreApplication::applicationVersion());

  if (CapabilityProbe::isToolAvailable(QStringLiteral("curl"))) {
    const auto result =
        runner.run(QStringLiteral("curl"),
                   {QStringLiteral("-fsSL"), QStringLiteral("--compressed"),
                    QStringLiteral("-A"), userAgent, url},
                   options);
    if (result.success()) {
      return result.stdout;
    }
  }

  if (CapabilityProbe::isToolAvailable(QStringLiteral("wget"))) {
    const auto result = runner.run(
        QStringLiteral("wget"),
        {QStringLiteral("-qO-"), QStringLiteral("-U"), userAgent, url},
        options);
    if (result.success()) {
      return result.stdout;
    }
  }

  return {};
}

// Each call re-downloads and re-parses the NVIDIA page. Consider adding a
// short-lived cache (e.g. 5-minute TTL) if this function is called
// frequently.
QStringList queryOfficialDriverVersions(CommandRunner &runner) {
  const QString pageText = fetchTextFromUrl(
      runner, QStringLiteral("https://www.nvidia.com/en-us/drivers/unix/"));
  if (pageText.isEmpty()) {
    return {};
  }

  return NvidiaVersionParser::parseOfficialUnixDriverVersions(
      pageText, CapabilityProbe::normalizedCpuArchitecture());
}

bool isOfficialUpdateAvailable(const QString &currentVersion,
                               const QString &latestVersion) {
  const QString normalizedCurrent =
      NvidiaVersionParser::normalizedDriverVersion(currentVersion);
  const QString normalizedLatest =
      NvidiaVersionParser::normalizedDriverVersion(latestVersion);
  if (normalizedCurrent.isEmpty() || normalizedLatest.isEmpty()) {
    return false;
  }

  return QVersionNumber::compare(QVersionNumber::fromString(normalizedCurrent),
                                 QVersionNumber::fromString(normalizedLatest)) <
         0;
}

UpdateStatusSnapshot collectUpdateStatus() {
  UpdateStatusSnapshot snapshot;
  NvidiaDetector detector;
  const QString kernelPackageName = detectInstalledKernelPackageName();
  snapshot.currentVersion = detector.installedDriverVersion();

  const QString architectureSupportMessage =
      CapabilityProbe::roAsdNvidiaDriverFlowSupportMessage();
  if (!architectureSupportMessage.isEmpty()) {
    snapshot.message = architectureSupportMessage;
    return snapshot;
  }

  CommandRunner runner;
  const QStringList officialVersions = queryOfficialDriverVersions(runner);
  if (!officialVersions.isEmpty()) {
    snapshot.latestVersion = officialVersions.constFirst();
    snapshot.remoteCatalogAvailable = true;
  }

  const bool hasDnf =
      !QStandardPaths::findExecutable(QStringLiteral("dnf")).isEmpty();
  if (hasDnf) {
    const auto listResult =
        runner.run(QStringLiteral("dnf"),
                   {QStringLiteral("--refresh"), QStringLiteral("list"),
                    QStringLiteral("--showduplicates"), kernelPackageName});

    if (listResult.success()) {
      snapshot.availableVersions =
          NvidiaVersionParser::parseAvailablePackageVersions(listResult.stdout,
                                                             kernelPackageName);
      snapshot.remoteCatalogAvailable = snapshot.remoteCatalogAvailable ||
                                        !snapshot.availableVersions.isEmpty();
    }

    snapshot.latestPackageVersion =
        queryLatestRemotePackageVersion(runner, kernelPackageName);
    if (snapshot.latestPackageVersion.isEmpty() &&
        !snapshot.availableVersions.isEmpty()) {
      snapshot.latestPackageVersion = snapshot.availableVersions.constLast();
    }
  }

  if (snapshot.latestVersion.isEmpty()) {
    snapshot.latestVersion = NvidiaVersionParser::normalizedDriverVersion(
        snapshot.latestPackageVersion);
  }

  if (snapshot.currentVersion.isEmpty()) {
    if (snapshot.remoteCatalogAvailable) {
      snapshot.updateAvailable = true;
      snapshot.message =
          snapshot.latestVersion.isEmpty()
              ? NvidiaUpdater::tr(
                    "Official NVIDIA driver sources are reachable. "
                    "You can install the driver now.")
              : NvidiaUpdater::tr("Latest official NVIDIA driver version: %1")
                    .arg(snapshot.latestVersion);
    } else {
      snapshot.message = hasDnf
                             ? NvidiaUpdater::tr("No official NVIDIA driver "
                                                 "version could be retrieved.")
                             : NvidiaUpdater::tr("dnf not found.");
    }
    return snapshot;
  }

  if (!snapshot.latestVersion.isEmpty()) {
    snapshot.updateAvailable = isOfficialUpdateAvailable(
        snapshot.currentVersion, snapshot.latestVersion);
    snapshot.message =
        snapshot.updateAvailable
            ? NvidiaUpdater::tr("Official NVIDIA update found: %1")
                  .arg(snapshot.latestVersion)
            : NvidiaUpdater::tr("Driver matches the latest official NVIDIA "
                                "production branch.");
    return snapshot;
  }

  if (!hasDnf) {
    snapshot.message = NvidiaUpdater::tr("dnf not found.");
    return snapshot;
  }

  const auto checkResult =
      runner.run(QStringLiteral("dnf"),
                 {QStringLiteral("check-update"), kernelPackageName});

  if (checkResult.exitCode == 100) {
    const QString checkUpdateVersion =
        NvidiaVersionParser::normalizedDriverVersion(
            NvidiaVersionParser::parseCheckUpdateVersion(checkResult.stdout,
                                                         kernelPackageName));
    if (!checkUpdateVersion.isEmpty()) {
      snapshot.latestVersion = checkUpdateVersion;
    }
    snapshot.updateAvailable = true;
    snapshot.message =
        snapshot.latestVersion.isEmpty()
            ? NvidiaUpdater::tr("Update found (version details unavailable).")
            : NvidiaUpdater::tr("Update found: %1").arg(snapshot.latestVersion);
  } else if (checkResult.exitCode == 0) {
    snapshot.message =
        NvidiaUpdater::tr("Driver is up to date. No new version found.");
  } else {
    snapshot.message = NvidiaUpdater::tr("Update check failed: %1")
                           .arg(checkResult.stderr.trimmed().isEmpty()
                                    ? checkResult.stdout.trimmed()
                                    : checkResult.stderr.trimmed());
  }

  return snapshot;
}

} // namespace

NvidiaUpdater::NvidiaUpdater(QObject *parent) : QObject(parent) {
  m_cancelRequested = std::make_shared<std::atomic_bool>(false);
}

void NvidiaUpdater::setBusy(bool busy) {
  if (m_busy == busy) {
    return;
  }

  m_busy = busy;
  emit busyChanged();
}

void NvidiaUpdater::runAsyncTask(const std::function<void()> &task) {
  if (m_busy) {
    emit progressMessage(tr("Another driver operation is already running."));
    return;
  }

  m_cancelRequested->store(false, std::memory_order_relaxed);
  setBusy(true);

  QThread *thread = QThread::create(task);
  connect(thread, &QThread::finished, this, [this, thread]() {
    setBusy(false);
    thread->deleteLater();
  });
  thread->start();
}

void NvidiaUpdater::cancelOperation() {
  if (!m_busy) {
    emit progressMessage(tr("No driver operation is running."));
    return;
  }

  m_cancelRequested->store(true, std::memory_order_relaxed);
  emit progressMessage(
      tr("Cancel requested. Waiting for the active command to stop safely..."));
}

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

bool NvidiaUpdater::transactionChanged(
    const CommandRunner::Result &result) const {
  const QString output = normalizedTransactionOutput(result);

  if (output.contains(QStringLiteral("nothing to do")) ||
      output.contains(QStringLiteral("nothing to do.")) ||
      output.contains(QStringLiteral("no packages marked for upgrade")) ||
      output.contains(QStringLiteral("no packages marked for update")) ||
      output.contains(QStringLiteral("no packages marked for reinstall")) ||
      output.contains(QStringLiteral("package is already installed"))) {
    return false;
  }

  return true;
}

SessionUtil::SessionInfo NvidiaUpdater::detectSessionInfo() const {
  return SessionUtil::detectSessionInfo();
}

QString NvidiaUpdater::detectInstalledKernelPackageName() const {
  return ::detectInstalledKernelPackageName();
}

QStringList
NvidiaUpdater::buildDriverTargets(const QString &version,
                                  const QString &sessionType,
                                  const QString &kernelPackageName) const {
  Q_UNUSED(sessionType);
  QStringList versionLockedPackages;
  if (!kernelPackageName.isEmpty()) {
    versionLockedPackages.append(kernelPackageName);
  }
  const QStringList locked = commonVersionLockedDriverPackages();
  for (const QString &pkg : locked) {
    versionLockedPackages.append(pkg);
  }

  QStringList targets = NvidiaVersionParser::buildVersionedPackageSpecs(
      versionLockedPackages, version);
  const QStringList floating = floatingDriverPackages();
  for (const QString &pkg : floating) {
    targets.append(pkg);
  }
  return targets;
}

QStringList NvidiaUpdater::buildTransactionArguments(
    const QString &requestedVersion, const QString &installedVersion,
    const QString &sessionType, const QString &kernelPackageName) const {
  const QString normalizedRequestedVersion = requestedVersion.trimmed();
  const QString normalizedInstalledVersion = installedVersion.trimmed();
  const QString targetVersion = normalizedRequestedVersion.isEmpty()
                                    ? m_latestPackageVersion.trimmed()
                                    : normalizedRequestedVersion;

  QStringList args;
  if (normalizedInstalledVersion.isEmpty()) {
    args.append(QStringLiteral("install"));
  } else if (!targetVersion.isEmpty()) {
    args.append(QStringLiteral("distro-sync"));
  } else {
    // Installing the named NVIDIA package set keeps the transaction scoped to
    // the driver stack instead of invoking a broad system update.
    args.append(QStringLiteral("install"));
  }

  args.append(QStringLiteral("-y"));
  args.append(QStringLiteral("--refresh"));
  args.append(QStringLiteral("--best"));

  if (!normalizedInstalledVersion.isEmpty()) {
    args.append(QStringLiteral("--allowerasing"));
  }

  const QStringList driverTargets =
      buildDriverTargets(targetVersion, sessionType, kernelPackageName);
  for (const QString &target : driverTargets) {
    args.append(target);
  }
  return args;
}

void NvidiaUpdater::refreshAvailableVersions() {
  QPointer<NvidiaUpdater> guard(this);
  runAsyncTask([guard]() {
    if (!guard) {
      return;
    }

    UpdateStatusSnapshot snapshot = collectUpdateStatus();
    QMetaObject::invokeMethod(
        guard,
        [guard, snapshot]() {
          if (!guard) {
            return;
          }

          guard->m_latestPackageVersion = snapshot.latestPackageVersion;
          guard->setAvailableVersions(snapshot.availableVersions);
        },
        Qt::QueuedConnection);
  });
}

void NvidiaUpdater::checkForUpdate() {
  // TR: Her kontrol denemesinde UI'ye gorunur bir baslangic mesaji gonder.
  // EN: Always emit a visible start message for each check request.
  emit progressMessage(tr("Starting update check..."));

  QPointer<NvidiaUpdater> guard(this);
  runAsyncTask([guard]() {
    if (!guard) {
      return;
    }

    const UpdateStatusSnapshot snapshot = collectUpdateStatus();
    QMetaObject::invokeMethod(
        guard,
        [guard, snapshot]() {
          if (!guard) {
            return;
          }

          if (guard->m_currentVersion != snapshot.currentVersion) {
            guard->m_currentVersion = snapshot.currentVersion;
            emit guard->currentVersionChanged();
          }

          if (guard->m_updateAvailable != snapshot.updateAvailable) {
            guard->m_updateAvailable = snapshot.updateAvailable;
            emit guard->updateAvailableChanged();
          }

          guard->m_latestPackageVersion = snapshot.latestPackageVersion;
          guard->setLatestVersion(snapshot.latestVersion);
          guard->setAvailableVersions(snapshot.availableVersions);
          const bool success = snapshot.updateAvailable ||
                               !snapshot.latestVersion.isEmpty() ||
                               snapshot.remoteCatalogAvailable ||
                               snapshot.currentVersion.isEmpty();
          emit guard->checkFinished(success, snapshot.message);
        },
        Qt::QueuedConnection);
  });
}

void NvidiaUpdater::applyUpdate() { applyVersion(QString()); }

void NvidiaUpdater::applyVersion(const QString &version) {
  const QString trimmedVersion = version.trimmed();
  const QStringList knownVersions = m_availableVersions;
  const QString hardwareMessage = missingNvidiaHardwareMessage();
  if (!hardwareMessage.isEmpty()) {
    emit updateFinished(false, hardwareMessage);
    return;
  }

  const QString architectureSupportMessage =
      CapabilityProbe::roAsdNvidiaDriverFlowSupportMessage();

  if (!architectureSupportMessage.isEmpty()) {
    emit updateFinished(false, architectureSupportMessage);
    return;
  }

  QPointer<NvidiaUpdater> guard(this);

  runAsyncTask([guard, trimmedVersion, knownVersions]() {
    if (!guard) {
      return;
    }

    CommandRunner runner;
    attachRunnerLogging(runner, guard);
    CommandRunner::RunOptions runOptions;
    runOptions.cancelRequested = guard->m_cancelRequested;

    if (QStandardPaths::findExecutable(QStringLiteral("dnf")).isEmpty()) {
      QMetaObject::invokeMethod(
          guard,
          [guard]() {
            if (guard) {
              emit guard->updateFinished(false,
                                         NvidiaUpdater::tr("dnf not found."));
            }
          },
          Qt::QueuedConnection);
      return;
    }

    NvidiaDetector detector;
    const QString installedVersion = detector.installedDriverVersion();
    const SessionUtil::SessionInfo sessionInfo = guard->detectSessionInfo();
    const QString sessionType = sessionInfo.type.trimmed().toLower();
    const QString kernelPackageName = guard->detectInstalledKernelPackageName();

    if (sessionType != QStringLiteral("wayland")) {
      QMetaObject::invokeMethod(
          guard,
          [guard]() {
            if (guard) {
              emit guard->updateFinished(
                  false, NvidiaUpdater::tr(
                             "The active display session could not be detected "
                             "as Wayland. ro-Control supports Wayland driver "
                             "setup only."));
            }
          },
          Qt::QueuedConnection);
      return;
    }

    if (!trimmedVersion.isEmpty() && !knownVersions.contains(trimmedVersion)) {
      QMetaObject::invokeMethod(
          guard,
          [guard]() {
            if (guard) {
              emit guard->updateFinished(
                  false, NvidiaUpdater::tr(
                             "Selected version not found in the repository."));
            }
          },
          Qt::QueuedConnection);
      return;
    }

    emitProgressAsync(
        guard, trimmedVersion.isEmpty()
                   ? NvidiaUpdater::tr(
                         "Updating NVIDIA driver to the latest version...")
                   : NvidiaUpdater::tr(
                         "Switching NVIDIA driver to selected version: %1")
                         .arg(trimmedVersion));

    const QStringList args = guard->buildTransactionArguments(
        trimmedVersion, installedVersion, sessionType, kernelPackageName);
    emitProgressAsync(
        guard, NvidiaUpdater::tr("Driver transaction kernel package: `%1`")
                   .arg(kernelPackageName));
    const QString transactionPackagesMessage =
        NvidiaUpdater::tr("Driver transaction packages for %1: %2")
            .arg(NvidiaUpdater::tr("Wayland"))
            .arg(quotedList(guard->buildDriverTargets(
                trimmedVersion.isEmpty() ? guard->m_latestPackageVersion
                                         : trimmedVersion,
                sessionType, kernelPackageName)));
    emitProgressAsync(guard, transactionPackagesMessage);

    QList<CommandRunner::RootCommand> rootCommands;
    {
      QStringList dnfLangArgs;
      dnfLangArgs << QStringLiteral("LANG=C") << QStringLiteral("dnf") << args;
      rootCommands.append({QStringLiteral("env"), dnfLangArgs});
    }
    rootCommands.append(
        {QStringLiteral("akmods"), {QStringLiteral("--force")}});
    rootCommands.append(buildSessionSpecificRootCommands(sessionType));

    const QString detectedSessionMessage =
        NvidiaUpdater::tr("Detected %1 session via %2.")
            .arg(sessionType == QStringLiteral("wayland")
                     ? NvidiaUpdater::tr("Wayland")
                     : sessionType,
                 sessionInfo.source.isEmpty()
                     ? NvidiaUpdater::tr("session probe")
                     : sessionInfo.source);
    emitProgressAsync(guard, detectedSessionMessage);

    auto result = runner.runAsRootBatch(rootCommands, runOptions);
    if (!result.success()) {
      const QString error =
          commandCanceled(result)
              ? NvidiaUpdater::tr("Operation canceled by user.")
              : NvidiaUpdater::tr("Update failed: ") +
                    commandError(result, NvidiaUpdater::tr("unknown error"));
      QMetaObject::invokeMethod(
          guard,
          [guard, error]() {
            if (guard) {
              emit guard->updateFinished(false, error);
            }
          },
          Qt::QueuedConnection);
      return;
    }

    if (!guard->transactionChanged(result)) {
      const UpdateStatusSnapshot snapshot = collectUpdateStatus();
      const QString noChangeMessage =
          trimmedVersion.isEmpty()
              ? NvidiaUpdater::tr(
                    "Driver is already at the latest available version.")
              : NvidiaUpdater::tr(
                    "Selected driver version is already installed.");

      QMetaObject::invokeMethod(
          guard,
          [guard, snapshot, noChangeMessage]() {
            if (!guard) {
              return;
            }

            if (guard->m_currentVersion != snapshot.currentVersion) {
              guard->m_currentVersion = snapshot.currentVersion;
              emit guard->currentVersionChanged();
            }
            if (guard->m_updateAvailable != snapshot.updateAvailable) {
              guard->m_updateAvailable = snapshot.updateAvailable;
              emit guard->updateAvailableChanged();
            }
            guard->m_latestPackageVersion = snapshot.latestPackageVersion;
            guard->setLatestVersion(snapshot.latestVersion);
            guard->setAvailableVersions(snapshot.availableVersions);
            emit guard->progressMessage(noChangeMessage);
            emit guard->updateFinished(true, noChangeMessage);
          },
          Qt::QueuedConnection);
      return;
    }

    const UpdateStatusSnapshot snapshot = collectUpdateStatus();
    const QString successMessage =
        trimmedVersion.isEmpty()
            ? (installedVersion.isEmpty()
                   ? NvidiaUpdater::tr("Latest version installed successfully. "
                                       "Please restart the system.")
                   : NvidiaUpdater::tr("Driver updated successfully. "
                                       "Please restart the system."))
            : NvidiaUpdater::tr("Selected version applied successfully. "
                                "Please restart the system.");

    QMetaObject::invokeMethod(
        guard,
        [guard, snapshot, successMessage]() {
          if (!guard) {
            return;
          }

          if (guard->m_currentVersion != snapshot.currentVersion) {
            guard->m_currentVersion = snapshot.currentVersion;
            emit guard->currentVersionChanged();
          }
          if (guard->m_updateAvailable != snapshot.updateAvailable) {
            guard->m_updateAvailable = snapshot.updateAvailable;
            emit guard->updateAvailableChanged();
          }
          guard->m_latestPackageVersion = snapshot.latestPackageVersion;
          guard->setLatestVersion(snapshot.latestVersion);
          guard->setAvailableVersions(snapshot.availableVersions);
          emit guard->progressMessage(snapshot.message);
          emit guard->updateFinished(true, successMessage);
        },
        Qt::QueuedConnection);
  });
}
