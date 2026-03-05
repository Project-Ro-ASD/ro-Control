#include "rammonitor.h"

#include <QFile>
#include <QTextStream>

RamMonitor::RamMonitor(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &RamMonitor::poll);
}

void RamMonitor::start(int intervalMs)
{
    poll();
    m_timer.start(intervalMs);
}

void RamMonitor::stop()
{
    m_timer.stop();
}

void RamMonitor::poll()
{
    // /proc/meminfo formatı: "MemTotal:       16384000 kB"
    QFile memFile(QStringLiteral("/proc/meminfo"));
    if (!memFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    long long memTotal = 0, memAvailable = 0;

    QTextStream stream(&memFile);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (line.startsWith(QStringLiteral("MemTotal:")))
            memTotal     = line.split(QLatin1Char(' '), Qt::SkipEmptyParts)[1].toLongLong();
        else if (line.startsWith(QStringLiteral("MemAvailable:")))
            memAvailable = line.split(QLatin1Char(' '), Qt::SkipEmptyParts)[1].toLongLong();
    }

    if (memTotal <= 0)
        return;

    // kB → MB
    const int totalMb = static_cast<int>(memTotal / 1024);
    const int usedMb  = static_cast<int>((memTotal - memAvailable) / 1024);
    const int percent = static_cast<int>(100.0 * usedMb / totalMb);

    if (totalMb != m_totalMb) { m_totalMb = totalMb; emit totalMbChanged(); }
    if (usedMb  != m_usedMb)  { m_usedMb  = usedMb;  emit usedMbChanged();  }
    if (percent != m_usedPercent) { m_usedPercent = percent; emit usedPercentChanged(); }
}
