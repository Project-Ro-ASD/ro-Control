// CPU istatistikleri

#include "cpumonitor.h"
#include "system/commandrunner.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

#include <algorithm>

namespace {

QString readFileText(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }

  return QString::fromUtf8(file.readAll()).trimmed();
}

QString pathOverrideOrDefault(const char *envVarName, const QString &fallback) {
  const QString overridePath = qEnvironmentVariable(envVarName).trimmed();
  return overridePath.isEmpty() ? fallback : overridePath;
}

int parseMilliCelsius(const QString &value) {
  bool ok = false;
  const int milliC = value.trimmed().toInt(&ok);
  return ok && milliC > 0 ? milliC / 1000 : 0;
}

bool isPreferredCpuSensorType(const QString &sensorType) {
  const QString lowered = sensorType.trimmed().toLower();
  return lowered.contains(QStringLiteral("cpu")) ||
         lowered.contains(QStringLiteral("pkg")) ||
         lowered.contains(QStringLiteral("package")) ||
         lowered.contains(QStringLiteral("core")) ||
         lowered.contains(QStringLiteral("k10temp")) ||
         lowered.contains(QStringLiteral("tctl")) ||
         lowered.contains(QStringLiteral("tdie")) ||
         lowered.contains(QStringLiteral("x86_pkg_temp"));
}

int readFirstValidTemperature(const QStringList &paths) {
  for (const QString &path : paths) {
    const int temperatureC = parseMilliCelsius(readFileText(path));
    if (temperatureC > 0) {
      return temperatureC;
    }
  }

  return 0;
}

int readCpuTemperatureFromThermalZones() {
  QDir thermalDir(pathOverrideOrDefault("RO_CONTROL_THERMAL_ROOT",
                                        QStringLiteral("/sys/class/thermal")));
  const QFileInfoList entries = thermalDir.entryInfoList(
      {QStringLiteral("thermal_zone*")}, QDir::Dirs | QDir::NoDotAndDotDot,
      QDir::Name);

  QStringList preferredPaths;
  QStringList fallbackPaths;

  for (const QFileInfo &entry : entries) {
    const QString basePath = entry.absoluteFilePath();
    const QString type = readFileText(basePath + QStringLiteral("/type"));
    const QString tempPath = basePath + QStringLiteral("/temp");

    if (isPreferredCpuSensorType(type)) {
      preferredPaths << tempPath;
    } else {
      fallbackPaths << tempPath;
    }
  }

  const int preferredTemperature = readFirstValidTemperature(preferredPaths);
  if (preferredTemperature > 0) {
    return preferredTemperature;
  }

  return readFirstValidTemperature(fallbackPaths);
}

int readCpuTemperatureFromHwmon() {
  QDir hwmonDir(pathOverrideOrDefault("RO_CONTROL_HWMON_ROOT",
                                      QStringLiteral("/sys/class/hwmon")));
  const QFileInfoList entries = hwmonDir.entryInfoList(
      {QStringLiteral("hwmon*")}, QDir::Dirs | QDir::NoDotAndDotDot,
      QDir::Name);

  QStringList preferredPaths;
  QStringList fallbackPaths;

  for (const QFileInfo &entry : entries) {
    const QString basePath = entry.absoluteFilePath();
    const QString sensorName =
        readFileText(basePath + QStringLiteral("/name")).toLower();
    const bool preferredSensor = isPreferredCpuSensorType(sensorName);

    const QFileInfoList inputs = QDir(basePath).entryInfoList(
        {QStringLiteral("temp*_input")}, QDir::Files, QDir::Name);
    for (const QFileInfo &input : inputs) {
      const QString inputPath = input.absoluteFilePath();
      const QString labelPath =
          inputPath.left(inputPath.size() - QStringLiteral("_input").size()) +
          QStringLiteral("_label");
      const QString label = readFileText(labelPath);
      if (preferredSensor || isPreferredCpuSensorType(label)) {
        preferredPaths << inputPath;
      } else {
        fallbackPaths << inputPath;
      }
    }
  }

  const int preferredTemperature = readFirstValidTemperature(preferredPaths);
  if (preferredTemperature > 0) {
    return preferredTemperature;
  }

  return readFirstValidTemperature(fallbackPaths);
}

int readCpuTemperatureC() {
  const int thermalZoneTemperature = readCpuTemperatureFromThermalZones();
  if (thermalZoneTemperature > 0) {
    return thermalZoneTemperature;
  }

  const int hwmonTemperature = readCpuTemperatureFromHwmon();
  if (hwmonTemperature > 0) {
    return hwmonTemperature;
  }

  CommandRunner runner;
  const auto result = runner.run(QStringLiteral("sensors"));
  if (!result.success()) {
    return 0;
  }

  static const QRegularExpression preferredLinePattern(
      QStringLiteral(
          R"((package|tctl|tdie|cpu|core)[^:\n]*:\s*[+-]?([0-9]+(?:\.[0-9]+)?))"),
      QRegularExpression::CaseInsensitiveOption);
  static const QRegularExpression genericLinePattern(
      QStringLiteral(R"(:\s*[+-]?([0-9]+(?:\.[0-9]+)?))"));

  const QStringList lines = result.stdout.split(QLatin1Char('\n'));
  for (const QString &line : lines) {
    const auto preferredMatch = preferredLinePattern.match(line);
    if (preferredMatch.hasMatch()) {
      bool ok = false;
      const double value = preferredMatch.captured(2).toDouble(&ok);
      if (ok && value > 0.0) {
        return static_cast<int>(value);
      }
    }
  }

  for (const QString &line : lines) {
    const auto genericMatch = genericLinePattern.match(line);
    if (genericMatch.hasMatch()) {
      bool ok = false;
      const double value = genericMatch.captured(1).toDouble(&ok);
      if (ok && value > 0.0) {
        return static_cast<int>(value);
      }
    }
  }

  return 0;
}

} // namespace

CpuMonitor::CpuMonitor(QObject *parent) : QObject(parent) {
  m_timer.setInterval(1000);
  m_timer.setTimerType(Qt::VeryCoarseTimer);
  connect(&m_timer, &QTimer::timeout, this, &CpuMonitor::refresh);

  start();
  refresh();
}

double CpuMonitor::usagePercent() const { return m_usagePercent; }

int CpuMonitor::temperatureC() const { return m_temperatureC; }

bool CpuMonitor::available() const { return m_available; }

bool CpuMonitor::running() const { return m_timer.isActive(); }

int CpuMonitor::updateInterval() const { return m_timer.interval(); }

void CpuMonitor::refresh() {
  QFile statFile("/proc/stat");
  if (!statFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    setAvailable(false);
    setUsagePercent(0.0);
    setTemperatureC(0);
    return;
  }

  QTextStream stream(&statFile);
  const QString firstLine = stream.readLine();

  if (!firstLine.startsWith("cpu ")) {
    setAvailable(false);
    return;
  }

  const QStringList parts = firstLine.split(' ', Qt::SkipEmptyParts);
  if (parts.size() < 8) {
    setAvailable(false);
    return;
  }

  bool ok = true;
  const quint64 user = parts.value(1).toULongLong(&ok);
  if (!ok) {
    setAvailable(false);
    return;
  }

  const quint64 nice = parts.value(2).toULongLong(&ok);
  const quint64 system = parts.value(3).toULongLong(&ok);
  const quint64 idle = parts.value(4).toULongLong(&ok);
  const quint64 iowait = parts.value(5).toULongLong(&ok);
  const quint64 irq = parts.value(6).toULongLong(&ok);
  const quint64 softirq = parts.value(7).toULongLong(&ok);
  const quint64 steal = parts.size() > 8 ? parts.value(8).toULongLong(&ok) : 0;

  if (!ok) {
    setAvailable(false);
    return;
  }

  const quint64 idleAll = idle + iowait;
  const quint64 nonIdle = user + nice + system + irq + softirq + steal;
  const quint64 total = idleAll + nonIdle;

  if (m_prevTotal > 0 && total >= m_prevTotal && idleAll >= m_prevIdle) {
    const quint64 totalDelta = total - m_prevTotal;
    const quint64 idleDelta = idleAll - m_prevIdle;

    if (totalDelta > 0) {
      const double value = (static_cast<double>(totalDelta - idleDelta) /
                            static_cast<double>(totalDelta)) *
                           100.0;
      setUsagePercent(std::clamp(value, 0.0, 100.0));
    }
  }

  m_prevTotal = total;
  m_prevIdle = idleAll;

  setTemperatureC(readCpuTemperatureC());
  setAvailable(true);
}

void CpuMonitor::start() {
  if (m_timer.isActive()) {
    return;
  }

  m_timer.start();
  emit runningChanged();
}

void CpuMonitor::stop() {
  if (!m_timer.isActive()) {
    return;
  }

  m_timer.stop();
  emit runningChanged();
}

void CpuMonitor::setUpdateInterval(int intervalMs) {
  if (intervalMs < 250 || m_timer.interval() == intervalMs) {
    return;
  }

  m_timer.setInterval(intervalMs);
  emit updateIntervalChanged();
}

void CpuMonitor::setUsagePercent(double value) {
  if (qFuzzyCompare(m_usagePercent, value)) {
    return;
  }

  m_usagePercent = value;
  emit usagePercentChanged();
}

void CpuMonitor::setTemperatureC(int value) {
  if (m_temperatureC == value) {
    return;
  }

  m_temperatureC = value;
  emit temperatureCChanged();
}

void CpuMonitor::setAvailable(bool value) {
  if (m_available == value) {
    return;
  }

  m_available = value;
  emit availableChanged();
}
