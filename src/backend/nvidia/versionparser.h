#pragma once

#include <QString>
#include <QStringList>

namespace NvidiaVersionParser {

QStringList parseAvailablePackageVersions(const QString &dnfOutput,
                                          const QString &packageName);
QString parseCheckUpdateVersion(const QString &dnfOutput,
                                const QString &packageName);
QStringList parseOfficialDriverLookupVersions(const QString &responseText);
QStringList parseOfficialUnixDriverVersions(const QString &pageText,
                                            const QString &architecture);
QString normalizedDriverVersion(const QString &version);
QString packageSpecForVersion(const QString &packageName,
                              const QString &version);
QStringList buildVersionedPackageSpecs(const QStringList &packageNames,
                                       const QString &version);

} // namespace NvidiaVersionParser
