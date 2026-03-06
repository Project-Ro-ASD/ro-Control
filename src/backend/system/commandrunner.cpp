#include "commandrunner.h"
#include <QProcess>

CommandRunner::CommandRunner(QObject *parent)
    : QObject(parent)
{}

CommandRunner::Result CommandRunner::run(const QString &program, const QStringList &args)
{
    QProcess process;

    // Tüm stdout verisini biriktir — hem anlık sinyal hem de sonuç için
    QByteArray stdoutBuf;

    // Stdout'u anlık olarak sinyal olarak ilet
    connect(&process, &QProcess::readyReadStandardOutput, this, [&]() {
        const QByteArray data = process.readAllStandardOutput();
        stdoutBuf.append(data);
        const QString line = QString::fromUtf8(data).trimmed();
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

    // waitForFinished sonrası kalan veriyi de oku
    const QByteArray remaining = process.readAllStandardOutput();
    stdoutBuf.append(remaining);

    return Result {
        .exitCode = process.exitCode(),
        .stdout   = QString::fromUtf8(stdoutBuf),
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
