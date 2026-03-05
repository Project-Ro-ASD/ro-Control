#include "cpumonitor.h"

#include <QFile>
#include <QTextStream>
#include <QDir>

CpuMonitor::CpuMonitor(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &CpuMonitor::poll);
}

void CpuMonitor::start(int intervalMs)
{
    poll();
    m_timer.start(intervalMs);
}

void CpuMonitor::stop()
{
    m_timer.stop();
}

void CpuMonitor::poll()
{
    // ── CPU Yükü (/proc/stat) ────────────────────────────────────────────────
    // /proc/stat ilk satırı: "cpu  user nice system idle iowait irq softirq..."
    QFile statFile(QStringLiteral("/proc/stat"));
    if (statFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&statFile);
        const QString line = stream.readLine(); // "cpu ..." satırı

        const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() >= 5) {
            // idle = index 4, total = tüm değerlerin toplamı
            long long idle  = parts[4].toLongLong();
            long long total = 0;
            for (int i = 1; i < parts.size(); ++i)
                total += parts[i].toLongLong();

            // Delta ile yük hesapla
            const long long diffIdle  = idle  - m_prevIdle;
            const long long diffTotal = total - m_prevTotal;

            if (diffTotal > 0) {
                const int load = static_cast<int>(100 * (1.0 - static_cast<double>(diffIdle) / diffTotal));
                if (load != m_load) {
                    m_load = load;
                    emit loadChanged();
                }
            }

            m_prevIdle  = idle;
            m_prevTotal = total;
        }
    }

    // ── CPU Sıcaklığı (hwmon) ────────────────────────────────────────────────
    const int temp = readCpuTemp();
    if (temp > 0 && temp != m_temperature) {
        m_temperature = temp;
        emit temperatureChanged();
    }
}

int CpuMonitor::readCpuTemp() const
{
    // /sys/class/hwmon/ altındaki sıcaklık sensörlerini tara
    const QDir hwmonDir(QStringLiteral("/sys/class/hwmon"));
    const QStringList hwmons = hwmonDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &hwmon : hwmons) {
        const QString namePath = hwmonDir.filePath(hwmon + QStringLiteral("/name"));
        QFile nameFile(namePath);
        if (!nameFile.open(QIODevice::ReadOnly))
            continue;

        const QString name = QString::fromUtf8(nameFile.readAll()).trimmed();

        // k10temp (AMD) veya coretemp (Intel) sensörü
        if (name == QStringLiteral("k10temp") || name == QStringLiteral("coretemp")) {
            QFile tempFile(hwmonDir.filePath(hwmon + QStringLiteral("/temp1_input")));
            if (tempFile.open(QIODevice::ReadOnly)) {
                const int milliCelsius = QString::fromUtf8(tempFile.readAll()).trimmed().toInt();
                return milliCelsius / 1000; // milli-Celsius → Celsius
            }
        }
    }

    return 0;
}
