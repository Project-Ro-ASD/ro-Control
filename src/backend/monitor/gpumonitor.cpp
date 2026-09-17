#include "gpumonitor.h"
#include "nvidia/detector.h"
#include "system/commandrunner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QRegularExpression>
#include <QtConcurrent>
#include <algorithm>

namespace {

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

bool isGpuHwmonName(const QString &name) {
  const QString lower = name.trimmed().toLower();
  return lower.contains(QStringLiteral("nvidia")) ||
         lower.contains(QStringLiteral("nouveau")) ||
         lower.contains(QStringLiteral("gpu"));
}

bool readNvidiaTemperatureFromHwmonClass(int *value) {
  static QString s_cachedGpuHwmonInput;
  static bool s_probed = false;

  if (s_probed && !s_cachedGpuHwmonInput.isEmpty()) {
    qint64 milliC = 0;
    if (readIntegerFile(s_cachedGpuHwmonInput, &milliC) && milliC > 0) {
      *value = static_cast<int>(milliC / 1000);
      return true;
    }
  }

  if (!s_probed) {
    s_probed = true;
    const QFileInfoList hwmonEntries =
        QDir(QStringLiteral("/sys/class/hwmon"))
            .entryInfoList({QStringLiteral("hwmon*")},
                           QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &entry : hwmonEntries) {
      if (!isGpuHwmonName(readFileText(entry.absoluteFilePath() +
                                       QStringLiteral("/name")))) {
        continue;
      }

      const QFileInfoList inputs =
          QDir(entry.absoluteFilePath())
              .entryInfoList({QStringLiteral("temp*_input")}, QDir::Files,
                             QDir::Name);
      for (const QFileInfo &input : inputs) {
        qint64 milliC = 0;
        if (readIntegerFile(input.absoluteFilePath(), &milliC) && milliC > 0) {
          s_cachedGpuHwmonInput = input.absoluteFilePath();
          *value = static_cast<int>(milliC / 1000);
          return true;
        }
      }
    }
  }

  return false;
}

bool readNvidiaTemperatureFromPciHwmon(int *value) {
  static QString s_cachedPciInput;
  static bool s_probed = false;

  if (s_probed && !s_cachedPciInput.isEmpty()) {
    qint64 milliC = 0;
    if (readIntegerFile(s_cachedPciInput, &milliC) && milliC > 0) {
      *value = static_cast<int>(milliC / 1000);
      return true;
    }
  }

  if (!s_probed) {
    s_probed = true;
    const QFileInfoList deviceEntries =
        QDir(QStringLiteral("/sys/bus/pci/devices"))
            .entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &deviceEntry : deviceEntries) {
      const QString devicePath = deviceEntry.absoluteFilePath();
      if (readFileText(devicePath + QStringLiteral("/vendor"))
              .compare(QStringLiteral("0x10de"), Qt::CaseInsensitive) != 0) {
        continue;
      }

      const QString deviceClass =
          readFileText(devicePath + QStringLiteral("/class")).toLower();
      if (!deviceClass.startsWith(QStringLiteral("0x03")) &&
          !deviceClass.startsWith(QStringLiteral("0x02"))) {
        continue;
      }

      const QFileInfoList hwmonEntries =
          QDir(devicePath)
              .entryInfoList({QStringLiteral("hwmon*")},
                             QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
      for (const QFileInfo &entry : hwmonEntries) {
        const QFileInfoList inputs =
            QDir(entry.absoluteFilePath())
                .entryInfoList({QStringLiteral("temp*_input")}, QDir::Files,
                               QDir::Name);
        for (const QFileInfo &input : inputs) {
          qint64 milliC = 0;
          if (readIntegerFile(input.absoluteFilePath(), &milliC) &&
              milliC > 0) {
            s_cachedPciInput = input.absoluteFilePath();
            *value = static_cast<int>(milliC / 1000);
            return true;
          }
        }
      }
    }
  }

  return false;
}

bool readTemperatureFromSensorsOutput(const QString &text, int *value) {
  static const QRegularExpression chipHeaderPattern(
      QStringLiteral(R"(^([^\s:][^:]+)$)"));
  static const QRegularExpression tempInputPattern(
      QStringLiteral(R"(\btemp\d+_input:\s*([+-]?\d+(?:\.\d+)?))"));

  bool inGpuChip = false;
  const QStringList lines = text.split(QLatin1Char('\n'));
  for (const QString &line : lines) {
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty()) {
      continue;
    }

    const auto chipMatch = chipHeaderPattern.match(line);
    if (chipMatch.hasMatch()) {
      inGpuChip = isGpuHwmonName(chipMatch.captured(1));
      continue;
    }

    if (!inGpuChip) {
      continue;
    }

    const auto tempMatch = tempInputPattern.match(trimmed);
    if (tempMatch.hasMatch()) {
      bool ok = false;
      const double parsed = tempMatch.captured(1).toDouble(&ok);
      if (ok && parsed > 0.0) {
        *value = static_cast<int>(parsed);
        return true;
      }
    }
  }

  return false;
}

bool readTemperatureFromSensorsCommand(CommandRunner &runner, int *value) {
  CommandRunner::RunOptions options;
  options.timeoutMs = 1500;
  const auto result =
      runner.run(QStringLiteral("sensors"), {QStringLiteral("-u")}, options);
  return result.success() &&
         readTemperatureFromSensorsOutput(result.stdout, value);
}

bool readTemperatureFromNvidiaSettings(CommandRunner &runner, int *value) {
  CommandRunner::RunOptions options;
  options.timeoutMs = 1500;
  const auto result =
      runner.run(QStringLiteral("nvidia-settings"),
                 {QStringLiteral("-q"), QStringLiteral("[gpu:0]/GPUCoreTemp"),
                  QStringLiteral("-t")},
                 options);
  if (!result.success()) {
    return false;
  }

  const QString line =
      result.stdout.split(QLatin1Char('\n'), Qt::SkipEmptyParts)
          .value(0)
          .trimmed();
  return parseMetricInt(line, value);
}

bool readNvidiaTemperatureFallback(CommandRunner &runner, int *value) {
  return readNvidiaTemperatureFromHwmonClass(value) ||
         readNvidiaTemperatureFromPciHwmon(value) ||
         readTemperatureFromSensorsCommand(runner, value) ||
         readTemperatureFromNvidiaSettings(runner, value);
}

bool readNvidiaHotspotAndMemoryTemps(CommandRunner &runner, int *hotspotC,
                                     int *memoryC) {
  if (hotspotC == nullptr || memoryC == nullptr) {
    return false;
  }

  static QString s_cachedHotspotPath;
  static QString s_cachedMemPath;
  static bool s_hwmonProbed = false;

  if (!s_hwmonProbed) {
    s_hwmonProbed = true;
    const QFileInfoList hwmonEntries =
        QDir(QStringLiteral("/sys/class/hwmon"))
            .entryInfoList({QStringLiteral("hwmon*")},
                           QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &entry : hwmonEntries) {
      const QString hwmonPath = entry.absoluteFilePath();
      if (!isGpuHwmonName(readFileText(hwmonPath + QStringLiteral("/name")))) {
        continue;
      }

      const QFileInfoList tempInputs = QDir(hwmonPath).entryInfoList(
          {QStringLiteral("temp*_input")}, QDir::Files, QDir::Name);
      for (const QFileInfo &tempFile : tempInputs) {
        const QString baseName = tempFile.baseName();
        const QString prefix = baseName.section(QLatin1Char('_'), 0, 0);
        const QString label = readFileText(hwmonPath + QLatin1Char('/') +
                                           prefix + QStringLiteral("_label"))
                                  .toLower();

        if (label.contains(QStringLiteral("junction")) ||
            label.contains(QStringLiteral("hotspot"))) {
          s_cachedHotspotPath = tempFile.absoluteFilePath();
        } else if (label.contains(QStringLiteral("mem")) ||
                   label.contains(QStringLiteral("vram"))) {
          s_cachedMemPath = tempFile.absoluteFilePath();
        }
      }
    }
  }

  if (!s_cachedHotspotPath.isEmpty()) {
    qint64 milliC = 0;
    if (readIntegerFile(s_cachedHotspotPath, &milliC) && milliC > 0) {
      *hotspotC = static_cast<int>(milliC / 1000);
    }
  }
  if (!s_cachedMemPath.isEmpty()) {
    qint64 milliC = 0;
    if (readIntegerFile(s_cachedMemPath, &milliC) && milliC > 0) {
      *memoryC = static_cast<int>(milliC / 1000);
    }
  }

  if (*hotspotC > 0 || *memoryC > 0) {
    return true;
  }

  static qint64 s_lastSensorsRun = 0;
  static bool s_sensorsSupported = true;
  const qint64 now = QDateTime::currentMSecsSinceEpoch();
  if (!s_sensorsSupported || (now - s_lastSensorsRun < 15000)) {
    return false;
  }
  s_lastSensorsRun = now;

  CommandRunner::RunOptions options;
  options.timeoutMs = 800;
  const auto result =
      runner.run(QStringLiteral("sensors"), {QStringLiteral("-u")}, options);
  if (!result.success()) {
    s_sensorsSupported = false;
    return false;
  }

  static const QRegularExpression chipHeaderPattern(
      QStringLiteral(R"(^([^\s:][^:]+)$)"));
  static const QRegularExpression junctionPattern(
      QStringLiteral(R"(\b(?:junction|hotspot)_input:\s*([+-]?\d+(?:\.\d+)?))"),
      QRegularExpression::CaseInsensitiveOption);
  static const QRegularExpression memPattern(
      QStringLiteral(R"(\b(?:mem|memory|vram)_input:\s*([+-]?\d+(?:\.\d+)?))"),
      QRegularExpression::CaseInsensitiveOption);

  bool inGpuChip = false;
  const QStringList lines = result.stdout.split(QLatin1Char('\n'));
  for (const QString &line : lines) {
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty()) {
      continue;
    }

    const auto chipMatch = chipHeaderPattern.match(line);
    if (chipMatch.hasMatch()) {
      inGpuChip = isGpuHwmonName(chipMatch.captured(1));
      continue;
    }
    if (!inGpuChip) {
      continue;
    }

    auto matchJunction = junctionPattern.match(trimmed);
    if (matchJunction.hasMatch()) {
      bool ok = false;
      const double val = matchJunction.captured(1).toDouble(&ok);
      if (ok && val > 0.0) {
        *hotspotC = static_cast<int>(val);
      }
    }

    auto matchMem = memPattern.match(trimmed);
    if (matchMem.hasMatch()) {
      bool ok = false;
      const double val = matchMem.captured(1).toDouble(&ok);
      if (ok && val > 0.0) {
        *memoryC = static_cast<int>(val);
      }
    }
  }

  return *hotspotC > 0 || *memoryC > 0;
}

bool hasNvidiaPciDevice() {
  const QFileInfoList deviceEntries =
      QDir(QStringLiteral("/sys/bus/pci/devices"))
          .entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const QFileInfo &deviceEntry : deviceEntries) {
    if (readFileText(deviceEntry.absoluteFilePath() + QStringLiteral("/vendor"))
            .compare(QStringLiteral("0x10de"), Qt::CaseInsensitive) == 0) {
      return true;
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
  m_timer.setInterval(1000);
  m_timer.setTimerType(Qt::CoarseTimer);
  connect(&m_timer, &QTimer::timeout, this, &GpuMonitor::refreshAsync);

  start();
  refresh();
}

bool GpuMonitor::available() const { return m_available; }

bool GpuMonitor::running() const { return m_timer.isActive(); }

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
  emit selectedGpuIndexChanged();
  refresh();
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
    return;
  }
  m_asyncRefreshInFlight = true;

  const int gpuIndex = m_selectedGpuIndex;
  auto *watcher = new QFutureWatcher<CommandRunner::Result>(this);
  watcher->setFuture(QtConcurrent::run(
      [gpuIndex] { return fetchNvidiaSmiTelemetryCsv(gpuIndex); }));

  connect(watcher, &QFutureWatcher<CommandRunner::Result>::finished, this,
          [this, watcher]() {
            const CommandRunner::Result result = watcher->result();
            m_asyncRefreshInFlight = false;
            ++m_refreshTickCount;
            queryGpuDevices(false);
            queryGpuProcesses(false);
            processRefreshResult(result);
            watcher->deleteLater();
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
        readGenericLinuxGpuMetrics(&nextTemp, &nextUtil, &nextUsed, &nextTotal);
    const bool hasTemperatureFallback =
        nextTemp > 0 || readNvidiaTemperatureFallback(runner, &nextTemp);

    if (!hasGenericMetrics && !hasTemperatureFallback) {
      setAvailable(false);
      if (!hasNvidiaPciDevice()) {
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
    readNvidiaHotspotAndMemoryTemps(runner, &nextHotspot, &nextMemTemp);
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
    setStatusMessage(tr("GPU telemetry output could not be parsed."));
    clearMetrics();
    return;
  }

  const QString nextName = NvidiaDetector::cleanGpuName(
      fields.at(0).trimmed(), QStringLiteral("NVIDIA"));
  int nextTemp = 0;
  int nextUtil = 0;
  int nextUsed = 0;
  int nextTotal = 0;
  int nextFanSpeed = 0;
  int nextTlimit = 0;
  double nextPowerDraw = 0.0;
  double nextPowerLimit = 0.0;
  int nextGraphicsClock = 0;
  int nextMemoryClock = 0;

  bool tempAvailable = parseMetricInt(fields.at(1), &nextTemp);
  const bool utilAvailable = parseMetricInt(fields.at(2), &nextUtil);
  const bool usedAvailable = parseMetricInt(fields.at(3), &nextUsed);
  const bool totalAvailable = parseMetricInt(fields.at(4), &nextTotal);
  if (fields.size() >= 6) {
    parseMetricInt(fields.at(5), &nextFanSpeed);
  }
  if (fields.size() >= 7) {
    parseMetricDouble(fields.at(6), &nextPowerDraw);
  }
  if (fields.size() >= 8) {
    parseMetricDouble(fields.at(7), &nextPowerLimit);
  }
  if (fields.size() >= 9) {
    parseMetricInt(fields.at(8), &nextGraphicsClock);
  }
  if (fields.size() >= 10) {
    parseMetricInt(fields.at(9), &nextMemoryClock);
  }

  QString nextPcieLink;
  if (fields.size() >= 14) {
    int curGen = 0;
    int maxGen = 0;
    int curWidth = 0;
    int maxWidth = 0;
    parseMetricInt(fields.at(10), &curGen);
    parseMetricInt(fields.at(11), &maxGen);
    parseMetricInt(fields.at(12), &curWidth);
    parseMetricInt(fields.at(13), &maxWidth);
    if (curGen > 0 && curWidth > 0) {
      if (maxGen > 0 && maxWidth > 0 &&
          (curGen != maxGen || curWidth != maxWidth)) {
        nextPcieLink = QStringLiteral("Gen %1 x%2 (Max: Gen %3 x%4)")
                           .arg(curGen)
                           .arg(curWidth)
                           .arg(maxGen)
                           .arg(maxWidth);
      } else {
        nextPcieLink = QStringLiteral("Gen %1 x%2").arg(curGen).arg(curWidth);
      }
    }
  }

  if (fields.size() >= 15) {
    parseMetricInt(fields.at(14), &nextTlimit);
  }

  if (!tempAvailable) {
    tempAvailable = readNvidiaTemperatureFallback(runner, &nextTemp);
  }

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
    setStatusMessage(
        tr("GPU telemetry output did not contain usable metrics."));
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

  if (m_fanSpeedPercent != nextFanSpeed) {
    m_fanSpeedPercent = nextFanSpeed;
    emit fanSpeedPercentChanged();
  }

  if (nextTlimit > 0 && m_thermalLimitC != nextTlimit) {
    m_thermalLimitC = nextTlimit;
    emit thermalLimitTemperatureCChanged();
  }

  if (!qFuzzyCompare(m_powerDrawW, nextPowerDraw)) {
    m_powerDrawW = nextPowerDraw;
    emit powerDrawWChanged();
  }

  if (!qFuzzyCompare(m_powerLimitW, nextPowerLimit)) {
    m_powerLimitW = nextPowerLimit;
    emit powerLimitWChanged();
  }

  if (m_graphicsClockMHz != nextGraphicsClock) {
    m_graphicsClockMHz = nextGraphicsClock;
    emit graphicsClockMHzChanged();
  }

  if (m_memoryClockMHz != nextMemoryClock) {
    m_memoryClockMHz = nextMemoryClock;
    emit memoryClockMHzChanged();
  }

  if (m_pcieLinkStatus != nextPcieLink) {
    m_pcieLinkStatus = nextPcieLink;
    emit pcieLinkStatusChanged();
  }

  int nextHotspot = 0;
  int nextMemTemp = 0;
  readNvidiaHotspotAndMemoryTemps(runner, &nextHotspot, &nextMemTemp);

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

  queryGpuProcesses();

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

  if (!qFuzzyIsNull(m_powerDrawW)) {
    m_powerDrawW = 0.0;
    emit powerDrawWChanged();
  }

  if (!qFuzzyIsNull(m_powerLimitW)) {
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
  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 1500;
  const auto res =
      runner.run(QStringLiteral("kill"),
                 {QStringLiteral("-15"), QString::number(pid)}, options);
  queryGpuProcesses(true);
  return res.success();
}

void GpuMonitor::queryGpuProcesses(bool force) {
  if (!force && !m_gpuProcesses.isEmpty() && (m_refreshTickCount % 4 != 1)) {
    return;
  }

  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 1500;

  QVariantList processes;
  QSet<int> seenPids;

  // 1. Query full nvidia-smi table which lists both Compute and Graphics
  // processes
  const auto smiResult = runner.run(QStringLiteral("nvidia-smi"), {}, options);
  if (smiResult.success()) {
    // Matches: |   0   N/A  N/A   204705   G   /usr/lib64/firefox/firefox
    // 168MiB |
    static const QRegularExpression procPattern(QStringLiteral(
        R"(\|\s*\d+\s+(?:N/A|\d+)\s+(?:N/A|\d+)\s+(\d+)\s+([CG\+]+)\s+(.+?)\s+(\d+)\s*MiB\s*\|)"));

    auto it = procPattern.globalMatch(smiResult.stdout);
    while (it.hasNext()) {
      auto match = it.next();
      int pid = match.captured(1).toInt();
      QString ptypeStr = match.captured(2).trimmed();
      QString rawName = match.captured(3).trimmed();
      int vram = match.captured(4).toInt();

      if (pid > 0 && !seenPids.contains(pid)) {
        seenPids.insert(pid);

        // Resolve clean process name from /proc/<pid>/comm if available
        QString cleanName =
            readFileText(QStringLiteral("/proc/%1/comm").arg(pid)).trimmed();
        if (cleanName.isEmpty()) {
          cleanName = rawName.contains(QLatin1Char('/'))
                          ? QFileInfo(rawName).fileName()
                          : rawName;
        }

        QString typeLabel;
        if (ptypeStr.contains(QLatin1Char('C')) &&
            ptypeStr.contains(QLatin1Char('G'))) {
          typeLabel = QStringLiteral("Compute / Graphics");
        } else if (ptypeStr.contains(QLatin1Char('C'))) {
          typeLabel = QStringLiteral("Compute / CUDA");
        } else {
          typeLabel = QStringLiteral("Graphics / Display");
        }

        QVariantMap item;
        item[QStringLiteral("pid")] = pid;
        item[QStringLiteral("name")] = cleanName;
        item[QStringLiteral("type")] = typeLabel;
        item[QStringLiteral("vramMiB")] = vram;
        processes.append(item);
      }
    }
  }

  // 2. Query compute applications only if first pass didn't find any processes
  // or failed
  if (processes.isEmpty()) {
    const auto computeResult = runner.run(
        QStringLiteral("nvidia-smi"),
        {QStringLiteral("--query-compute-apps=pid,process_name,used_memory"),
         QStringLiteral("--format=csv,noheader,nounits")},
        options);

    if (computeResult.success()) {
      const QStringList lines =
          computeResult.stdout.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
      for (const QString &line : lines) {
        const QStringList cols =
            line.split(QLatin1Char(','), Qt::KeepEmptyParts);
        if (cols.size() >= 3) {
          int pid = 0;
          int vram = 0;
          parseMetricInt(cols.at(0), &pid);
          QString rawName = cols.at(1).trimmed();
          parseMetricInt(cols.at(2), &vram);
          if (pid > 0 && !seenPids.contains(pid)) {
            seenPids.insert(pid);
            QString cleanName =
                readFileText(QStringLiteral("/proc/%1/comm").arg(pid))
                    .trimmed();
            if (cleanName.isEmpty()) {
              cleanName = rawName.contains(QLatin1Char('/'))
                              ? QFileInfo(rawName).fileName()
                              : rawName;
            }
            QVariantMap item;
            item[QStringLiteral("pid")] = pid;
            item[QStringLiteral("name")] = cleanName;
            item[QStringLiteral("type")] = QStringLiteral("Compute / CUDA");
            item[QStringLiteral("vramMiB")] = vram;
            processes.append(item);
          }
        }
      }
    }
  }

  // Sort processes from highest VRAM usage to lowest
  std::sort(processes.begin(), processes.end(),
            [](const QVariant &a, const QVariant &b) {
              const int vramA =
                  a.toMap().value(QStringLiteral("vramMiB")).toInt();
              const int vramB =
                  b.toMap().value(QStringLiteral("vramMiB")).toInt();
              if (vramA != vramB) {
                return vramA > vramB;
              }
              return a.toMap().value(QStringLiteral("pid")).toInt() <
                     b.toMap().value(QStringLiteral("pid")).toInt();
            });

  if (m_gpuProcesses != processes) {
    m_gpuProcesses = processes;
    emit gpuProcessesChanged();
  }
}

void GpuMonitor::queryGpuDevices(bool force) {
  if (!force && !m_gpuDevices.isEmpty() && (m_refreshTickCount % 30 != 1)) {
    return;
  }

  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 1200;

  const auto result =
      runner.run(QStringLiteral("nvidia-smi"),
                 {QStringLiteral("--query-gpu=index,name,uuid,pci.bus_id"),
                  QStringLiteral("--format=csv,noheader,nounits")},
                 options);

  QVariantList devices;
  if (result.success()) {
    const QStringList lines =
        result.stdout.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
      const QStringList cols = line.split(QLatin1Char(','), Qt::KeepEmptyParts);
      if (cols.size() >= 4) {
        int idx = 0;
        parseMetricInt(cols.at(0), &idx);
        QString name = NvidiaDetector::cleanGpuName(cols.at(1).trimmed(),
                                                    QStringLiteral("NVIDIA"));
        QString uuid = cols.at(2).trimmed();
        QString busId = cols.at(3).trimmed();

        QVariantMap item;
        item[QStringLiteral("index")] = idx;
        item[QStringLiteral("name")] = name;
        item[QStringLiteral("uuid")] = uuid;
        item[QStringLiteral("pciBusId")] = busId;
        devices.append(item);
      }
    }
  }

  if (devices.isEmpty()) {
    QVariantMap defaultItem;
    defaultItem[QStringLiteral("index")] = 0;
    defaultItem[QStringLiteral("name")] =
        m_gpuName.isEmpty() ? QStringLiteral("NVIDIA GPU") : m_gpuName;
    defaultItem[QStringLiteral("uuid")] = QStringLiteral("N/A");
    defaultItem[QStringLiteral("pciBusId")] = QStringLiteral("0000:00:00.0");
    devices.append(defaultItem);
  }

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
