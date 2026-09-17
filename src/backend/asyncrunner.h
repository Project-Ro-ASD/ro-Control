#pragma once

#include <QMetaObject>
#include <QPointer>
#include <QString>
#include <QStringList>

#include "system/commandrunner.h"

template <typename T>
void emitProgressAsync(const QPointer<T> &guard, const QString &message) {
  QMetaObject::invokeMethod(
      guard,
      [guard, message]() {
        if (guard) {
          emit guard->progressMessage(message);
        }
      },
      Qt::QueuedConnection);
}

template <typename T>
void attachRunnerLogging(CommandRunner &runner, const QPointer<T> &guard) {
  QObject::connect(
      &runner, &CommandRunner::outputLine, guard,
      [guard](const QString &message) { emitProgressAsync(guard, message); });

  QObject::connect(
      &runner, &CommandRunner::errorLine, guard,
      [guard](const QString &message) { emitProgressAsync(guard, message); });

  QObject::connect(
      &runner, &CommandRunner::commandStarted, guard,
      [guard](const QString &program, const QStringList &args, int attempt) {
        QStringList visibleArgs = args;
        bool privilegedBatch = false;
        if (!visibleArgs.isEmpty() &&
            visibleArgs.constFirst().contains(
                QStringLiteral("ro-control-helper"))) {
          visibleArgs.removeFirst();
          privilegedBatch =
              !visibleArgs.isEmpty() &&
              visibleArgs.constFirst() == QStringLiteral("--batch");
        }

        if (program == QStringLiteral("pkexec") && privilegedBatch) {
          emitProgressAsync(
              guard,
              T::tr("Starting privileged transaction batch "
                    "(attempt %1). The exact commands and package manager "
                    "output will appear below.")
                  .arg(attempt));
          return;
        }

        const QString commandLine = QStringLiteral("$ %1 %2").arg(
            program, visibleArgs.join(QLatin1Char(' ')).trimmed());
        emitProgressAsync(guard, T::tr("Starting command (attempt %1): %2")
                                     .arg(attempt)
                                     .arg(commandLine.trimmed()));
      });

  QObject::connect(
      &runner, &CommandRunner::commandFinished, guard,
      [guard](const QString &program, int exitCode, int attempt,
              int elapsedMs) {
        if (program == QStringLiteral("pkexec")) {
          return;
        }

        emitProgressAsync(
            guard, T::tr("Command finished (attempt %1, exit %2, %3 ms): %4")
                       .arg(attempt)
                       .arg(exitCode)
                       .arg(elapsedMs)
                       .arg(program));
      });
}
