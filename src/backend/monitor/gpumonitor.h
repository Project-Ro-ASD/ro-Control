#pragma once

#include <QObject>
#include <QTimer>

// GpuMonitor: nvidia-smi üzerinden gerçek zamanlı GPU istatistikleri.
// QML bu sınıfın property'lerine bağlanır — otomatik güncellenir.
class GpuMonitor : public QObject {
  Q_OBJECT

  // QML'den okunabilir property'ler
  Q_PROPERTY(int temperature READ temperature NOTIFY temperatureChanged)
  Q_PROPERTY(int load READ load NOTIFY loadChanged)
  Q_PROPERTY(int vramUsed READ vramUsed NOTIFY vramUsedChanged)
  Q_PROPERTY(int vramTotal READ vramTotal NOTIFY vramTotalChanged)
  Q_PROPERTY(bool available READ available NOTIFY availableChanged)

public:
  explicit GpuMonitor(QObject *parent = nullptr);

  int temperature() const { return m_temperature; }
  int load() const { return m_load; }
  int vramUsed() const { return m_vramUsed; }
  int vramTotal() const { return m_vramTotal; }
  bool available() const { return m_available; }

  // Polling'i başlat/durdur (ms cinsinden interval)
  Q_INVOKABLE void start(int intervalMs = 1000);
  Q_INVOKABLE void stop();

signals:
  void temperatureChanged();
  void loadChanged();
  void vramUsedChanged();
  void vramTotalChanged();
  void availableChanged();

private slots:
  void poll();

private:
  QTimer m_timer;

  int m_temperature = 0;
  int m_load = 0;
  int m_vramUsed = 0;
  int m_vramTotal = 0;
  bool m_available = false;
};
