#include "gpumonitor.h"
#include "system/commandrunner.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <algorithm>

namespace {

QString normalizedMetricField(const QString &field) {
  QString normalized = field.trimmed();
  normalized.remove(QRegularExpression(QStringLiteral(R"(\s*\[[^\]]+\]\s*)")));
  normalized.remove(QRegularExpression(QStringLiteral(R"(\s*%\s*)")));
  return normalized.trimmed();
}

bool parseMetricInt(const QString &field, int *value) {
  if (value == nullptr) {
    return false;
  }

  const QString normalized = normalizedMetricField(field);
  if (normalized.isEmpty() ||
      normalized.compare(QStringLiteral("n/a"), Qt::CaseInsensitive) == 0 ||
      normalized.compare(QStringLiteral("not supported"),
                         Qt::CaseInsensitive) == 0 ||
      normalized.compare(QStringLiteral("unknown"), Qt::CaseInsensitive) == 0) {
    return false;
  }

  bool ok = false;
  const int parsedValue = normalized.toInt(&ok);
  if (!ok) {
    return false;
  }

  *value = parsedValue;
  return true;
}

QString drmRootPath() {
  const QString overridePath =
      qEnvironmentVariable("RO_CONTROL_DRM_ROOT").trimmed();
  return overridePath.isEmpty() ? QStringLiteral("/sys/class/drm")
                                : overridePath;
}

QString readFileText(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }

  return QString::fromUtf8(file.readAll()).trimmed();
}

bool readIntegerFile(const QString &path, qint64 *value) {
  if (value == nullptr) {
    return false;
  }

  bool ok = false;
  const qint64 parsedValue = readFileText(path).toLongLong(&ok);
  if (!ok) {
    return false;
  }

  *value = parsedValue;
  return true;
}

bool readFirstTemperatureFromHwmon(const QString &basePath, int *value) {
  const QFileInfoList hwmonEntries = QDir(basePath).entryInfoList(
      {QStringLiteral("hwmon*")}, QDir::Dirs | QDir::NoDotAndDotDot,
      QDir::Name);
  for (const QFileInfo &entry : hwmonEntries) {
    const QFileInfoList inputs =
        QDir(entry.absoluteFilePath())
            .entryInfoList({QStringLiteral("temp*_input")}, QDir::Files,
                           QDir::Name);
    for (const QFileInfo &input : inputs) {
      qint64 milliC = 0;
      if (readIntegerFile(input.absoluteFilePath(), &milliC) && milliC > 0) {
        *value = static_cast<int>(milliC / 1000);
        return true;
      }
    }
  }

  return false;
}

bool readGenericLinuxGpuMetrics(int *temperatureC, int *utilizationPercent,
                                int *memoryUsedMiB, int *memoryTotalMiB) {
  const QFileInfoList cardEntries =
      QDir(drmRootPath())
          .entryInfoList({QStringLiteral("card*")},
                         QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

  bool anyMetric = false;
  for (const QFileInfo &cardEntry : cardEntries) {
    const QString devicePath =
        cardEntry.absoluteFilePath() + QStringLiteral("/device");
    if (!QFile::exists(devicePath)) {
      continue;
    }

    int tempValue = 0;
    if (temperatureC != nullptr &&
        readFirstTemperatureFromHwmon(devicePath, &tempValue)) {
      *temperatureC = tempValue;
      anyMetric = true;
    }

    qint64 busyPercent = 0;
    if (utilizationPercent != nullptr &&
        readIntegerFile(devicePath + QStringLiteral("/gpu_busy_percent"),
                        &busyPercent)) {
      *utilizationPercent = std::clamp(static_cast<int>(busyPercent), 0, 100);
      anyMetric = true;
    }

    qint64 usedBytes = 0;
    qint64 totalBytes = 0;
    const bool usedOk = readIntegerFile(
        devicePath + QStringLiteral("/mem_info_vram_used"), &usedBytes);
    const bool totalOk = readIntegerFile(
        devicePath + QStringLiteral("/mem_info_vram_total"), &totalBytes);
    if (usedOk && totalOk && totalBytes > 0) {
      if (memoryUsedMiB != nullptr) {
        *memoryUsedMiB =
            std::max(0, static_cast<int>(static_cast<qint64>(usedBytes) /
                                         (1024 * 1024)));
      }
      if (memoryTotalMiB != nullptr) {
        *memoryTotalMiB =
            std::max(0, static_cast<int>(static_cast<qint64>(totalBytes) /
                                         (1024 * 1024)));
      }
      anyMetric = true;
    }

    if (anyMetric) {
      return true;
    }
  }

  return false;
}

} // namespace

GpuMonitor::GpuMonitor(QObject *parent) : QObject(parent) {
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
  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 1500;

  const auto result = runner.run(
      QStringLiteral("nvidia-smi"),
      {QStringLiteral(
           "--query-gpu=name,temperature.gpu,utilization.gpu,memory.used,"
           "memory.total"),
       QStringLiteral("--format=csv,noheader,nounits")},
      options);

  if (!result.success()) {
    int nextTemp = 0;
    int nextUtil = 0;
    int nextUsed = 0;
    int nextTotal = 0;

    if (!readGenericLinuxGpuMetrics(&nextTemp, &nextUtil, &nextUsed,
                                    &nextTotal)) {
      setAvailable(false);
      clearMetrics();
      return;
    }

    if (m_temperatureC != nextTemp) {
      m_temperatureC = nextTemp;
      emit temperatureCChanged();
    }
    if (m_utilizationPercent != nextUtil) {
      m_utilizationPercent = nextUtil;
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

    const int usagePercent =
        nextTotal > 0
            ? std::clamp(static_cast<int>((static_cast<double>(nextUsed) /
                                           static_cast<double>(nextTotal)) *
                                          100.0),
                         0, 100)
            : 0;
    if (m_memoryUsagePercent != usagePercent) {
      m_memoryUsagePercent = usagePercent;
      emit memoryUsagePercentChanged();
    }

    setAvailable(true);
    return;
  }

  const QString stdoutText = result.stdout;
  const QString firstLine = stdoutText.split('\n', Qt::SkipEmptyParts).value(0);
  const QStringList fields = firstLine.split(',', Qt::KeepEmptyParts);

  if (fields.size() < 5) {
    setAvailable(false);
    clearMetrics();
    return;
  }

  const QString nextName = fields.at(0).trimmed();
  int nextTemp = 0;
  int nextUtil = 0;
  int nextUsed = 0;
  int nextTotal = 0;

  const bool tempAvailable = parseMetricInt(fields.at(1), &nextTemp);
  const bool utilAvailable = parseMetricInt(fields.at(2), &nextUtil);
  const bool usedAvailable = parseMetricInt(fields.at(3), &nextUsed);
  const bool totalAvailable = parseMetricInt(fields.at(4), &nextTotal);

  if (nextTotal < 0 || nextUsed < 0) {
    setAvailable(false);
    clearMetrics();
    return;
  }

  const int usagePercent =
      (usedAvailable && totalAvailable && nextTotal > 0)
          ? std::clamp(static_cast<int>((static_cast<double>(nextUsed) /
                                         static_cast<double>(nextTotal)) *
                                        100.0),
                       0, 100)
          : 0;

  const bool telemetryAvailable = !nextName.isEmpty() || tempAvailable ||
                                  utilAvailable || usedAvailable ||
                                  totalAvailable;
  if (!telemetryAvailable) {
    setAvailable(false);
    clearMetrics();
    return;
  }

  if (m_gpuName != nextName) {
    m_gpuName = nextName;
    emit gpuNameChanged();
  }

  if (m_temperatureC != nextTemp) {
    m_temperatureC = nextTemp;
    emit temperatureCChanged();
  }

  if (m_utilizationPercent != std::clamp(nextUtil, 0, 100)) {
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
