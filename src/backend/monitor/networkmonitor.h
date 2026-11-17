#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

#include <QHash>

class NetworkMonitor : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool available READ available NOTIFY availableChanged)
  Q_PROPERTY(bool running READ running NOTIFY runningChanged)
  Q_PROPERTY(int interfaceCount READ interfaceCount NOTIFY networkStatsChanged)
  Q_PROPERTY(quint64 totalRxBytesPerSec READ totalRxBytesPerSec NOTIFY networkStatsChanged)
  Q_PROPERTY(quint64 totalTxBytesPerSec READ totalTxBytesPerSec NOTIFY networkStatsChanged)
  Q_PROPERTY(QVariantList interfaces READ interfaces NOTIFY networkStatsChanged)
  Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval
                 NOTIFY updateIntervalChanged)
  Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY
                 statusMessageChanged)

public:
  struct NetworkInterfaceStats {
    QString name;
    quint64 rxBytes = 0;   // since boot
    quint64 txBytes = 0;   // since boot
    quint64 rxBytesPerSec = 0;
    quint64 txBytesPerSec = 0;
    quint64 rxPackets = 0;
    quint64 txPackets = 0;
    quint64 rxErrors = 0;
    quint64 txErrors = 0;
    quint64 rxDropped = 0;
    quint64 txDropped = 0;
  };

  explicit NetworkMonitor(QObject *parent = nullptr);

  bool available() const;
  bool running() const;
  int interfaceCount() const;
  quint64 totalRxBytesPerSec() const;
  quint64 totalTxBytesPerSec() const;
  QVariantList interfaces() const;
  int updateInterval() const;
  QString statusMessage() const;

  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();
  Q_INVOKABLE void refresh();
  void setUpdateInterval(int intervalMs);

signals:
  void availableChanged();
  void runningChanged();
  void networkStatsChanged();
  void updateIntervalChanged();
  void statusMessageChanged();

private:
  void parseNetDev();
  void computeDeltas();
  void setAvailable(bool value);
  void setStatusMessage(const QString &value);

  static QString netDevPath();

  QTimer m_timer;
  bool m_available = false;
  QString m_statusMessage;
  QList<NetworkInterfaceStats> m_interfaces;
  quint64 m_totalRxBytesPerSec = 0;
  quint64 m_totalTxBytesPerSec = 0;

  // Previous samples for rate computation
  QHash<QString, quint64> m_prevRxBytes;
  QHash<QString, quint64> m_prevTxBytes;
  QElapsedTimer m_sampleTimer;
};