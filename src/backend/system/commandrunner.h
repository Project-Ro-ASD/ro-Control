#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <atomic>
#include <memory>

// CommandRunner is the shared process execution layer for backend modules.
// Backend code routes system commands through this class instead of ad hoc
// calls.
class CommandRunner : public QObject {
  Q_OBJECT

public:
  struct RunOptions {
    int startTimeoutMs = 3000;
    int timeoutMs = -1;
    int retries = 0;
    int retryDelayMs = 250;
    QByteArray stdinData;
    std::shared_ptr<std::atomic_bool> cancelRequested;
  };

  struct Result {
    int exitCode = 0;
    QString stdout;
    QString stderr;
    int attempt = 1;
    bool success() const { return exitCode == 0; }
  };

  struct RootCommand {
    QString program;
    QStringList args;
  };

  explicit CommandRunner(QObject *parent = nullptr);
  static QString resolveProgramPath(const QString &program);

  // Blocking process execution that returns when the command completes.
  Result run(const QString &program, const QStringList &args = {});
  Result run(const QString &program, const QStringList &args,
             const RunOptions &options);

  // Privileged command execution routed through pkexec.
  Result runAsRoot(const QString &program, const QStringList &args = {});
  Result runAsRoot(const QString &program, const QStringList &args,
                   const RunOptions &options);
  Result runAsRootBatch(const QList<RootCommand> &commands);
  Result runAsRootBatch(const QList<RootCommand> &commands,
                        const RunOptions &options);

signals:
  // Streaming output for long-running operations such as DNF transactions.
  void outputLine(const QString &line);
  void errorLine(const QString &line);
  void commandStarted(const QString &program, const QStringList &args,
                      int attempt);
  void commandFinished(const QString &program, int exitCode, int attempt,
                       int elapsedMs);

private:
  QString resolveProgram(const QString &program) const;
  static QString helperPath();
  static QString overrideEnvironmentVariableName(const QString &program);
  Result runOnce(const QString &program, const QStringList &args,
                 const RunOptions &options, int attempt);
};
