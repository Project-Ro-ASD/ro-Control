#pragma once

#include <QObject>
#include <QTimer>

// Gercek zamanli RAM istatistikleri
class RamMonitor : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool available READ available NOTIFY availableChanged)
  Q_PROPERTY(bool running READ running NOTIFY runningChanged)
  Q_PROPERTY(int totalMiB READ totalMiB NOTIFY totalMiBChanged)
  Q_PROPERTY(int usedMiB READ usedMiB NOTIFY usedMiBChanged)
  Q_PROPERTY(int usagePercent READ usagePercent NOTIFY usagePercentChanged)
  Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval
                 NOTIFY updateIntervalChanged)

public:
  explicit RamMonitor(QObject *parent = nullptr);

  bool available() const;
  bool running() const;
  int totalMiB() const;
  int usedMiB() const;
  int usagePercent() const;
  int updateInterval() const;

  Q_INVOKABLE void refresh();
  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();
  void setUpdateInterval(int intervalMs);

signals:
  void availableChanged();
  void runningChanged();
  void totalMiBChanged();
  void usedMiBChanged();
  void usagePercentChanged();
  void updateIntervalChanged();

private:
  void clearMetrics();
  void setAvailable(bool value);

  QTimer m_timer;
  bool m_available = false;
  int m_totalMiB = 0;
  int m_usedMiB = 0;
  int m_usagePercent = 0;
};
