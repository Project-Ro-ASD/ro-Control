#pragma once

#include <QElapsedTimer>
#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>

class DiskMonitor : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool available READ available NOTIFY availableChanged)
  Q_PROPERTY(bool running READ running NOTIFY runningChanged)
  Q_PROPERTY(QVariantList diskStats READ diskStats NOTIFY diskStatsChanged)
  Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval
                 NOTIFY updateIntervalChanged)
  Q_PROPERTY(int diskCount READ diskCount NOTIFY diskStatsChanged)
  Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY
                 statusMessageChanged)

public:
  struct DiskStats {
    QString device;
    QString mountPoint;
    QString fsType;
    quint64 totalBytes = 0;
    quint64 freeBytes = 0;
    quint64 usedBytes = 0;
    double usagePercent = 0.0;
    quint64 readBytes = 0;   // since boot, from /proc/diskstats
    quint64 writeBytes = 0;  // since boot, from /proc/diskstats
    quint64 readBytesPerSec = 0;
    quint64 writeBytesPerSec = 0;
  };

  explicit DiskMonitor(QObject *parent = nullptr);
  ~DiskMonitor() override = default;

  bool available() const;
  bool running() const;
  int diskCount() const;
  QVariantList diskStats() const;
  int updateInterval() const;
  QString statusMessage() const;

  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();
  Q_INVOKABLE void refresh();
  void setUpdateInterval(int intervalMs);

signals:
  void availableChanged();
  void runningChanged();
  void diskStatsChanged();
  void updateIntervalChanged();
  void statusMessageChanged();

private:
  void setAvailable(bool value);
  void setStatusMessage(const QString &value);

  static QString mountsPath();
  static QString diskstatsPath();

  QTimer m_timer;
  bool m_available = false;
  QString m_statusMessage;
  QList<DiskStats> m_disks;

  // Previous I/O counters (per base device name) for rate computation.
  QHash<QString, quint64> m_prevReadSectors;
  QHash<QString, quint64> m_prevWriteSectors;
  QElapsedTimer m_sampleTimer;
};