#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <atomic>
#include <functional>
#include <memory>

#include "system/commandrunner.h"

namespace SessionUtil {
struct SessionInfo;
}

// NvidiaUpdater compares the installed driver state with newer available
// builds.
class NvidiaUpdater : public QObject {
  Q_OBJECT

  Q_PROPERTY(
      bool updateAvailable READ updateAvailable NOTIFY updateAvailableChanged)
  Q_PROPERTY(
      QString currentVersion READ currentVersion NOTIFY currentVersionChanged)
  Q_PROPERTY(
      QString latestVersion READ latestVersion NOTIFY latestVersionChanged)
  Q_PROPERTY(QStringList availableVersions READ availableVersions NOTIFY
                 availableVersionsChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
  friend class TestUpdater;
  explicit NvidiaUpdater(QObject *parent = nullptr);

  bool updateAvailable() const { return m_updateAvailable; }
  QString currentVersion() const { return m_currentVersion; }
  QString latestVersion() const { return m_latestVersion; }
  QStringList availableVersions() const { return m_availableVersions; }
  bool busy() const { return m_busy; }

  Q_INVOKABLE void checkForUpdate();
  Q_INVOKABLE void applyUpdate();
  Q_INVOKABLE void applyVersion(const QString &version);
  Q_INVOKABLE void refreshAvailableVersions();
  Q_INVOKABLE void cancelOperation();
  void setLatestPackageVersion(const QString &version) {
    m_latestPackageVersion = version;
  }

  bool transactionChanged(const CommandRunner::Result &result) const;
  QStringList buildTransactionArguments(const QString &requestedVersion,
                                        const QString &installedVersion,
                                        const QString &sessionType,
                                        const QString &kernelPackageName) const;
  QStringList buildDriverTargets(const QString &version,
                                 const QString &sessionType,
                                 const QString &kernelPackageName) const;

signals:
  void updateAvailableChanged();
  void currentVersionChanged();
  void latestVersionChanged();
  void availableVersionsChanged();
  void busyChanged();
  void progressMessage(const QString &message);
  void checkFinished(bool success, const QString &message);
  void updateFinished(bool success, const QString &message);

private:
  void setBusy(bool busy);
  void runAsyncTask(const std::function<void()> &task);
  void setLatestVersion(const QString &version);
  void setAvailableVersions(const QStringList &versions);
  QString detectInstalledKernelPackageName() const;
  SessionUtil::SessionInfo detectSessionInfo() const;
  QString m_latestPackageVersion;
  bool m_updateAvailable = false;
  QString m_currentVersion;
  QString m_latestVersion;
  QStringList m_availableVersions;
  bool m_busy = false;
  std::shared_ptr<std::atomic_bool> m_cancelRequested;
};
