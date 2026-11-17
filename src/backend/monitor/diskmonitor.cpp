#include "diskmonitor.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <sys/statvfs.h>

#include <sys/statvfs.h>

namespace {

constexpr int kDefaultUpdateIntervalMs = 2000;
constexpr int kMinUpdateIntervalMs = 1000;

QString qEnvOrDefault(const char *name, const QString &fallback) {
  const QString v = qEnvironmentVariable(name).trimmed();
  return v.isEmpty() ? fallback : v;
}

} // namespace

DiskMonitor::DiskMonitor(QObject *parent) : QObject(parent) {
  m_timer.setInterval(kDefaultUpdateIntervalMs);
  m_timer.setTimerType(Qt::CoarseTimer);
  connect(&m_timer, &QTimer::timeout, this, &DiskMonitor::refresh);

  m_sampleTimer.start();
  refresh();
  start();
}

QString DiskMonitor::mountsPath() {
  return qEnvOrDefault("RO_CONTROL_MOUNTS_PATH",
                       QStringLiteral("/proc/mounts"));
}

QString DiskMonitor::diskstatsPath() {
  return qEnvOrDefault("RO_CONTROL_DISKSTATS_PATH",
                       QStringLiteral("/proc/diskstats"));
}

bool DiskMonitor::available() const {
  return m_available;
}

bool DiskMonitor::running() const {
  return m_timer.isActive();
}

int DiskMonitor::diskCount() const {
  return static_cast<int>(m_disks.size());
}

QVariantList DiskMonitor::diskStats() const {
  QVariantList list;
  list.reserve(m_disks.size());
  for (const auto &d : m_disks) {
    QVariantMap map;
    map.insert(QStringLiteral("device"), d.device);
    map.insert(QStringLiteral("mountPoint"), d.mountPoint);
    map.insert(QStringLiteral("fsType"), d.fsType);
    map.insert(QStringLiteral("totalBytes"),
               QVariant::fromValue<quint64>(d.totalBytes));
    map.insert(QStringLiteral("freeBytes"),
               QVariant::fromValue<quint64>(d.freeBytes));
    map.insert(QStringLiteral("usedBytes"),
               QVariant::fromValue<quint64>(d.usedBytes));
    map.insert(QStringLiteral("usagePercent"), d.usagePercent);
    map.insert(QStringLiteral("readBytes"),
               QVariant::fromValue<quint64>(d.readBytes));
    map.insert(QStringLiteral("writeBytes"),
               QVariant::fromValue<quint64>(d.writeBytes));
    map.insert(QStringLiteral("readBytesPerSec"),
               QVariant::fromValue<quint64>(d.readBytesPerSec));
    map.insert(QStringLiteral("writeBytesPerSec"),
               QVariant::fromValue<quint64>(d.writeBytesPerSec));
    list.append(map);
  }
  return list;
}

int DiskMonitor::updateInterval() const {
  return m_timer.interval();
}

void DiskMonitor::setUpdateInterval(int intervalMs) {
  if (intervalMs < kMinUpdateIntervalMs || m_timer.interval() == intervalMs) {
    return;
  }
  m_timer.setInterval(intervalMs);
  emit updateIntervalChanged();
}

QString DiskMonitor::statusMessage() const {
  return m_statusMessage;
}

void DiskMonitor::start() {
  if (m_timer.isActive()) {
    return;
  }
  m_timer.start();
  emit runningChanged();
}

void DiskMonitor::stop() {
  if (!m_timer.isActive()) {
    return;
  }
  m_timer.stop();
  emit runningChanged();
}

void DiskMonitor::refresh() {
  // Read /proc/mounts to discover real block-device mount points
  QFile mountsFile(mountsPath());
  if (!mountsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    setStatusMessage(QStringLiteral("Cannot read /proc/mounts"));
    m_disks.clear();
    setAvailable(false);
    emit diskStatsChanged();
    return;
  }

  QTextStream mstream(&mountsFile);
  QList<DiskStats> fresh;
  fresh.reserve(16);

  while (!mstream.atEnd()) {
    const QString line = mstream.readLine();
    if (line.isEmpty() || line.startsWith('#')) {
      continue;
    }

    const QStringList parts = line.split(' ');
    if (parts.size() < 3) {
      continue;
    }

    const QString device = parts.at(0);
    const QString mountPoint = parts.at(1);
    const QString fsType = parts.at(2);

    // Keep only real block devices (paths like /dev/sda1, /dev/nvme0n1p1)
    if (!device.startsWith(QLatin1String("/dev/"))) {
      continue;
    }
    if (mountPoint.startsWith(QLatin1String("/proc"))
        || mountPoint.startsWith(QLatin1String("/sys"))
        || mountPoint.startsWith(QLatin1String("/dev"))
        || mountPoint.startsWith(QLatin1String("/run"))
        || fsType.startsWith(QLatin1String("fuse"))
        || fsType == QLatin1String("autofs")) {
      continue;
    }

    DiskStats disk;
    disk.device = device;
    disk.mountPoint = mountPoint;
    disk.fsType = fsType;
    fresh.append(disk);
  }
  mountsFile.close();

  // statvfs for each mount point to get totals
  for (auto &disk : fresh) {
    struct statvfs st;
    if (statvfs(qPrintable(disk.mountPoint), &st) != 0) {
      disk.usagePercent = 0.0;
      continue;
    }
    const quint64 blockSize = static_cast<quint64>(st.f_frsize);
    disk.totalBytes = static_cast<quint64>(st.f_blocks) * blockSize;
    disk.freeBytes = static_cast<quint64>(st.f_bfree) * blockSize;
    disk.usedBytes = (disk.totalBytes > disk.freeBytes)
                         ? (disk.totalBytes - disk.freeBytes)
                         : 0;
    disk.usagePercent = (disk.totalBytes > 0)
                            ? (100.0 * static_cast<double>(disk.usedBytes)
                               / static_cast<double>(disk.totalBytes))
                            : 0.0;
  }

  // Parse /proc/diskstats for I/O counters
  // Format: major minor dev sectorsRead ... sectorsWritten ...
  QFile diskstatsFile(diskstatsPath());
  if (diskstatsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream ds(&diskstatsFile);

    QHash<QString, quint64> deviceSectorsRead;
    QHash<QString, quint64> deviceSectorsWritten;

    while (!ds.atEnd()) {
      const QString line = ds.readLine();
      if (line.isEmpty()) {
        continue;
      }

      const QStringList parts = line.split(' ', Qt::SkipEmptyParts);
      if (parts.size() < 14) {
        continue;
      }

      const QString deviceName = parts.at(2);   // e.g. "sda", "nvme0n1"
      bool ok = false;
      const quint64 sectorsRead = parts.at(5).toULongLong(&ok);
      if (!ok) {
        continue;
      }
      const quint64 sectorsWritten = parts.at(9).toULongLong(&ok);
      if (!ok) {
        continue;
      }

      deviceSectorsRead.insert(deviceName, sectorsRead);
      deviceSectorsWritten.insert(deviceName, sectorsWritten);
    }
    diskstatsFile.close();

    // Map disks to device names by stripping "/dev/" and partition suffixes
    // Simple heuristic: last path component, strip trailing digits for partition
    // numbers. E.g., /dev/sda1 -> sda, /dev/nvme0n1p1 -> nvme0n1
    const quint64 sampleMs =
        static_cast<quint64>(m_sampleTimer.elapsed());
    if (sampleMs > 0) {
      for (auto &disk : fresh) {
        const QString base = QFileInfo(disk.device).fileName();
        QString deviceName = base;
        // Strip trailing digits (partition number) and optional 'p' before them
        while (!deviceName.isEmpty()
               && (deviceName.back().isDigit()
                   || deviceName.back() == QLatin1Char('p'))) {
          deviceName.chop(1);
        }

        const quint64 currRead = deviceSectorsRead.value(deviceName, 0);
        const quint64 currWrite = deviceSectorsWritten.value(deviceName, 0);
        const quint64 prevRead = m_prevReadSectors.value(deviceName, 0);
        const quint64 prevWrite = m_prevWriteSectors.value(deviceName, 0);

        // 512 bytes per sector
        disk.readBytes = currRead * 512;
        disk.writeBytes = currWrite * 512;

        // Compute rates if we have a previous sample
        if (prevRead > 0 && currRead >= prevRead) {
          disk.readBytesPerSec =
              (currRead - prevRead) * 512 * 1000 / sampleMs;
        }
        if (prevWrite > 0 && currWrite >= prevWrite) {
          disk.writeBytesPerSec =
              (currWrite - prevWrite) * 512 * 1000 / sampleMs;
        }

        // update previous counters
        m_prevReadSectors.insert(deviceName, currRead);
        m_prevWriteSectors.insert(deviceName, currWrite);
      }
    }
  }

  m_disks = fresh;
  m_sampleTimer.restart();

  setAvailable(!m_disks.isEmpty());
  emit diskStatsChanged();
}

void DiskMonitor::setAvailable(bool value) {
  if (m_available == value) {
    return;
  }
  m_available = value;
  emit availableChanged();
}

void DiskMonitor::setStatusMessage(const QString &value) {
  if (m_statusMessage == value) {
    return;
  }
  m_statusMessage = value;
  emit statusMessageChanged();
}