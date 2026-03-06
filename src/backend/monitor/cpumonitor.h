#pragma once

#include <QObject>
#include <QTimer>

// CpuMonitor: /proc/stat üzerinden gerçek zamanlı CPU istatistikleri.
class CpuMonitor : public QObject {
  Q_OBJECT

  Q_PROPERTY(int load READ load NOTIFY loadChanged)
  Q_PROPERTY(int temperature READ temperature NOTIFY temperatureChanged)

public:
  explicit CpuMonitor(QObject *parent = nullptr);

  int load() const { return m_load; }
  int temperature() const { return m_temperature; }

  Q_INVOKABLE void start(int intervalMs = 1000);
  Q_INVOKABLE void stop();

signals:
  void loadChanged();
  void temperatureChanged();

private slots:
  void poll();

private:
  int readCpuTemp() const;

  QTimer m_timer;
  int m_load = 0;
  int m_temperature = 0;

  // /proc/stat için önceki değerler (delta hesabı)
  long long m_prevIdle = 0;
  long long m_prevTotal = 0;
};
