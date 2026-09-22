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
  Q_PROPERTY(bool zramAvailable READ zramAvailable NOTIFY zramChanged)
  Q_PROPERTY(int zramTotalMiB READ zramTotalMiB NOTIFY zramChanged)
  Q_PROPERTY(int zramUsedMiB READ zramUsedMiB NOTIFY zramChanged)
  Q_PROPERTY(int zramPhysicalMiB READ zramPhysicalMiB NOTIFY zramChanged)
  Q_PROPERTY(
      double zramCompressionRatio READ zramCompressionRatio NOTIFY zramChanged)
  Q_PROPERTY(bool zswapEnabled READ zswapEnabled NOTIFY zswapChanged)
  Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval
                 NOTIFY updateIntervalChanged)

public:
  explicit RamMonitor(QObject *parent = nullptr);

  bool available() const;
  bool running() const;
  int totalMiB() const;
  int usedMiB() const;
  int usagePercent() const;
  bool zramAvailable() const;
  int zramTotalMiB() const;
  int zramUsedMiB() const;
  int zramPhysicalMiB() const;
  double zramCompressionRatio() const;
  bool zswapEnabled() const;
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
  void zramChanged();
  void zswapChanged();
  void updateIntervalChanged();

private:
  void clearMetrics();
  void refreshCompressionTelemetry();
  void setAvailable(bool value);

  QTimer m_timer;
  bool m_available = false;
  int m_totalMiB = 0;
  int m_usedMiB = 0;
  int m_usagePercent = 0;
  bool m_zramAvailable = false;
  int m_zramTotalMiB = 0;
  int m_zramUsedMiB = 0;
  int m_zramPhysicalMiB = 0;
  double m_zramCompressionRatio = 0.0;
  bool m_zswapEnabled = false;
};
