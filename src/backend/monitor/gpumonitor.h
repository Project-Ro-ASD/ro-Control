#pragma once

#include <QObject>
#include <QTimer>

#include "system/commandrunner.h"

// Gercek zamanli GPU istatistikleri
class GpuMonitor : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool available READ available NOTIFY availableChanged)
  Q_PROPERTY(bool running READ running NOTIFY runningChanged)
  Q_PROPERTY(QString gpuName READ gpuName NOTIFY gpuNameChanged)
  Q_PROPERTY(int temperatureC READ temperatureC NOTIFY temperatureCChanged)
  Q_PROPERTY(int hotspotTemperatureC READ hotspotTemperatureC NOTIFY
                 hotspotTemperatureCChanged)
  Q_PROPERTY(int memoryTemperatureC READ memoryTemperatureC NOTIFY
                 memoryTemperatureCChanged)
  Q_PROPERTY(int utilizationPercent READ utilizationPercent NOTIFY
                 utilizationPercentChanged)
  Q_PROPERTY(int memoryUsedMiB READ memoryUsedMiB NOTIFY memoryUsedMiBChanged)
  Q_PROPERTY(
      int memoryTotalMiB READ memoryTotalMiB NOTIFY memoryTotalMiBChanged)
  Q_PROPERTY(int memoryUsagePercent READ memoryUsagePercent NOTIFY
                 memoryUsagePercentChanged)
  Q_PROPERTY(
      int fanSpeedPercent READ fanSpeedPercent NOTIFY fanSpeedPercentChanged)
  Q_PROPERTY(int thermalLimitTemperatureC READ thermalLimitTemperatureC NOTIFY
                 thermalLimitTemperatureCChanged)
  Q_PROPERTY(double powerDrawW READ powerDrawW NOTIFY powerDrawWChanged)
  Q_PROPERTY(double powerLimitW READ powerLimitW NOTIFY powerLimitWChanged)
  Q_PROPERTY(
      int graphicsClockMHz READ graphicsClockMHz NOTIFY graphicsClockMHzChanged)
  Q_PROPERTY(
      int memoryClockMHz READ memoryClockMHz NOTIFY memoryClockMHzChanged)
  Q_PROPERTY(
      QString pcieLinkStatus READ pcieLinkStatus NOTIFY pcieLinkStatusChanged)
  Q_PROPERTY(
      QVariantList gpuProcesses READ gpuProcesses NOTIFY gpuProcessesChanged)
  Q_PROPERTY(
      int gpuProcessCount READ gpuProcessCount NOTIFY gpuProcessesChanged)
  Q_PROPERTY(int gpuCount READ gpuCount NOTIFY gpuDevicesChanged)
  Q_PROPERTY(int selectedGpuIndex READ selectedGpuIndex WRITE
                 setSelectedGpuIndex NOTIFY selectedGpuIndexChanged)
  Q_PROPERTY(QVariantList gpuDevices READ gpuDevices NOTIFY gpuDevicesChanged)
  Q_PROPERTY(
      QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
  Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval
                 NOTIFY updateIntervalChanged)

public:
  explicit GpuMonitor(QObject *parent = nullptr);

  bool available() const;
  bool running() const;
  QString gpuName() const;
  int temperatureC() const;
  int hotspotTemperatureC() const;
  int memoryTemperatureC() const;
  int utilizationPercent() const;
  int memoryUsedMiB() const;
  int memoryTotalMiB() const;
  int memoryUsagePercent() const;
  int fanSpeedPercent() const;
  int thermalLimitTemperatureC() const;
  double powerDrawW() const;
  double powerLimitW() const;
  int graphicsClockMHz() const;
  int memoryClockMHz() const;
  QString pcieLinkStatus() const;
  QVariantList gpuProcesses() const;
  int gpuProcessCount() const;
  int gpuCount() const;
  int selectedGpuIndex() const;
  QVariantList gpuDevices() const;
  QString statusMessage() const;
  int updateInterval() const;

  Q_INVOKABLE void refresh();
  // UI-triggered refreshes must not run nvidia-smi on the QML thread.
  Q_INVOKABLE void requestRefresh();
  void refreshAsync();
  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();
  Q_INVOKABLE bool killProcess(int pid);
  Q_INVOKABLE void setSelectedGpuIndex(int index);
  void setUpdateInterval(int intervalMs);

signals:
  void availableChanged();
  void runningChanged();
  void gpuNameChanged();
  void temperatureCChanged();
  void hotspotTemperatureCChanged();
  void memoryTemperatureCChanged();
  void utilizationPercentChanged();
  void memoryUsedMiBChanged();
  void memoryTotalMiBChanged();
  void memoryUsagePercentChanged();
  void fanSpeedPercentChanged();
  void thermalLimitTemperatureCChanged();
  void powerDrawWChanged();
  void powerLimitWChanged();
  void graphicsClockMHzChanged();
  void memoryClockMHzChanged();
  void pcieLinkStatusChanged();
  void gpuProcessesChanged();
  void gpuDevicesChanged();
  void selectedGpuIndexChanged();
  void statusMessageChanged();
  void updateIntervalChanged();

private:
  void processRefreshResult(const CommandRunner::Result &result);
  void clearMetrics();
  void setAvailable(bool value);
  void setStatusMessage(const QString &value);
  void queryGpuProcesses(bool force = false);
  void queryGpuDevices(bool force = false);

  QTimer m_timer;
  bool m_available = false;
  QString m_gpuName;
  QString m_statusMessage;
  int m_temperatureC = 0;
  int m_hotspotTemperatureC = 0;
  int m_memoryTemperatureC = 0;
  int m_utilizationPercent = 0;
  int m_memoryUsedMiB = 0;
  int m_memoryTotalMiB = 0;
  int m_memoryUsagePercent = 0;
  int m_fanSpeedPercent = 0;
  int m_thermalLimitC = 0;
  double m_powerDrawW = 0.0;
  double m_powerLimitW = 0.0;
  int m_graphicsClockMHz = 0;
  int m_memoryClockMHz = 0;
  QString m_pcieLinkStatus;
  QVariantList m_gpuProcesses;
  QVariantList m_gpuDevices;
  int m_selectedGpuIndex = 0;
  quint64 m_refreshTickCount = 0;
  bool m_asyncRefreshInFlight = false;
};
