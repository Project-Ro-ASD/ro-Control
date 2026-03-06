#pragma once

#include <QObject>
#include <QTimer>

// Gercek zamanli CPU istatistikleri
class CpuMonitor : public QObject {
  Q_OBJECT
  Q_PROPERTY(double usagePercent READ usagePercent NOTIFY usagePercentChanged)
  Q_PROPERTY(int temperatureC READ temperatureC NOTIFY temperatureCChanged)
  Q_PROPERTY(bool available READ available NOTIFY availableChanged)
  Q_PROPERTY(bool running READ running NOTIFY runningChanged)
  Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval
                 NOTIFY updateIntervalChanged)

public:
  explicit CpuMonitor(QObject *parent = nullptr);

  double usagePercent() const;
  int temperatureC() const;
  bool available() const;
  bool running() const;
  int updateInterval() const;

  Q_INVOKABLE void refresh();
  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();
  void setUpdateInterval(int intervalMs);

signals:
  void usagePercentChanged();
  void temperatureCChanged();
  void availableChanged();
  void runningChanged();
  void updateIntervalChanged();

private:
  void setUsagePercent(double value);
  void setTemperatureC(int value);
  void setAvailable(bool value);

  QTimer m_timer;
  double m_usagePercent = 0.0;
  int m_temperatureC = 0;
  bool m_available = false;
  quint64 m_prevIdle = 0;
  quint64 m_prevTotal = 0;
};
