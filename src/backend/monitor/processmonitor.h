#pragma once

#include <QElapsedTimer>
#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

class ProcessMonitor : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool running READ running NOTIFY runningChanged)
  Q_PROPERTY(bool available READ available NOTIFY availableChanged)
  Q_PROPERTY(int processCount READ processCount NOTIFY processesChanged)
  Q_PROPERTY(QVariantList allProcesses READ allProcesses NOTIFY processesChanged)
  Q_PROPERTY(
      QVariantList cpuTopProcesses READ cpuTopProcesses NOTIFY processesChanged)
  Q_PROPERTY(QVariantList memoryTopProcesses READ memoryTopProcesses NOTIFY
                 processesChanged)
  Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval
                 NOTIFY updateIntervalChanged)
  Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
  struct ProcessInfo {
    int pid = 0;
    int ppid = 0;
    QString name;
    QString user;
    double cpuPercent = 0.0;
    qint64 memoryBytes = 0;      // RSS in bytes
    qint64 virtualMemoryBytes = 0; // VSZ in bytes
    qint64 readBytes = 0;
    qint64 writeBytes = 0;
    qint64 totalCpuTicks = 0;    // utime + stime, raw ticks
    qint64 startTimeTicks = 0;
    QChar state = QLatin1Char('?');
    int priority = 0;
    int nice = 0;
    int threadCount = 0;
    QString cmdline;
  };

  explicit ProcessMonitor(QObject *parent = nullptr);
  ~ProcessMonitor() override = default;

  bool running() const;
  bool available() const;
  int processCount() const;
  QVariantList allProcesses() const;
  QVariantList cpuTopProcesses() const;
  QVariantList memoryTopProcesses() const;
  int updateInterval() const;
  QString statusMessage() const;

  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();
  Q_INVOKABLE void refresh();

  // Send a signal to a process. signal defaults to SIGTERM (15).
  Q_INVOKABLE bool killProcess(int pid, int signal = 15);

  // Renice a process to the given nice value (-20..19).
  Q_INVOKABLE bool setProcessPriority(int pid, int niceValue);

  void setUpdateInterval(int intervalMs);

signals:
  void runningChanged();
  void availableChanged();
  void processesChanged();
  void updateIntervalChanged();
  void statusMessageChanged();

private:
  static QString procPath();
  static QString readFileText(const QString &path);
  static bool parseProcStat(int pid, ProcessInfo &info, qint64 &rssBytes,
                            const QString &procRoot);
  static qint64 readTotalCpuTicks();
  static int onlineCpuCount();
  QVariantMap processToMap(const ProcessInfo &info) const;
  QVariantList topProcesses(bool byMemory, int limit) const;
  void setAvailable(bool value);
  void setStatusMessage(const QString &value);
  void clearProcesses();

  QTimer m_timer;
  bool m_available = false;
  QString m_statusMessage;
  QVector<ProcessInfo> m_processes;

  QHash<int, qint64> m_prevCpuTicks;
  qint64 m_prevTotalCpuTicks = 0;
  QElapsedTimer m_sampleTimer;
};
