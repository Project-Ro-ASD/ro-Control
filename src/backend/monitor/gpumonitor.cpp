#include "gpumonitor.h"
#include "system/commandrunner.h"

GpuMonitor::GpuMonitor(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &GpuMonitor::poll);
}

void GpuMonitor::start(int intervalMs)
{
    poll(); // İlk veriyi hemen al
    m_timer.start(intervalMs);
}

void GpuMonitor::stop()
{
    m_timer.stop();
}

void GpuMonitor::poll()
{
    CommandRunner runner;

    // nvidia-smi'den tek seferde tüm değerleri çek
    // Çıktı formatı: "temperature,utilization,vram_used,vram_total"
    const auto result = runner.run(
        QStringLiteral("nvidia-smi"),
        {
            QStringLiteral("--query-gpu=temperature.gpu,utilization.gpu,memory.used,memory.total"),
            QStringLiteral("--format=csv,noheader,nounits")
        }
    );

    if (!result.success()) {
        if (m_available) {
            m_available = false;
            emit availableChanged();
        }
        return;
    }

    // "72, 45, 2048, 8192" formatını parse et
    const QStringList parts = result.stdout.trimmed().split(QStringLiteral(", "));
    if (parts.size() < 4)
        return;

    bool ok = false;

    const int temp = parts[0].trimmed().toInt(&ok);
    if (ok && temp != m_temperature) {
        m_temperature = temp;
        emit temperatureChanged();
    }

    const int load = parts[1].trimmed().toInt(&ok);
    if (ok && load != m_load) {
        m_load = load;
        emit loadChanged();
    }

    const int vramUsed = parts[2].trimmed().toInt(&ok);
    if (ok && vramUsed != m_vramUsed) {
        m_vramUsed = vramUsed;
        emit vramUsedChanged();
    }

    const int vramTotal = parts[3].trimmed().toInt(&ok);
    if (ok && vramTotal != m_vramTotal) {
        m_vramTotal = vramTotal;
        emit vramTotalChanged();
    }

    if (!m_available) {
        m_available = true;
        emit availableChanged();
    }
}
