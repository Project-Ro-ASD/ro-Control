#pragma once

#include <QString>
#include <QStringList>

namespace NvidiaVersionParser {

QStringList parseAvailablePackageVersions(const QString &dnfOutput,
                                          const QString &packageName);
QString parseCheckUpdateVersion(const QString &dnfOutput,
                                const QString &packageName);
QString packageSpecForVersion(const QString &packageName,
                              const QString &version);
QStringList buildVersionedPackageSpecs(const QStringList &packageNames,
                                       const QString &version);

} // namespace NvidiaVersionParser
