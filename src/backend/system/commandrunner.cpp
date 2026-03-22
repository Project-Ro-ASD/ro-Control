#include "commandrunner.h"

#include <QElapsedTimer>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QThread>

#include <algorithm>

CommandRunner::CommandRunner(QObject *parent) : QObject(parent) {}

CommandRunner::Result CommandRunner::run(const QString &program,
                                         const QStringList &args) {
  return run(program, args, RunOptions{});
}

CommandRunner::Result CommandRunner::run(const QString &program,
                                         const QStringList &args,
                                         const RunOptions &options) {
  const int totalAttempts = std::max(1, options.retries + 1);
  Result lastResult{.exitCode = -1, .stdout = {}, .stderr = {}, .attempt = 1};

  for (int attempt = 1; attempt <= totalAttempts; ++attempt) {
    lastResult = runOnce(program, args, options, attempt);

    if (lastResult.success() || attempt == totalAttempts) {
      break;
    }

    if (options.retryDelayMs > 0) {
      QThread::msleep(static_cast<unsigned long>(options.retryDelayMs));
    }
  }

  return lastResult;
}

QString CommandRunner::overrideEnvironmentVariableName(const QString &program) {
  QString normalized = program.toUpper();
  normalized.replace(QLatin1Char('-'), QLatin1Char('_'));
  normalized.replace(QLatin1Char('.'), QLatin1Char('_'));
  normalized.replace(QLatin1Char('/'), QLatin1Char('_'));
  return QStringLiteral("RO_CONTROL_COMMAND_%1").arg(normalized);
}

QString CommandRunner::resolveProgramPath(const QString &program) {
  if (program.isEmpty()) {
    return {};
  }

  const QString overridePath =
      qEnvironmentVariable(overrideEnvironmentVariableName(program).toUtf8())
          .trimmed();
  if (!overridePath.isEmpty()) {
    return overridePath;
  }

  if (program.contains(QLatin1Char('/'))) {
    return program;
  }

  return QStandardPaths::findExecutable(program);
}

QString CommandRunner::resolveProgram(const QString &program) const {
  return resolveProgramPath(program);
}

CommandRunner::Result CommandRunner::runOnce(const QString &program,
                                             const QStringList &args,
                                             const RunOptions &options,
                                             int attempt) {
  QProcess process;
  QByteArray stdoutBuffer;
  QByteArray stderrBuffer;
  QElapsedTimer timer;

  emit commandStarted(program, args, attempt);
  timer.start();

  const QString resolvedProgram = resolveProgram(program);
  if (resolvedProgram.isEmpty()) {
    const Result result{
        .exitCode = -1,
        .stdout = {},
        .stderr = QStringLiteral("Executable not found: %1").arg(program),
        .attempt = attempt,
    };
    emit commandFinished(program, result.exitCode, attempt,
                         static_cast<int>(timer.elapsed()));
    return result;
  }

  // Stdout'u anlik olarak yayinla ve sonucu korumak icin buffer'a biriktir.
  connect(&process, &QProcess::readyReadStandardOutput, this, [&]() {
    const QByteArray chunk = process.readAllStandardOutput();
    stdoutBuffer.append(chunk);

    const QString line = QString::fromUtf8(chunk).trimmed();
    if (!line.isEmpty())
      emit outputLine(line);
  });

  connect(&process, &QProcess::readyReadStandardError, this, [&]() {
    const QByteArray chunk = process.readAllStandardError();
    stderrBuffer.append(chunk);

    const QString line = QString::fromUtf8(chunk).trimmed();
    if (!line.isEmpty())
      emit errorLine(line);
  });

  process.start(resolvedProgram, args);

  if (!process.waitForStarted(options.startTimeoutMs)) {
    const Result result{
        .exitCode = -1,
        .stdout = {},
        .stderr = QStringLiteral("Failed to start: %1").arg(program),
        .attempt = attempt,
    };
    emit commandFinished(program, result.exitCode, attempt,
                         static_cast<int>(timer.elapsed()));
    return result;
  }

  const bool finished = process.waitForFinished(options.timeoutMs);

  if (!finished) {
    process.kill();
    process.waitForFinished(1000);
    const Result result{
        .exitCode = -2,
        .stdout = QString::fromUtf8(stdoutBuffer),
        .stderr = QStringLiteral("Timed out: %1").arg(program),
        .attempt = attempt,
    };
    emit commandFinished(program, result.exitCode, attempt,
                         static_cast<int>(timer.elapsed()));
    return result;
  }

  stdoutBuffer.append(process.readAllStandardOutput());
  stderrBuffer.append(process.readAllStandardError());

  const Result result{
      .exitCode = process.exitCode(),
      .stdout = QString::fromUtf8(stdoutBuffer),
      .stderr = QString::fromUtf8(stderrBuffer),
      .attempt = attempt,
  };

  emit commandFinished(program, result.exitCode, attempt,
                       static_cast<int>(timer.elapsed()));
  return result;
}

CommandRunner::Result CommandRunner::runAsRoot(const QString &program,
                                               const QStringList &args) {
  return runAsRoot(program, args, RunOptions{});
}

CommandRunner::Result CommandRunner::runAsRoot(const QString &program,
                                               const QStringList &args,
                                               const RunOptions &options) {
  QStringList pkexecArgs;
  QString helperPath = QStringLiteral(RO_CONTROL_HELPER_BUILD_PATH);
  if (!QFileInfo::exists(helperPath)) {
    helperPath = QStringLiteral(RO_CONTROL_HELPER_INSTALL_PATH);
  }

  pkexecArgs << helperPath << program << args;
  return run(QStringLiteral("pkexec"), pkexecArgs, options);
}
