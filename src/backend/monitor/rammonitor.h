#pragma once

#include <QObject>
#include <QTimer>

// RamMonitor: /proc/meminfo üzerinden RAM kullanımı.
class RamMonitor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int totalMb    READ totalMb    NOTIFY totalMbChanged)
    Q_PROPERTY(int usedMb     READ usedMb     NOTIFY usedMbChanged)
    Q_PROPERTY(int usedPercent READ usedPercent NOTIFY usedPercentChanged)

public:
    explicit RamMonitor(QObject *parent = nullptr);

    int totalMb()     const { return m_totalMb; }
    int usedMb()      const { return m_usedMb; }
    int usedPercent() const { return m_usedPercent; }

    Q_INVOKABLE void start(int intervalMs = 2000);
    Q_INVOKABLE void stop();

signals:
    void totalMbChanged();
    void usedMbChanged();
    void usedPercentChanged();

private slots:
    void poll();

private:
    QTimer m_timer;
    int    m_totalMb     = 0;
    int    m_usedMb      = 0;
    int    m_usedPercent = 0;
};
