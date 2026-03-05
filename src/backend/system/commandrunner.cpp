#include "commandrunner.h"
#include <QProcess>

CommandRunner::CommandRunner(QObject *parent)
    : QObject(parent)
{}

CommandRunner::Result CommandRunner::run(const QString &program, const QStringList &args)
{
    QProcess process;

    // Stdout'u anlık olarak sinyal olarak ilet
    connect(&process, &QProcess::readyReadStandardOutput, this, [&]() {
        const QString line = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        if (!line.isEmpty())
            emit outputLine(line);
    });

    process.start(program, args);

    if (!process.waitForStarted(3000)) {
        return Result {
            .exitCode = -1,
            .stdout   = {},
            .stderr   = QStringLiteral("Failed to start: %1").arg(program),
        };
    }

    process.waitForFinished(-1);

    return Result {
        .exitCode = process.exitCode(),
        .stdout   = QString::fromUtf8(process.readAllStandardOutput()),
        .stderr   = QString::fromUtf8(process.readAllStandardError()),
    };
}

CommandRunner::Result CommandRunner::runAsRoot(const QString &program, const QStringList &args)
{
    // pkexec ile privilege escalation — sudo yerine PolicyKit kullanıyoruz
    QStringList pkexecArgs;
    pkexecArgs << program << args;
    return run(QStringLiteral("pkexec"), pkexecArgs);
}
