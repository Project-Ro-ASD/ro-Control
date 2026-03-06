#include "rammonitor.h"

#include <QFile>
#include <QTextStream>

#include <algorithm>

RamMonitor::RamMonitor(QObject *parent)
        : QObject(parent) {
    m_timer.setInterval(1000);
    m_timer.setTimerType(Qt::VeryCoarseTimer);
    connect(&m_timer, &QTimer::timeout, this, &RamMonitor::refresh);

    start();
    refresh();
}

bool RamMonitor::available() const { return m_available; }

bool RamMonitor::running() const { return m_timer.isActive(); }

int RamMonitor::totalMiB() const { return m_totalMiB; }

int RamMonitor::usedMiB() const { return m_usedMiB; }

int RamMonitor::usagePercent() const { return m_usagePercent; }

int RamMonitor::updateInterval() const { return m_timer.interval(); }

void RamMonitor::refresh() {
    QFile meminfo("/proc/meminfo");
    if (!meminfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setAvailable(false);
        clearMetrics();
        return;
    }

    qint64 memTotalKiB = -1;
    qint64 memAvailableKiB = -1;

    QTextStream stream(&meminfo);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();

        if (line.startsWith("MemTotal:")) {
            const QString value = line.section(':', 1, 1).trimmed().section(' ', 0, 0);
            bool ok = false;
            memTotalKiB = value.toLongLong(&ok);
            if (!ok) {
                memTotalKiB = -1;
            }
        } else if (line.startsWith("MemAvailable:")) {
            const QString value = line.section(':', 1, 1).trimmed().section(' ', 0, 0);
            bool ok = false;
            memAvailableKiB = value.toLongLong(&ok);
            if (!ok) {
                memAvailableKiB = -1;
            }
        }

        if (memTotalKiB >= 0 && memAvailableKiB >= 0) {
            break;
        }
    }

    if (memTotalKiB <= 0 || memAvailableKiB < 0 || memAvailableKiB > memTotalKiB) {
        setAvailable(false);
        clearMetrics();
        return;
    }

    const qint64 usedKiB = memTotalKiB - memAvailableKiB;
    const int nextTotalMiB = static_cast<int>(memTotalKiB / 1024);
    const int nextUsedMiB = static_cast<int>(usedKiB / 1024);
    const int nextUsagePercent = std::clamp(
            static_cast<int>((static_cast<double>(usedKiB) /
                                                static_cast<double>(memTotalKiB)) *
                                             100.0),
            0, 100);

    if (m_totalMiB != nextTotalMiB) {
        m_totalMiB = nextTotalMiB;
        emit totalMiBChanged();
    }

    if (m_usedMiB != nextUsedMiB) {
        m_usedMiB = nextUsedMiB;
        emit usedMiBChanged();
    }

    if (m_usagePercent != nextUsagePercent) {
        m_usagePercent = nextUsagePercent;
        emit usagePercentChanged();
    }

    setAvailable(true);
}

void RamMonitor::start() {
    if (m_timer.isActive()) {
        return;
    }

    m_timer.start();
    emit runningChanged();
}

void RamMonitor::stop() {
    if (!m_timer.isActive()) {
        return;
    }

    m_timer.stop();
    emit runningChanged();
}

void RamMonitor::setUpdateInterval(int intervalMs) {
    if (intervalMs < 250 || m_timer.interval() == intervalMs) {
        return;
    }

    m_timer.setInterval(intervalMs);
    emit updateIntervalChanged();
}

void RamMonitor::clearMetrics() {
    if (m_totalMiB != 0) {
        m_totalMiB = 0;
        emit totalMiBChanged();
    }

    if (m_usedMiB != 0) {
        m_usedMiB = 0;
        emit usedMiBChanged();
    }

    if (m_usagePercent != 0) {
        m_usagePercent = 0;
        emit usagePercentChanged();
    }
}

void RamMonitor::setAvailable(bool value) {
    if (m_available == value) {
        return;
    }

    m_available = value;
    emit availableChanged();
}
