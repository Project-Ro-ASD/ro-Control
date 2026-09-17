#include "rammonitor.h"
#include "system/commandrunner.h"

#include <QDir>
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

QString swapsPath() {
  const QString overridePath =
      qEnvironmentVariable("RO_CONTROL_SWAPS_PATH").trimmed();
  return overridePath.isEmpty() ? QStringLiteral("/proc/swaps") : overridePath;
}

QString zramSysfsRoot() {
  const QString overridePath =
      qEnvironmentVariable("RO_CONTROL_ZRAM_SYSFS_ROOT").trimmed();
  return overridePath.isEmpty() ? QStringLiteral("/sys/block") : overridePath;
}

QString zswapEnabledPath() {
  const QString overridePath =
      qEnvironmentVariable("RO_CONTROL_ZSWAP_ENABLED_PATH").trimmed();
  return overridePath.isEmpty()
             ? QStringLiteral("/sys/module/zswap/parameters/enabled")
             : overridePath;
}

qint64 readIntegerFile(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return -1;

  bool ok = false;
  const qint64 value =
      QString::fromUtf8(file.readAll()).trimmed().toLongLong(&ok);
  return ok && value >= 0 ? value : -1;
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

    static const QRegularExpression spacesPattern(QStringLiteral(R"(\s+)"));
    const QStringList fields = line.split(spacesPattern, Qt::SkipEmptyParts);
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

bool RamMonitor::zramAvailable() const { return m_zramAvailable; }

int RamMonitor::zramTotalMiB() const { return m_zramTotalMiB; }

int RamMonitor::zramUsedMiB() const { return m_zramUsedMiB; }

int RamMonitor::zramPhysicalMiB() const { return m_zramPhysicalMiB; }

double RamMonitor::zramCompressionRatio() const {
  return m_zramCompressionRatio;
}

bool RamMonitor::zswapEnabled() const { return m_zswapEnabled; }

int RamMonitor::updateInterval() const { return m_timer.interval(); }

void RamMonitor::refresh() {
  // TR: Linux RAM metrikleri /proc/meminfo uzerinden okunur.
  // EN: Linux memory metrics are read from /proc/meminfo.
  refreshCompressionTelemetry();
  qint64 memTotalKiB = -1;
  qint64 memAvailableKiB = -1;
  qint64 memFreeKiB = -1;
  qint64 buffersKiB = -1;
  qint64 cachedKiB = -1;
  qint64 sReclaimableKiB = -1;
  qint64 shmemKiB = -1;

  QFile meminfo(meminfoPath());
  if (meminfo.open(QIODevice::ReadOnly)) {
    const QByteArray data = meminfo.readAll();
    const char *ptr = data.constData();
    const char *end = ptr + data.size();

    auto parseValue = [](const char *p, const char *lineEnd) -> qint64 {
      while (p < lineEnd && (*p == ' ' || *p == '\t' || *p == ':')) {
        ++p;
      }
      char *valEnd = nullptr;
      return std::strtoll(p, &valEnd, 10);
    };

    while (ptr < end) {
      const char *nextNewline =
          static_cast<const char *>(std::memchr(ptr, '\n', end - ptr));
      const char *lineEnd = nextNewline ? nextNewline : end;
      const size_t lineLen = lineEnd - ptr;

      if (lineLen > 9 && std::strncmp(ptr, "MemTotal:", 9) == 0) {
        memTotalKiB = parseValue(ptr + 9, lineEnd);
      } else if (lineLen > 13 && std::strncmp(ptr, "MemAvailable:", 13) == 0) {
        memAvailableKiB = parseValue(ptr + 13, lineEnd);
      } else if (lineLen > 8 && std::strncmp(ptr, "MemFree:", 8) == 0) {
        memFreeKiB = parseValue(ptr + 8, lineEnd);
      } else if (lineLen > 8 && std::strncmp(ptr, "Buffers:", 8) == 0) {
        buffersKiB = parseValue(ptr + 8, lineEnd);
      } else if (lineLen > 7 && std::strncmp(ptr, "Cached:", 7) == 0) {
        cachedKiB = parseValue(ptr + 7, lineEnd);
      } else if (lineLen > 13 && std::strncmp(ptr, "SReclaimable:", 13) == 0) {
        sReclaimableKiB = parseValue(ptr + 13, lineEnd);
      } else if (lineLen > 6 && std::strncmp(ptr, "Shmem:", 6) == 0) {
        shmemKiB = parseValue(ptr + 6, lineEnd);
      }

      ptr = lineEnd + (nextNewline ? 1 : 0);
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

void RamMonitor::refreshCompressionTelemetry() {
  qint64 zramTotalKiB = 0;
  qint64 zramUsedKiB = 0;
  QFile swaps(swapsPath());
  if (swaps.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream stream(&swaps);
    while (!stream.atEnd()) {
      const QStringList fields = stream.readLine().simplified().split(
          QLatin1Char(' '), Qt::SkipEmptyParts);
      if (fields.size() < 5 ||
          !fields.at(0).contains(QStringLiteral("zram"), Qt::CaseInsensitive)) {
        continue;
      }
      bool sizeOk = false;
      bool usedOk = false;
      const qint64 sizeKiB = fields.at(2).toLongLong(&sizeOk);
      const qint64 usedKiB = fields.at(3).toLongLong(&usedOk);
      if (sizeOk && sizeKiB >= 0)
        zramTotalKiB += sizeKiB;
      if (usedOk && usedKiB >= 0)
        zramUsedKiB += usedKiB;
    }
  }

  qint64 originalBytes = 0;
  qint64 physicalBytes = 0;
  const QDir blockRoot(zramSysfsRoot());
  const QStringList devices = blockRoot.entryList(
      {QStringLiteral("zram*")}, QDir::Dirs | QDir::NoDotAndDotDot);
  qint64 diskBytes = 0;
  for (const QString &device : devices) {
    const QString base = blockRoot.filePath(device);
    const qint64 deviceDiskBytes =
        readIntegerFile(base + QStringLiteral("/disksize"));
    if (deviceDiskBytes > 0)
      diskBytes += deviceDiskBytes;

    QFile stat(base + QStringLiteral("/mm_stat"));
    if (!stat.open(QIODevice::ReadOnly | QIODevice::Text))
      continue;
    const QStringList values = QString::fromUtf8(stat.readAll())
                                   .simplified()
                                   .split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (values.size() < 3)
      continue;
    bool originalOk = false;
    bool physicalOk = false;
    const qint64 deviceOriginal = values.at(0).toLongLong(&originalOk);
    const qint64 devicePhysical = values.at(2).toLongLong(&physicalOk);
    if (originalOk && deviceOriginal > 0)
      originalBytes += deviceOriginal;
    if (physicalOk && devicePhysical > 0)
      physicalBytes += devicePhysical;
  }

  if (zramTotalKiB == 0 && diskBytes > 0)
    zramTotalKiB = diskBytes / 1024;
  if (zramUsedKiB == 0 && originalBytes > 0)
    zramUsedKiB = originalBytes / 1024;

  const bool available = zramTotalKiB > 0 || !devices.isEmpty();
  const int totalMiB = static_cast<int>(zramTotalKiB / 1024);
  const int usedMiB = static_cast<int>(zramUsedKiB / 1024);
  const int physicalMiB = static_cast<int>(physicalBytes / (1024 * 1024));
  const double ratio = physicalBytes > 0 && originalBytes > 0
                           ? static_cast<double>(originalBytes) /
                                 static_cast<double>(physicalBytes)
                           : 0.0;
  if (m_zramAvailable != available || m_zramTotalMiB != totalMiB ||
      m_zramUsedMiB != usedMiB || m_zramPhysicalMiB != physicalMiB ||
      !qFuzzyCompare(m_zramCompressionRatio + 1.0, ratio + 1.0)) {
    m_zramAvailable = available;
    m_zramTotalMiB = totalMiB;
    m_zramUsedMiB = usedMiB;
    m_zramPhysicalMiB = physicalMiB;
    m_zramCompressionRatio = ratio;
    emit zramChanged();
  }

  QFile zswap(zswapEnabledPath());
  bool zswapEnabled = false;
  if (zswap.open(QIODevice::ReadOnly | QIODevice::Text)) {
    const QString value =
        QString::fromUtf8(zswap.readAll()).trimmed().toLower();
    zswapEnabled = value == QStringLiteral("y") ||
                   value == QStringLiteral("1") ||
                   value == QStringLiteral("enabled");
  }
  if (m_zswapEnabled != zswapEnabled) {
    m_zswapEnabled = zswapEnabled;
    emit zswapChanged();
  }
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
