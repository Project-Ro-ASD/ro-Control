// Shell komut çalıştırıcı

#include "commandrunner.h"
#include <QProcess>

CommandRunner::CommandRunner(QObject *parent)
    : QObject(parent)
{}

CommandRunner::Result CommandRunner::run(const QString &command, const QStringList &args)
{
    QProcess process;
    process.start(command, args);
    process.waitForFinished(-1); // -1 = timeout yok

    return Result {
        .exitCode = process.exitCode(),
        .stdout   = QString::fromUtf8(process.readAllStandardOutput()),
        .stderr   = QString::fromUtf8(process.readAllStandardError()),
    };
}
