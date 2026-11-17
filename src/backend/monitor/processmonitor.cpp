#include "processmonitor.h"

#include "system/commandrunner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <algorithm>
#include <unistd.h>

namespace {

// /proc/<pid>/stat fields after the comm field are 0-indexed here:
//   0 state, 1 ppid, 11 utime, 12 stime, 15 priority, 16 nice,
//   17 num_threads, 19 starttime, 20 vsize, 21 rss
constexpr int kStatFieldCount = 22;
constexpr qint64 kMinimumSampleMs = 500;

qint64 toLongLong(const QString &value, bool &ok) {
  return value.trimmed().toLongLong(&ok);
}

QString procRootOrDefault() {
  const QString v = qEnvironmentVariable("RO_CONTROL_PROC_PATH").trimmed();
  return v.isEmpty() ? QStringLiteral("/proc") : v;
}

} // namespace

ProcessMonitor::ProcessMonitor(QObject *parent) : QObject(parent) {
  m_timer.setInterval(2000);
  m_timer.setTimerType(Qt::VeryCoarseTimer);
  connect(&m_timer, &QTimer::timeout, this, &ProcessMonitor::refresh);

  m_sampleTimer.start();
  start();
  refresh();
}

bool ProcessMonitor::running() const { return m_timer.isActive(); }

bool ProcessMonitor::available() const { return m_available; }

int ProcessMonitor::processCount() const { return m_processes.size(); }

QVariantList ProcessMonitor::allProcesses() const {
  QVariantList list;
  list.reserve(m_processes.size());
  for (const ProcessInfo &info : m_processes) {
    list.append(processToMap(info));
  }
  return list;
}

QVariantList ProcessMonitor::cpuTopProcesses() const {
  return topProcesses(false, 20);
}

QVariantList ProcessMonitor::memoryTopProcesses() const {
  return topProcesses(true, 20);
}

int ProcessMonitor::updateInterval() const { return m_timer.interval(); }

QString ProcessMonitor::statusMessage() const { return m_statusMessage; }

void ProcessMonitor::start() {
  if (m_timer.isActive()) {
    return;
  }

  m_timer.start();
  emit runningChanged();
}

void ProcessMonitor::stop() {
  if (!m_timer.isActive()) {
    return;
  }

  m_timer.stop();
  emit runningChanged();
}

void ProcessMonitor::refresh() {
  const QString procRoot = procPath();
  if (!QFileInfo::exists(procRoot)) {
    setAvailable(false);
    clearProcesses();
    return;
  }

  const qint64 totalTicks = readTotalCpuTicks();
  const qint64 elapsedMs =
      m_sampleTimer.isValid() ? m_sampleTimer.elapsed() : kMinimumSampleMs;
  const int cpuCount = onlineCpuCount();

  QDir procDir(procRoot);
  const QFileInfoList entries =
      procDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

  QVector<ProcessInfo> processes;
  processes.reserve(entries.size());
  QHash<int, qint64> currentTicks;
  currentTicks.reserve(entries.size());

  for (const QFileInfo &entry : entries) {
    bool pidOk = false;
    const int pid = entry.fileName().toInt(&pidOk);
    if (!pidOk || pid <= 0) {
      continue;
    }

    ProcessInfo info;
    info.pid = pid;
    info.user = entry.owner().isEmpty() ? QStringLiteral("?") : entry.owner();

    qint64 rssBytes = 0;
    if (!parseProcStat(pid, info, rssBytes, procRoot)) {
      continue;
    }

    info.memoryBytes = rssBytes;
    info.virtualMemoryBytes = info.virtualMemoryBytes; // already set by parser

    // Command line fallback to comm when /proc/<pid>/cmdline is empty
    // (kernel threads).
    QString cmdline = readFileText(
        QStringLiteral("%1/%2/cmdline").arg(procRoot).arg(pid));
    if (cmdline.isEmpty()) {
      cmdline = info.name;
    } else {
      cmdline = cmdline.replace(QLatin1Char('\0'), QLatin1Char(' ')).trimmed();
    }
    info.cmdline = cmdline;

    // Per-process I/O counters; kernel threads or restricted processes may
    // not expose them, leaving the values at zero.
    const QString ioText =
        readFileText(QStringLiteral("%1/%2/io").arg(procRoot).arg(pid));
    if (!ioText.isEmpty()) {
      const QStringList lines =
          ioText.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
      for (const QString &line : lines) {
        const int colon = line.indexOf(QLatin1Char(':'));
        if (colon <= 0) {
          continue;
        }
        const QString key = line.left(colon).trimmed();
        bool ok = false;
        const qint64 value = line.mid(colon + 1).trimmed().toLongLong(&ok);
        if (!ok) {
          continue;
        }
        if (key == QStringLiteral("read_bytes")) {
          info.readBytes = value;
        } else if (key == QStringLiteral("write_bytes")) {
          info.writeBytes = value;
        }
      }
    }

    // CPU percentage: delta of process ticks over delta of total ticks,
    // normalized per logical CPU (top/htop style, 100% == one full core).
    const qint64 prevTicks = m_prevCpuTicks.value(pid, 0);
    if (m_prevTotalCpuTicks > 0 && totalTicks > m_prevTotalCpuTicks &&
        prevTicks > 0 && info.totalCpuTicks >= prevTicks && elapsedMs > 0) {
      const qint64 deltaTotal = totalTicks - m_prevTotalCpuTicks;
      const qint64 deltaProc = info.totalCpuTicks - prevTicks;
      if (deltaTotal > 0) {
        const double value =
            (static_cast<double>(deltaProc) / static_cast<double>(deltaTotal)) *
            100.0 * cpuCount;
        info.cpuPercent = std::clamp(value, 0.0, 100.0 * cpuCount);
      }
    }

    currentTicks.insert(pid, info.totalCpuTicks);
    processes.append(info);
  }

  m_processes = processes;
  m_prevCpuTicks = currentTicks;
  m_prevTotalCpuTicks = totalTicks;
  m_sampleTimer.restart();

  setAvailable(true);
  setStatusMessage(tr("Process telemetry is being read from /proc."));
  emit processesChanged();
}

void ProcessMonitor::setUpdateInterval(int intervalMs) {
  if (intervalMs < 250 || m_timer.interval() == intervalMs) {
    return;
  }

  m_timer.setInterval(intervalMs);
  emit updateIntervalChanged();
}

bool ProcessMonitor::killProcess(int pid, int signal) {
  if (pid <= 0) {
    setStatusMessage(tr("Invalid process id."));
    return false;
  }

  CommandRunner runner;
  auto result = runner.run(
      QStringLiteral("kill"),
      {QStringLiteral("-%1").arg(signal), QString::number(pid)});
  if (!result.success()) {
    // If regular kill fails (e.g. process belongs to another user or daemon),
    // retry via Polkit root helper.
    result = runner.runAsRoot(
        QStringLiteral("kill"),
        {QStringLiteral("-%1").arg(signal), QString::number(pid)});
  }
  if (result.success()) {
    return true;
  }

  const QString detail = result.stderr.trimmed();
  setStatusMessage(detail.isEmpty()
                       ? tr("Failed to send signal %1 to process %2.")
                             .arg(signal)
                             .arg(pid)
                       : detail);
  return false;
}

bool ProcessMonitor::setProcessPriority(int pid, int niceValue) {
  if (pid <= 0) {
    setStatusMessage(tr("Invalid process id."));
    return false;
  }

  if (niceValue < -20 || niceValue > 19) {
    setStatusMessage(tr("Nice value must be between -20 and 19."));
    return false;
  }

  CommandRunner runner;
  CommandRunner::Result result;
  // Negative nice (higher priority) or renicing another user's process requires root.
  if (niceValue < 0) {
    result = runner.runAsRoot(
        QStringLiteral("renice"),
        {QStringLiteral("-n"), QString::number(niceValue), QStringLiteral("-p"),
         QString::number(pid)});
  } else {
    result = runner.run(
        QStringLiteral("renice"),
        {QStringLiteral("-n"), QString::number(niceValue), QStringLiteral("-p"),
         QString::number(pid)});
    if (!result.success()) {
      result = runner.runAsRoot(
          QStringLiteral("renice"),
          {QStringLiteral("-n"), QString::number(niceValue), QStringLiteral("-p"),
           QString::number(pid)});
    }
  }

  if (result.success()) {
    return true;
  }

  const QString detail = result.stderr.trimmed();
  setStatusMessage(detail.isEmpty()
                       ? tr("Failed to renice process %1 to %2.")
                             .arg(pid)
                             .arg(niceValue)
                       : detail);
  return false;
}

QString ProcessMonitor::procPath() {
  return procRootOrDefault();
}

QString ProcessMonitor::readFileText(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }

  return QString::fromUtf8(file.readAll()).trimmed();
}

bool ProcessMonitor::parseProcStat(int pid, ProcessInfo &info,
                                   qint64 &rssBytes,
                                   const QString &procRoot) {
  const QString statText =
      readFileText(QStringLiteral("%1/%2/stat").arg(procRoot).arg(pid));
  if (statText.isEmpty()) {
    return false;
  }

  const int lp = statText.indexOf(QLatin1Char('('));
  const int rp = statText.lastIndexOf(QLatin1Char(')'));
  if (lp < 0 || rp <= lp) {
    return false;
  }

  info.name = statText.mid(lp + 1, rp - lp - 1);
  const QString rest = statText.mid(rp + 1).trimmed();
  const QStringList fields =
      rest.split(QLatin1Char(' '), Qt::SkipEmptyParts);
  if (fields.size() < kStatFieldCount) {
    return false;
  }

  bool ok = false;
  info.state = fields.value(0).isEmpty() ? QLatin1Char('?')
                                          : fields.value(0).at(0);
  info.ppid = fields.value(1).toInt(&ok);
  if (!ok) {
    return false;
  }

  const qint64 utime = toLongLong(fields.value(11), ok);
  const qint64 stime = toLongLong(fields.value(12), ok);
  if (!ok) {
    return false;
  }

  info.totalCpuTicks = utime + stime;
  info.priority = fields.value(15).toInt();
  info.nice = fields.value(16).toInt();
  info.threadCount = fields.value(17).toInt();
  info.startTimeTicks = toLongLong(fields.value(19), ok);
  info.virtualMemoryBytes = toLongLong(fields.value(20), ok);
  const qint64 rssPages = toLongLong(fields.value(21), ok);
  if (!ok) {
    return false;
  }

  const long pageSize = sysconf(_SC_PAGESIZE);
  rssBytes = rssPages * pageSize;
  return true;
}

qint64 ProcessMonitor::readTotalCpuTicks() {
  QFile statFile(QStringLiteral("%1/stat").arg(procPath()));
  if (!statFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return 0;
  }

  QTextStream stream(&statFile);
  const QString firstLine = stream.readLine();
  if (!firstLine.startsWith(QStringLiteral("cpu "))) {
    return 0;
  }

  const QStringList parts = firstLine.split(QLatin1Char(' '), Qt::SkipEmptyParts);
  qint64 total = 0;
  for (int i = 1; i < parts.size(); ++i) {
    bool ok = false;
    total += parts.at(i).toLongLong(&ok);
    if (!ok) {
      return 0;
    }
  }

  return total;
}

int ProcessMonitor::onlineCpuCount() {
  const long count = sysconf(_SC_NPROCESSORS_ONLN);
  return count > 0 ? static_cast<int>(count) : 1;
}

QVariantMap ProcessMonitor::processToMap(const ProcessInfo &info) const {
  QVariantMap map;
  map.insert(QStringLiteral("pid"), info.pid);
  map.insert(QStringLiteral("ppid"), info.ppid);
  map.insert(QStringLiteral("name"), info.name);
  map.insert(QStringLiteral("user"), info.user);
  map.insert(QStringLiteral("cpuPercent"), info.cpuPercent);
  map.insert(QStringLiteral("memoryBytes"),
             QVariant::fromValue<qint64>(info.memoryBytes));
  map.insert(QStringLiteral("memoryMiB"),
             static_cast<int>(info.memoryBytes / (1024 * 1024)));
  map.insert(QStringLiteral("virtualMemoryMiB"),
             static_cast<int>(info.virtualMemoryBytes / (1024 * 1024)));
  map.insert(QStringLiteral("readBytes"),
             QVariant::fromValue<qint64>(info.readBytes));
  map.insert(QStringLiteral("writeBytes"),
             QVariant::fromValue<qint64>(info.writeBytes));
  map.insert(QStringLiteral("state"), QString(info.state));
  map.insert(QStringLiteral("priority"), info.priority);
  map.insert(QStringLiteral("nice"), info.nice);
  map.insert(QStringLiteral("threadCount"), info.threadCount);
  map.insert(QStringLiteral("cmdline"), info.cmdline);
  return map;
}

QVariantList ProcessMonitor::topProcesses(bool byMemory, int limit) const {
  QVector<ProcessInfo> sorted = m_processes;
  std::sort(sorted.begin(), sorted.end(),
            [byMemory](const ProcessInfo &a, const ProcessInfo &b) {
              return byMemory ? (a.memoryBytes > b.memoryBytes)
                              : (a.cpuPercent > b.cpuPercent);
            });

  QVariantList list;
  const qsizetype count =
      limit > 0 ? std::min(static_cast<qsizetype>(limit), sorted.size())
                : sorted.size();
  list.reserve(count);
  for (qsizetype i = 0; i < count; ++i) {
    list.append(processToMap(sorted.at(i)));
  }
  return list;
}

void ProcessMonitor::setAvailable(bool value) {
  if (m_available == value) {
    return;
  }

  m_available = value;
  emit availableChanged();
}

void ProcessMonitor::setStatusMessage(const QString &value) {
  if (m_statusMessage == value) {
    return;
  }

  m_statusMessage = value;
  emit statusMessageChanged();
}

void ProcessMonitor::clearProcesses() {
  if (m_processes.isEmpty()) {
    return;
  }

  m_processes.clear();
  m_prevCpuTicks.clear();
  m_prevTotalCpuTicks = 0;
  emit processesChanged();
}
