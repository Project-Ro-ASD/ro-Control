#include "gpumonitor.h"
#include "gpufallbackreader.h"
#include "gpuprocessinventory.h"
#include "nvidia/detector.h"
#include "system/commandrunner.h"

#include <QFutureWatcher>
#include <QRegularExpression>
#include <QSettings>
#include <QtConcurrent>
#include <algorithm>

namespace {

struct AsyncRefreshPayload {
  CommandRunner::Result telemetryResult;
  bool hasDevices = false;
  QVariantList devices;
  bool hasProcesses = false;
  QVariantList processes;
};

QString normalizedMetricField(const QString &field) {
  static const QRegularExpression bracketRegex(
      QStringLiteral(R"(\s*\[[^\]]+\]\s*)"));
  static const QRegularExpression percentRegex(QStringLiteral(R"(\s*%\s*)"));
  static const QRegularExpression wattRegex(
      QStringLiteral(R"(\s*w\b)"), QRegularExpression::CaseInsensitiveOption);
  static const QRegularExpression mhzRegex(
      QStringLiteral(R"(\s*mhz\b)"), QRegularExpression::CaseInsensitiveOption);

  QString normalized = field.trimmed();
  normalized.remove(bracketRegex);
  normalized.remove(percentRegex);
  normalized.remove(wattRegex);
  normalized.remove(mhzRegex);
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

bool parseMetricDouble(const QString &field, double *value) {
  if (value == nullptr) {
    return false;
  }

  static const QRegularExpression wattRegex(
      QStringLiteral(R"(\s*w\b)"), QRegularExpression::CaseInsensitiveOption);
  static const QRegularExpression mhzRegex(
      QStringLiteral(R"(\s*mhz\b)"), QRegularExpression::CaseInsensitiveOption);

  QString normalized = normalizedMetricField(field);
  normalized.remove(wattRegex);
  normalized.remove(mhzRegex);
  if (normalized.isEmpty() ||
      normalized.compare(QStringLiteral("n/a"), Qt::CaseInsensitive) == 0 ||
      normalized.compare(QStringLiteral("not supported"),
                         Qt::CaseInsensitive) == 0 ||
      normalized.compare(QStringLiteral("unknown"), Qt::CaseInsensitive) == 0) {
    return false;
  }

  bool ok = false;
  const double parsedValue = normalized.toDouble(&ok);
  if (!ok) {
    return false;
  }

  *value = parsedValue;
  return true;
}

CommandRunner::Result fetchNvidiaSmiTelemetryCsv(int gpuIndex) {
  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 1500;

  QStringList queryArgs = {
      QStringLiteral(
          "--query-gpu=name,temperature.gpu,utilization.gpu,memory.used,"
          "memory.total,fan.speed,power.draw,power.limit,"
          "clocks.current.graphics,clocks.current.memory,"
          "pcie.link.gen.current,pcie.link.gen.max,"
          "pcie.link.width.current,pcie.link.width.max,"
          "temperature.gpu.tlimit"),
      QStringLiteral("--format=csv,noheader,nounits")};

  if (gpuIndex > 0) {
    queryArgs.prepend(QStringLiteral("--id=%1").arg(gpuIndex));
  }

  return runner.run(QStringLiteral("nvidia-smi"), queryArgs, options);
}

} // namespace

GpuMonitor::GpuMonitor(QObject *parent) : QObject(parent) {
  QSettings settings;
  m_selectedGpuIndex =
      settings.value(QStringLiteral("gpu/selectedIndex"), 0).toInt();
  if (m_selectedGpuIndex < 0) {
    m_selectedGpuIndex = 0;
  }

  m_timer.setInterval(1000);
  m_timer.setTimerType(Qt::CoarseTimer);
  connect(&m_timer, &QTimer::timeout, this, &GpuMonitor::refreshAsync);

  start();
  // Defer the first driver query until the event loop is running. Constructing
  // a QML-facing monitor must never synchronously start nvidia-smi.
  QTimer::singleShot(0, this, &GpuMonitor::refreshAsync);
}

bool GpuMonitor::available() const { return m_available; }

bool GpuMonitor::running() const { return m_timer.isActive(); }

bool GpuMonitor::refreshInProgress() const { return m_asyncRefreshInFlight; }

QString GpuMonitor::gpuName() const {
  return NvidiaDetector::localizeGpuName(m_gpuName);
}

int GpuMonitor::temperatureC() const { return m_temperatureC; }

int GpuMonitor::hotspotTemperatureC() const { return m_hotspotTemperatureC; }

int GpuMonitor::memoryTemperatureC() const { return m_memoryTemperatureC; }

int GpuMonitor::utilizationPercent() const { return m_utilizationPercent; }

int GpuMonitor::memoryUsedMiB() const { return m_memoryUsedMiB; }

int GpuMonitor::memoryTotalMiB() const { return m_memoryTotalMiB; }

int GpuMonitor::memoryUsagePercent() const { return m_memoryUsagePercent; }

int GpuMonitor::fanSpeedPercent() const { return m_fanSpeedPercent; }

int GpuMonitor::thermalLimitTemperatureC() const { return m_thermalLimitC; }

double GpuMonitor::powerDrawW() const { return m_powerDrawW; }

double GpuMonitor::powerLimitW() const { return m_powerLimitW; }

int GpuMonitor::graphicsClockMHz() const { return m_graphicsClockMHz; }

int GpuMonitor::memoryClockMHz() const { return m_memoryClockMHz; }

QString GpuMonitor::pcieLinkStatus() const { return m_pcieLinkStatus; }

QVariantList GpuMonitor::gpuProcesses() const { return m_gpuProcesses; }

int GpuMonitor::gpuProcessCount() const {
  return static_cast<int>(m_gpuProcesses.size());
}

int GpuMonitor::gpuCount() const {
  return std::max(1, static_cast<int>(m_gpuDevices.size()));
}

int GpuMonitor::selectedGpuIndex() const { return m_selectedGpuIndex; }

QVariantList GpuMonitor::gpuDevices() const { return m_gpuDevices; }

void GpuMonitor::setSelectedGpuIndex(int index) {
  if (index < 0 || m_selectedGpuIndex == index) {
    return;
  }

  m_selectedGpuIndex = index;
  QSettings settings;
  settings.setValue(QStringLiteral("gpu/selectedIndex"), index);
  emit selectedGpuIndexChanged();
  queryGpuDevices(true);
  queryGpuProcesses(true);
  refreshAsync();
}

QString GpuMonitor::statusMessage() const { return m_statusMessage; }

int GpuMonitor::updateInterval() const { return m_timer.interval(); }

void GpuMonitor::refresh() {
  ++m_refreshTickCount;
  queryGpuDevices(false);
  queryGpuProcesses(false);
  processRefreshResult(fetchNvidiaSmiTelemetryCsv(m_selectedGpuIndex));
}

void GpuMonitor::requestRefresh() { refreshAsync(); }

void GpuMonitor::refreshAsync() {
  if (m_asyncRefreshInFlight) {
    // Retain a single follow-up update without starting concurrent driver
    // processes for repeated UI clicks or timer ticks.
    m_refreshQueued = true;
    return;
  }
  m_asyncRefreshInFlight = true;
  emit refreshInProgressChanged();

  const int gpuIndex = m_selectedGpuIndex;
  const quint64 tick = ++m_refreshTickCount;
  const bool queryDevicesNeeded = m_gpuDevices.isEmpty() || (tick % 60 == 1);
  const bool queryProcessesNeeded = m_gpuProcesses.isEmpty() || (tick % 4 == 1);
  const QString currentGpuName = m_gpuName;

  auto *watcher = new QFutureWatcher<AsyncRefreshPayload>(this);
  watcher->setFuture(QtConcurrent::run(
      [gpuIndex, queryDevicesNeeded, queryProcessesNeeded, currentGpuName] {
        AsyncRefreshPayload payload;
        payload.telemetryResult = fetchNvidiaSmiTelemetryCsv(gpuIndex);
        if (queryDevicesNeeded) {
          payload.devices =
              GpuProcessInventory::queryGpuDevices(gpuIndex, currentGpuName);
          payload.hasDevices = true;
        }
        if (queryProcessesNeeded) {
          payload.processes = GpuProcessInventory::queryGpuProcesses(gpuIndex);
          payload.hasProcesses = true;
        }
        return payload;
      }));

  connect(watcher, &QFutureWatcher<AsyncRefreshPayload>::finished, this,
          [this, watcher]() {
            const AsyncRefreshPayload payload = watcher->result();
            m_asyncRefreshInFlight = false;
            emit refreshInProgressChanged();

            if (payload.hasDevices && m_gpuDevices != payload.devices) {
              m_gpuDevices = payload.devices;
              emit gpuDevicesChanged();
            }

            if (payload.hasProcesses && m_gpuProcesses != payload.processes) {
              m_gpuProcesses = payload.processes;
              emit gpuProcessesChanged();
            }

            processRefreshResult(payload.telemetryResult);
            emit telemetryRefreshFinished();
            watcher->deleteLater();

            if (m_refreshQueued) {
              m_refreshQueued = false;
              QTimer::singleShot(0, this, &GpuMonitor::refreshAsync);
            }
          });
}

void GpuMonitor::processRefreshResult(const CommandRunner::Result &result) {
  CommandRunner runner;

  if (!result.success()) {
    int nextTemp = 0;
    int nextUtil = 0;
    int nextUsed = 0;
    int nextTotal = 0;

    const bool hasGenericMetrics =
        GpuFallbackReader::readGenericLinuxGpuMetrics(&nextTemp, &nextUtil,
                                                      &nextUsed, &nextTotal);
    const bool hasTemperatureFallback =
        nextTemp > 0 ||
        GpuFallbackReader::readNvidiaTemperatureFallback(runner, &nextTemp);

    if (!hasGenericMetrics && !hasTemperatureFallback) {
      setAvailable(false);
      if (!GpuFallbackReader::hasNvidiaPciDevice()) {
        setStatusMessage(tr("No NVIDIA GPU detected in this session."));
      } else {
        setStatusMessage(
            tr("NVIDIA driver is not exposing telemetry on this system."));
      }
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

    if (m_gpuName.isEmpty()) {
      const QString detectedName = NvidiaDetector::detectGpuNameFromProc();
      if (!detectedName.isEmpty() && m_gpuName != detectedName) {
        m_gpuName = detectedName;
        emit gpuNameChanged();
      }
    }

    int nextHotspot = 0;
    int nextMemTemp = 0;
    GpuFallbackReader::readNvidiaHotspotAndMemoryTemps(runner, &nextHotspot,
                                                       &nextMemTemp);
    if (nextTemp > 0) {
      if (nextHotspot <= 0 || nextHotspot < nextTemp) {
        const int loadOffset = (std::clamp(nextUtil, 0, 100) * 10) / 100;
        nextHotspot = nextTemp + 5 + std::min(10, loadOffset);
      }
    }
    if (m_hotspotTemperatureC != nextHotspot) {
      m_hotspotTemperatureC = nextHotspot;
      emit hotspotTemperatureCChanged();
    }
    if (m_memoryTemperatureC != nextMemTemp) {
      m_memoryTemperatureC = nextMemTemp;
      emit memoryTemperatureCChanged();
    }

    setAvailable(true);
    setStatusMessage(
        hasGenericMetrics
            ? tr("GPU telemetry is being read from Linux metrics.")
            : tr("GPU temperature is being read from system sensors."));
    return;
  }

  const QString stdoutText = result.stdout;
  const QString firstLine = stdoutText.split('\n', Qt::SkipEmptyParts).value(0);
  const QStringList fields = firstLine.split(',', Qt::KeepEmptyParts);

  if (fields.size() < 5) {
    setAvailable(false);
    setStatusMessage(tr("Failed to parse nvidia-smi telemetry output."));
    clearMetrics();
    return;
  }

  const QString rawGpuName = fields.value(0).trimmed();
  const QString name =
      NvidiaDetector::cleanGpuName(rawGpuName, QStringLiteral("NVIDIA"));

  int nextTemp = 0;
  int nextUtil = 0;
  int nextUsed = 0;
  int nextTotal = 0;
  int nextFan = 0;
  double nextDraw = 0.0;
  double nextLimit = 0.0;
  int nextGfxClock = 0;
  int nextMemClock = 0;
  QString nextPcie;
  int nextTLimit = 0;

  const bool hasTemp = parseMetricInt(fields.value(1), &nextTemp);
  const bool hasUtil = parseMetricInt(fields.value(2), &nextUtil);
  const bool hasUsed = parseMetricInt(fields.value(3), &nextUsed);
  const bool hasTotal = parseMetricInt(fields.value(4), &nextTotal);
  const bool hasFan = parseMetricInt(fields.value(5), &nextFan);
  const bool hasDraw = parseMetricDouble(fields.value(6), &nextDraw);
  const bool hasLimit = parseMetricDouble(fields.value(7), &nextLimit);
  const bool hasGfxClock = parseMetricInt(fields.value(8), &nextGfxClock);
  const bool hasMemClock = parseMetricInt(fields.value(9), &nextMemClock);

  if (fields.size() >= 14) {
    QString pcieGenCurr = normalizedMetricField(fields.value(10));
    QString pcieGenMax = normalizedMetricField(fields.value(11));
    QString pcieWidthCurr = normalizedMetricField(fields.value(12));
    QString pcieWidthMax = normalizedMetricField(fields.value(13));

    if (!pcieGenCurr.isEmpty() && !pcieGenMax.isEmpty() &&
        !pcieWidthCurr.isEmpty() && !pcieWidthMax.isEmpty()) {
      nextPcie = QStringLiteral("PCIe Gen%1 x%2 (Max: Gen%3 x%4)")
                     .arg(pcieGenCurr, pcieWidthCurr, pcieGenMax, pcieWidthMax);
    }
  }

  if (fields.size() >= 15) {
    parseMetricInt(fields.value(14), &nextTLimit);
  }

  const int nextUsagePercent =
      (hasUsed && hasTotal && nextTotal > 0)
          ? std::clamp(static_cast<int>((static_cast<double>(nextUsed) /
                                         static_cast<double>(nextTotal)) *
                                        100.0),
                       0, 100)
          : 0;

  if (m_gpuName != name) {
    m_gpuName = name;
    emit gpuNameChanged();
  }

  if (hasTemp && m_temperatureC != nextTemp) {
    m_temperatureC = nextTemp;
    emit temperatureCChanged();
  }

  if (hasUtil && m_utilizationPercent != nextUtil) {
    m_utilizationPercent = nextUtil;
    emit utilizationPercentChanged();
  }

  if (hasUsed && m_memoryUsedMiB != nextUsed) {
    m_memoryUsedMiB = nextUsed;
    emit memoryUsedMiBChanged();
  }

  if (hasTotal && m_memoryTotalMiB != nextTotal) {
    m_memoryTotalMiB = nextTotal;
    emit memoryTotalMiBChanged();
  }

  if (m_memoryUsagePercent != nextUsagePercent) {
    m_memoryUsagePercent = nextUsagePercent;
    emit memoryUsagePercentChanged();
  }

  if (hasFan && m_fanSpeedPercent != nextFan) {
    m_fanSpeedPercent = nextFan;
    emit fanSpeedPercentChanged();
  }

  if (nextTLimit > 0 && m_thermalLimitC != nextTLimit) {
    m_thermalLimitC = nextTLimit;
    emit thermalLimitTemperatureCChanged();
  }

  if (hasDraw && !qFuzzyCompare(m_powerDrawW, nextDraw)) {
    m_powerDrawW = nextDraw;
    emit powerDrawWChanged();
  }

  if (hasLimit && !qFuzzyCompare(m_powerLimitW, nextLimit)) {
    m_powerLimitW = nextLimit;
    emit powerLimitWChanged();
  }

  if (hasGfxClock && m_graphicsClockMHz != nextGfxClock) {
    m_graphicsClockMHz = nextGfxClock;
    emit graphicsClockMHzChanged();
  }

  if (hasMemClock && m_memoryClockMHz != nextMemClock) {
    m_memoryClockMHz = nextMemClock;
    emit memoryClockMHzChanged();
  }

  if (!nextPcie.isEmpty() && m_pcieLinkStatus != nextPcie) {
    m_pcieLinkStatus = nextPcie;
    emit pcieLinkStatusChanged();
  }

  int nextHotspot = 0;
  int nextMemTemp = 0;
  GpuFallbackReader::readNvidiaHotspotAndMemoryTemps(runner, &nextHotspot,
                                                     &nextMemTemp);
  if (m_temperatureC > 0) {
    if (nextHotspot <= 0 || nextHotspot < m_temperatureC) {
      const int loadOffset =
          (std::clamp(m_utilizationPercent, 0, 100) * 10) / 100;
      nextHotspot = m_temperatureC + 5 + std::min(10, loadOffset);
    }
  }

  if (m_hotspotTemperatureC != nextHotspot) {
    m_hotspotTemperatureC = nextHotspot;
    emit hotspotTemperatureCChanged();
  }

  if (m_memoryTemperatureC != nextMemTemp) {
    m_memoryTemperatureC = nextMemTemp;
    emit memoryTemperatureCChanged();
  }

  setAvailable(true);
  setStatusMessage(tr("GPU telemetry is being read from nvidia-smi."));
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

  if (m_hotspotTemperatureC != 0) {
    m_hotspotTemperatureC = 0;
    emit hotspotTemperatureCChanged();
  }

  if (m_memoryTemperatureC != 0) {
    m_memoryTemperatureC = 0;
    emit memoryTemperatureCChanged();
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

  if (m_fanSpeedPercent != 0) {
    m_fanSpeedPercent = 0;
    emit fanSpeedPercentChanged();
  }

  if (m_thermalLimitC != 0) {
    m_thermalLimitC = 0;
    emit thermalLimitTemperatureCChanged();
  }

  if (!qFuzzyCompare(m_powerDrawW, 0.0)) {
    m_powerDrawW = 0.0;
    emit powerDrawWChanged();
  }

  if (!qFuzzyCompare(m_powerLimitW, 0.0)) {
    m_powerLimitW = 0.0;
    emit powerLimitWChanged();
  }

  if (m_graphicsClockMHz != 0) {
    m_graphicsClockMHz = 0;
    emit graphicsClockMHzChanged();
  }

  if (m_memoryClockMHz != 0) {
    m_memoryClockMHz = 0;
    emit memoryClockMHzChanged();
  }

  if (!m_pcieLinkStatus.isEmpty()) {
    m_pcieLinkStatus.clear();
    emit pcieLinkStatusChanged();
  }

  if (!m_gpuProcesses.isEmpty()) {
    m_gpuProcesses.clear();
    emit gpuProcessesChanged();
  }
}

bool GpuMonitor::killProcess(int pid) {
  if (pid <= 1) {
    return false;
  }
  const bool isListedGpuProcess = std::any_of(
      m_gpuProcesses.cbegin(), m_gpuProcesses.cend(), [pid](const QVariant &v) {
        return v.toMap().value(QStringLiteral("pid")).toInt() == pid;
      });
  if (!isListedGpuProcess) {
    setStatusMessage(
        tr("The selected process is no longer using the active GPU."));
    return false;
  }

  QString err;
  const bool ok = GpuProcessInventory::killProcess(pid, m_gpuProcesses, &err);
  if (!ok) {
    setStatusMessage(tr("The process could not be terminated. Check ownership "
                        "and permissions."));
    return false;
  }
  queryGpuProcesses(true);
  setStatusMessage(tr("Termination signal sent to the selected GPU process."));
  return true;
}

void GpuMonitor::queryGpuProcesses(bool force) {
  if (!force && !m_gpuProcesses.isEmpty() && (m_refreshTickCount % 4 != 1)) {
    return;
  }

  const auto processes =
      GpuProcessInventory::queryGpuProcesses(m_selectedGpuIndex);
  if (m_gpuProcesses != processes) {
    m_gpuProcesses = processes;
    emit gpuProcessesChanged();
  }
}

void GpuMonitor::queryGpuDevices(bool force) {
  if (!force && !m_gpuDevices.isEmpty() && (m_refreshTickCount % 60 != 1)) {
    return;
  }

  const auto devices =
      GpuProcessInventory::queryGpuDevices(m_selectedGpuIndex, m_gpuName);
  if (m_gpuDevices != devices) {
    m_gpuDevices = devices;
    emit gpuDevicesChanged();
  }
}

void GpuMonitor::setAvailable(bool value) {
  if (m_available == value) {
    return;
  }

  m_available = value;
  emit availableChanged();
}

void GpuMonitor::setStatusMessage(const QString &value) {
  if (m_statusMessage == value) {
    return;
  }

  m_statusMessage = value;
  emit statusMessageChanged();
}
