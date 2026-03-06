#include "gpumonitor.h"
#include "system/commandrunner.h"

#include <QProcess>

#include <algorithm>

GpuMonitor::GpuMonitor(QObject *parent)
        : QObject(parent) {
    m_timer.setInterval(1500);
    m_timer.setTimerType(Qt::VeryCoarseTimer);
    connect(&m_timer, &QTimer::timeout, this, &GpuMonitor::refresh);

    start();
    refresh();
}

bool GpuMonitor::available() const { return m_available; }

bool GpuMonitor::running() const { return m_timer.isActive(); }

QString GpuMonitor::gpuName() const { return m_gpuName; }

int GpuMonitor::temperatureC() const { return m_temperatureC; }

int GpuMonitor::utilizationPercent() const { return m_utilizationPercent; }

int GpuMonitor::memoryUsedMiB() const { return m_memoryUsedMiB; }

int GpuMonitor::memoryTotalMiB() const { return m_memoryTotalMiB; }

int GpuMonitor::memoryUsagePercent() const { return m_memoryUsagePercent; }

int GpuMonitor::updateInterval() const { return m_timer.interval(); }

void GpuMonitor::refresh() {
    QProcess process;
    process.start("nvidia-smi",
                                {"--query-gpu=name,temperature.gpu,utilization.gpu,memory.used,"
                                 "memory.total",
                                 "--format=csv,noheader,nounits"});

    if (!process.waitForFinished(1500) ||
            process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        setAvailable(false);
        clearMetrics();
        return;
    }

    const QString stdoutText = QString::fromUtf8(process.readAllStandardOutput());
    const QString firstLine = stdoutText.split('\n', Qt::SkipEmptyParts).value(0);
    const QStringList fields = firstLine.split(',', Qt::KeepEmptyParts);

    if (fields.size() < 5) {
        setAvailable(false);
        clearMetrics();
        return;
    }

    bool ok = true;
    const QString nextName = fields.at(0).trimmed();
    const int nextTemp = fields.at(1).trimmed().toInt(&ok);
    if (!ok) {
        setAvailable(false);
        clearMetrics();
        return;
    }

    const int nextUtil = fields.at(2).trimmed().toInt(&ok);
    const int nextUsed = fields.at(3).trimmed().toInt(&ok);
    const int nextTotal = fields.at(4).trimmed().toInt(&ok);
    if (!ok || nextTotal < 0 || nextUsed < 0) {
        setAvailable(false);
        clearMetrics();
        return;
    }

    const int usagePercent =
            nextTotal > 0
                    ? std::clamp(static_cast<int>((static_cast<double>(nextUsed) /
                                                                                 static_cast<double>(nextTotal)) *
                                                                                100.0),
                                             0, 100)
                    : 0;

    if (m_gpuName != nextName) {
        m_gpuName = nextName;
        emit gpuNameChanged();
    }

    if (m_temperatureC != nextTemp) {
        m_temperatureC = nextTemp;
        emit temperatureCChanged();
    }

    if (m_utilizationPercent != nextUtil) {
        m_utilizationPercent = std::clamp(nextUtil, 0, 100);
        emit utilizationPercentChanged();
    }

    if (m_memoryUsedMiB != nextUsed) {
        m_memoryUsedMiB = nextUsed;
        emit memoryUsedMiBChanged();
    }

    if (m_memoryTotalMiB != nextTotal) {
        m_memoryTotalMiB = nextTotal;
        emit memoryTotalMiBChanged();
    }

    if (m_memoryUsagePercent != usagePercent) {
        m_memoryUsagePercent = usagePercent;
        emit memoryUsagePercentChanged();
    }

    setAvailable(true);
}

void GpuMonitor::start() {
    if (m_timer.isActive()) {
        return;
    }

    m_timer.start();
    emit runningChanged();
}

void GpuMonitor::stop() {
    if (!m_timer.isActive()) {
        return;
    }

    m_timer.stop();
    emit runningChanged();
}

void GpuMonitor::setUpdateInterval(int intervalMs) {
    if (intervalMs < 250 || m_timer.interval() == intervalMs) {
        return;
    }

    m_timer.setInterval(intervalMs);
    emit updateIntervalChanged();
}

void GpuMonitor::clearMetrics() {
    if (!m_gpuName.isEmpty()) {
        m_gpuName.clear();
        emit gpuNameChanged();
    }

    if (m_temperatureC != 0) {
        m_temperatureC = 0;
        emit temperatureCChanged();
    }

    if (m_utilizationPercent != 0) {
        m_utilizationPercent = 0;
        emit utilizationPercentChanged();
    }

    if (m_memoryUsedMiB != 0) {
        m_memoryUsedMiB = 0;
        emit memoryUsedMiBChanged();
    }

    if (m_memoryTotalMiB != 0) {
        m_memoryTotalMiB = 0;
        emit memoryTotalMiBChanged();
    }

    if (m_memoryUsagePercent != 0) {
        m_memoryUsagePercent = 0;
        emit memoryUsagePercentChanged();
    }
}

void GpuMonitor::setAvailable(bool value) {
    if (m_available == value) {
        return;
    }

    m_available = value;
    emit availableChanged();
}
