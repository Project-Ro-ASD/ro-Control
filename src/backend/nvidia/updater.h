#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class CommandRunner;

// NvidiaUpdater: Kurulu surucu ile mevcut en guncel surumu karsilastirir.
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

public:
  explicit NvidiaUpdater(QObject *parent = nullptr);

  bool updateAvailable() const { return m_updateAvailable; }
  QString currentVersion() const { return m_currentVersion; }
  QString latestVersion() const { return m_latestVersion; }
  QStringList availableVersions() const { return m_availableVersions; }

  Q_INVOKABLE void checkForUpdate();
  Q_INVOKABLE void applyUpdate();
  Q_INVOKABLE void applyVersion(const QString &version);
  Q_INVOKABLE void refreshAvailableVersions();

signals:
  void updateAvailableChanged();
  void currentVersionChanged();
  void latestVersionChanged();
  void availableVersionsChanged();
  void progressMessage(const QString &message);
  void updateFinished(bool success, const QString &message);

private:
  void setLatestVersion(const QString &version);
  void setAvailableVersions(const QStringList &versions);
  QStringList buildDriverTargets(const QString &version,
                                 const QString &sessionType) const;
  bool finalizeDriverChange(CommandRunner &runner, const QString &sessionType,
                            QString *errorMessage);
  QString detectSessionType() const;
  bool m_updateAvailable = false;
  QString m_currentVersion;
  QString m_latestVersion;
  QStringList m_availableVersions;
};
