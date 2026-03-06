#pragma once

#include <QObject>
#include <QTimer>

// Gercek zamanli GPU istatistikleri
class GpuMonitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString gpuName READ gpuName NOTIFY gpuNameChanged)
    Q_PROPERTY(int temperatureC READ temperatureC NOTIFY temperatureCChanged)
    Q_PROPERTY(int utilizationPercent READ utilizationPercent NOTIFY utilizationPercentChanged)
    Q_PROPERTY(int memoryUsedMiB READ memoryUsedMiB NOTIFY memoryUsedMiBChanged)
    Q_PROPERTY(int memoryTotalMiB READ memoryTotalMiB NOTIFY memoryTotalMiBChanged)
    Q_PROPERTY(int memoryUsagePercent READ memoryUsagePercent NOTIFY memoryUsagePercentChanged)
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)

public:
    explicit GpuMonitor(QObject *parent = nullptr);

    bool available() const;
    bool running() const;
    QString gpuName() const;
    int temperatureC() const;
    int utilizationPercent() const;
    int memoryUsedMiB() const;
    int memoryTotalMiB() const;
    int memoryUsagePercent() const;
    int updateInterval() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    void setUpdateInterval(int intervalMs);

signals:
    void availableChanged();
    void runningChanged();
    void gpuNameChanged();
    void temperatureCChanged();
    void utilizationPercentChanged();
    void memoryUsedMiBChanged();
    void memoryTotalMiBChanged();
    void memoryUsagePercentChanged();
    void updateIntervalChanged();

private:
    void clearMetrics();
    void setAvailable(bool value);

    QTimer m_timer;
    bool m_available = false;
    QString m_gpuName;
    int m_temperatureC = 0;
    int m_utilizationPercent = 0;
    int m_memoryUsedMiB = 0;
    int m_memoryTotalMiB = 0;
    int m_memoryUsagePercent = 0;
};
