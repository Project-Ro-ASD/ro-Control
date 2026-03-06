// CPU istatistikleri

#include "cpumonitor.h"

#include <QFile>
#include <QTextStream>

#include <algorithm>

namespace {

int readCpuTemperatureC() {
    for (int i = 0; i < 32; ++i) {
        QFile thermalFile(
                QString("/sys/class/thermal/thermal_zone%1/temp").arg(i));
        if (!thermalFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        const QByteArray raw = thermalFile.readAll().trimmed();
        bool ok = false;
        const int milliC = raw.toInt(&ok);
        if (ok && milliC > 0) {
            return milliC / 1000;
        }
    }

    for (int i = 0; i < 32; ++i) {
        QFile hwmonFile(QString("/sys/class/hwmon/hwmon%1/temp1_input").arg(i));
        if (!hwmonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        const QByteArray raw = hwmonFile.readAll().trimmed();
        bool ok = false;
        const int milliC = raw.toInt(&ok);
        if (ok && milliC > 0) {
            return milliC / 1000;
        }
    }

    return 0;
}

}  // namespace

CpuMonitor::CpuMonitor(QObject *parent)
        : QObject(parent) {
    m_timer.setInterval(1000);
    m_timer.setTimerType(Qt::VeryCoarseTimer);
    connect(&m_timer, &QTimer::timeout, this, &CpuMonitor::refresh);

    start();
    refresh();
}

double CpuMonitor::usagePercent() const { return m_usagePercent; }

int CpuMonitor::temperatureC() const { return m_temperatureC; }

bool CpuMonitor::available() const { return m_available; }

bool CpuMonitor::running() const { return m_timer.isActive(); }

int CpuMonitor::updateInterval() const { return m_timer.interval(); }

void CpuMonitor::refresh() {
    QFile statFile("/proc/stat");
    if (!statFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setAvailable(false);
        setUsagePercent(0.0);
        setTemperatureC(0);
        return;
    }

    QTextStream stream(&statFile);
    const QString firstLine = stream.readLine();

    if (!firstLine.startsWith("cpu ")) {
        setAvailable(false);
        return;
    }

    const QStringList parts = firstLine.split(' ', Qt::SkipEmptyParts);
    if (parts.size() < 8) {
        setAvailable(false);
        return;
    }

    bool ok = true;
    const quint64 user = parts.value(1).toULongLong(&ok);
    if (!ok) {
        setAvailable(false);
        return;
    }

    const quint64 nice = parts.value(2).toULongLong(&ok);
    const quint64 system = parts.value(3).toULongLong(&ok);
    const quint64 idle = parts.value(4).toULongLong(&ok);
    const quint64 iowait = parts.value(5).toULongLong(&ok);
    const quint64 irq = parts.value(6).toULongLong(&ok);
    const quint64 softirq = parts.value(7).toULongLong(&ok);
    const quint64 steal = parts.size() > 8 ? parts.value(8).toULongLong(&ok) : 0;

    if (!ok) {
        setAvailable(false);
        return;
    }

    const quint64 idleAll = idle + iowait;
    const quint64 nonIdle = user + nice + system + irq + softirq + steal;
    const quint64 total = idleAll + nonIdle;

    if (m_prevTotal > 0 && total >= m_prevTotal && idleAll >= m_prevIdle) {
        const quint64 totalDelta = total - m_prevTotal;
        const quint64 idleDelta = idleAll - m_prevIdle;

        if (totalDelta > 0) {
            const double value =
                    (static_cast<double>(totalDelta - idleDelta) /
                     static_cast<double>(totalDelta)) *
                    100.0;
            setUsagePercent(std::clamp(value, 0.0, 100.0));
        }
    }

    m_prevTotal = total;
    m_prevIdle = idleAll;

    setTemperatureC(readCpuTemperatureC());
    setAvailable(true);
}

void CpuMonitor::start() {
    if (m_timer.isActive()) {
        return;
    }

    m_timer.start();
    emit runningChanged();
}

void CpuMonitor::stop() {
    if (!m_timer.isActive()) {
        return;
    }

    m_timer.stop();
    emit runningChanged();
}

void CpuMonitor::setUpdateInterval(int intervalMs) {
    if (intervalMs < 250 || m_timer.interval() == intervalMs) {
        return;
    }

    m_timer.setInterval(intervalMs);
    emit updateIntervalChanged();
}

void CpuMonitor::setUsagePercent(double value) {
    if (qFuzzyCompare(m_usagePercent, value)) {
        return;
    }

    m_usagePercent = value;
    emit usagePercentChanged();
}

void CpuMonitor::setTemperatureC(int value) {
    if (m_temperatureC == value) {
        return;
    }

    m_temperatureC = value;
    emit temperatureCChanged();
}

void CpuMonitor::setAvailable(bool value) {
    if (m_available == value) {
        return;
    }

    m_available = value;
    emit availableChanged();
}
