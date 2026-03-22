#include "rammonitor.h"
#include "system/commandrunner.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

#include <algorithm>

namespace {

struct RamSnapshot {
  bool valid = false;
  int totalMiB = 0;
  int usedMiB = 0;
  int usagePercent = 0;
};

QString meminfoPath() {
  const QString overridePath =
      qEnvironmentVariable("RO_CONTROL_MEMINFO_PATH").trimmed();
  return overridePath.isEmpty() ? QStringLiteral("/proc/meminfo")
                                : overridePath;
}

RamSnapshot buildSnapshot(qint64 memTotalKiB, qint64 memAvailableKiB) {
  if (memTotalKiB <= 0 || memAvailableKiB < 0 ||
      memAvailableKiB > memTotalKiB) {
    return {};
  }

  const qint64 usedKiB = memTotalKiB - memAvailableKiB;
  RamSnapshot snapshot;
  snapshot.valid = true;
  snapshot.totalMiB = static_cast<int>(memTotalKiB / 1024);
  snapshot.usedMiB = static_cast<int>(usedKiB / 1024);
  snapshot.usagePercent =
      std::clamp(static_cast<int>((static_cast<double>(usedKiB) /
                                   static_cast<double>(memTotalKiB)) *
                                  100.0),
                 0, 100);
  return snapshot;
}

RamSnapshot readSnapshotFromFree() {
  CommandRunner runner;
  const auto result =
      runner.run(QStringLiteral("free"), {QStringLiteral("--mebi")});
  if (!result.success()) {
    return {};
  }

  const QStringList lines =
      result.stdout.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
  for (const QString &line : lines) {
    if (!line.startsWith(QStringLiteral("Mem:"))) {
      continue;
    }

    const QStringList fields = line.split(
        QRegularExpression(QStringLiteral(R"(\s+)")), Qt::SkipEmptyParts);
    if (fields.size() < 3) {
      return {};
    }

    bool totalOk = false;
    const int totalMiB = fields.value(1).toInt(&totalOk);
    if (!totalOk || totalMiB <= 0) {
      return {};
    }

    int usedMiB = 0;
    bool usedOk = false;
    if (fields.size() >= 7) {
      const int availableMiB = fields.value(6).toInt(&usedOk);
      if (usedOk) {
        usedMiB = std::clamp(totalMiB - availableMiB, 0, totalMiB);
      }
    }

    if (!usedOk) {
      usedMiB = fields.value(2).toInt(&usedOk);
      if (!usedOk) {
        return {};
      }
      usedMiB = std::clamp(usedMiB, 0, totalMiB);
    }

    RamSnapshot snapshot;
    snapshot.valid = true;
    snapshot.totalMiB = totalMiB;
    snapshot.usedMiB = usedMiB;
    snapshot.usagePercent =
        std::clamp(static_cast<int>((static_cast<double>(usedMiB) /
                                     static_cast<double>(totalMiB)) *
                                    100.0),
                   0, 100);
    return snapshot;
  }

  return {};
}

} // namespace

RamMonitor::RamMonitor(QObject *parent) : QObject(parent) {
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
  // TR: Linux RAM metrikleri /proc/meminfo uzerinden okunur.
  // EN: Linux memory metrics are read from /proc/meminfo.
  qint64 memTotalKiB = -1;
  qint64 memAvailableKiB = -1;
  qint64 memFreeKiB = -1;
  qint64 buffersKiB = -1;
  qint64 cachedKiB = -1;
  qint64 sReclaimableKiB = -1;
  qint64 shmemKiB = -1;

  QFile meminfo(meminfoPath());
  if (meminfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
    // TR: "Anahtar: deger" satirlarini guvenli sekilde ayriştir.
    QTextStream stream(&meminfo);
    while (!stream.atEnd()) {
      const QString line = stream.readLine().trimmed();
      if (line.isEmpty()) {
        continue;
      }

      const int colonIdx = line.indexOf(QLatin1Char(':'));
      if (colonIdx <= 0) {
        continue;
      }

      const QString key = line.left(colonIdx).trimmed();
      const QString valueStr = line.mid(colonIdx + 1).trimmed();

      // The value might be "19842104 kB". We just need the number.
      const int spaceIdx = valueStr.indexOf(QLatin1Char(' '));
      const QString numStr = spaceIdx > 0 ? valueStr.left(spaceIdx) : valueStr;

      bool ok = false;
      const qint64 value = numStr.toLongLong(&ok);
      if (!ok) {
        continue;
      }

      if (key == QStringLiteral("MemTotal")) {
        memTotalKiB = value;
      } else if (key == QStringLiteral("MemAvailable")) {
        memAvailableKiB = value;
      } else if (key == QStringLiteral("MemFree")) {
        memFreeKiB = value;
      } else if (key == QStringLiteral("Buffers")) {
        buffersKiB = value;
      } else if (key == QStringLiteral("Cached")) {
        cachedKiB = value;
      } else if (key == QStringLiteral("SReclaimable")) {
        sReclaimableKiB = value;
      } else if (key == QStringLiteral("Shmem")) {
        shmemKiB = value;
      }
    }
  }

  // TR: Bazi kernel/ortamlarda MemAvailable olmayabilir; yaklasik deger
  // hesapla. EN: Some kernels/environments do not expose MemAvailable; compute
  // a fallback.
  if (memAvailableKiB < 0 && memFreeKiB >= 0 && buffersKiB >= 0 &&
      cachedKiB >= 0) {
    const qint64 reclaimable = sReclaimableKiB > 0 ? sReclaimableKiB : 0;
    const qint64 shmem = shmemKiB > 0 ? shmemKiB : 0;
    memAvailableKiB = memFreeKiB + buffersKiB + cachedKiB + reclaimable - shmem;
  }

  // TR: Tutarsiz veri geldiyse metrikleri sifirla ve "unavailable" olarak isle.
  // EN: If metrics are inconsistent, clear values and mark monitor unavailable.
  if (memTotalKiB <= 0 || memAvailableKiB < 0 ||
      memAvailableKiB > memTotalKiB) {
    setAvailable(false);
    clearMetrics();
    return;
  }

  RamSnapshot snapshot = buildSnapshot(memTotalKiB, memAvailableKiB);
  if (!snapshot.valid) {
    snapshot = readSnapshotFromFree();
  }

  if (!snapshot.valid) {
    setAvailable(false);
    clearMetrics();
    return;
  }

  if (m_totalMiB != snapshot.totalMiB) {
    m_totalMiB = snapshot.totalMiB;
    emit totalMiBChanged();
  }

  if (m_usedMiB != snapshot.usedMiB) {
    m_usedMiB = snapshot.usedMiB;
    emit usedMiBChanged();
  }

  if (m_usagePercent != snapshot.usagePercent) {
    m_usagePercent = snapshot.usagePercent;
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
