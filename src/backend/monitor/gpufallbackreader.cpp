#include "gpufallbackreader.h"
#include "system/commandrunner.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <algorithm>

namespace {

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

bool readTemperatureFromSensorsCommand(CommandRunner &runner, int *value) {
  CommandRunner::RunOptions options;
  options.timeoutMs = 1500;
  const auto result =
      runner.run(QStringLiteral("sensors"), {QStringLiteral("-u")}, options);
  return result.success() &&
         GpuFallbackReader::readTemperatureFromSensorsOutput(result.stdout,
                                                             value);
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
  bool ok = false;
  const int val = line.toInt(&ok);
  if (ok && val > 0) {
    *value = val;
    return true;
  }
  return false;
}

} // namespace

bool GpuFallbackReader::readTemperatureFromSensorsOutput(const QString &text,
                                                         int *value) {
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

bool GpuFallbackReader::readGenericLinuxGpuMetrics(int *temperatureC,
                                                   int *utilizationPercent,
                                                   int *memoryUsedMiB,
                                                   int *memoryTotalMiB) {
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

bool GpuFallbackReader::readNvidiaTemperatureFallback(CommandRunner &runner,
                                                      int *value) {
  return readNvidiaTemperatureFromHwmonClass(value) ||
         readNvidiaTemperatureFromPciHwmon(value) ||
         readTemperatureFromSensorsCommand(runner, value) ||
         readTemperatureFromNvidiaSettings(runner, value);
}

bool GpuFallbackReader::readNvidiaHotspotAndMemoryTemps(CommandRunner &runner,
                                                        int *hotspotC,
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

bool GpuFallbackReader::hasNvidiaPciDevice() {
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
