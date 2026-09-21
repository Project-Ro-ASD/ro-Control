#include "gpuprocessinventory.h"
#include "nvidia/detector.h"
#include "system/commandrunner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>

namespace {

QString readFileText(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }

  return QString::fromUtf8(file.readAll()).trimmed();
}

bool parseMetricInt(const QString &field, int *value) {
  if (value == nullptr) {
    return false;
  }

  static const QRegularExpression bracketRegex(
      QStringLiteral(R"(\s*\[[^\]]+\]\s*)"));
  static const QRegularExpression percentRegex(QStringLiteral(R"(\s*%\s*)"));
  static const QRegularExpression mibRegex(
      QStringLiteral(R"(\s*mib\b)"), QRegularExpression::CaseInsensitiveOption);

  QString normalized = field.trimmed();
  normalized.remove(bracketRegex);
  normalized.remove(percentRegex);
  normalized.remove(mibRegex);
  normalized = normalized.trimmed();

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

} // namespace

QVariantList GpuProcessInventory::queryGpuProcesses(int selectedGpuIndex) {
  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 1500;

  QVariantList processes;
  QSet<int> seenPids;

  // 1. Query full nvidia-smi table which lists both Compute and Graphics
  // processes
  const auto smiResult =
      runner.run(QStringLiteral("nvidia-smi"),
                 {QStringLiteral("--id=%1").arg(selectedGpuIndex)}, options);
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
        {QStringLiteral("--id=%1").arg(selectedGpuIndex),
         QStringLiteral("--query-compute-apps=pid,process_name,used_memory"),
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

  return processes;
}

QVariantList GpuProcessInventory::queryGpuDevices(int selectedGpuIndex,
                                                  const QString &fallbackName) {
  Q_UNUSED(selectedGpuIndex);
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
        fallbackName.isEmpty() ? QStringLiteral("NVIDIA GPU") : fallbackName;
    defaultItem[QStringLiteral("uuid")] = QStringLiteral("N/A");
    defaultItem[QStringLiteral("pciBusId")] = QStringLiteral("0000:00:00.0");
    devices.append(defaultItem);
  }

  return devices;
}

bool GpuProcessInventory::killProcess(int pid,
                                      const QVariantList &knownProcesses,
                                      QString *errorMessage) {
  if (pid <= 1) {
    if (errorMessage) {
      *errorMessage = QStringLiteral("Invalid process ID.");
    }
    return false;
  }

  if (!knownProcesses.isEmpty()) {
    const bool isListedGpuProcess = std::any_of(
        knownProcesses.cbegin(), knownProcesses.cend(),
        [pid](const QVariant &v) {
          return v.toMap().value(QStringLiteral("pid")).toInt() == pid;
        });
    if (!isListedGpuProcess) {
      if (errorMessage) {
        *errorMessage = QStringLiteral(
            "The selected process is no longer using the active GPU.");
      }
      return false;
    }
  }

  CommandRunner runner;
  CommandRunner::RunOptions options;
  options.timeoutMs = 1500;
  const auto res =
      runner.run(QStringLiteral("kill"),
                 {QStringLiteral("-15"), QString::number(pid)}, options);
  if (!res.success()) {
    if (errorMessage) {
      *errorMessage = QStringLiteral("The process could not be terminated. "
                                     "Check ownership and permissions.");
    }
    return false;
  }

  return true;
}
