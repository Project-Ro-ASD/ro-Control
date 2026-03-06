#pragma once

#include <QObject>
#include <QString>

// NvidiaUpdater: Kurulu surucu ile mevcut en guncel surumu karsilastirir.
class NvidiaUpdater : public QObject {
  Q_OBJECT

  Q_PROPERTY(
      bool updateAvailable READ updateAvailable NOTIFY updateAvailableChanged)
  Q_PROPERTY(
      QString currentVersion READ currentVersion NOTIFY currentVersionChanged)
  Q_PROPERTY(
      QString latestVersion READ latestVersion NOTIFY latestVersionChanged)

public:
  explicit NvidiaUpdater(QObject *parent = nullptr);

  bool updateAvailable() const { return m_updateAvailable; }
  QString currentVersion() const { return m_currentVersion; }
  QString latestVersion() const { return m_latestVersion; }

  Q_INVOKABLE void checkForUpdate();
  Q_INVOKABLE void applyUpdate();

signals:
  void updateAvailableChanged();
  void currentVersionChanged();
  void latestVersionChanged();
  void progressMessage(const QString &message);
  void updateFinished(bool success, const QString &message);

private:
  bool m_updateAvailable = false;
  QString m_currentVersion;
  QString m_latestVersion;
};
