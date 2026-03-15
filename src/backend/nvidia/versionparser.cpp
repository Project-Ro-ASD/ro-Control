#include "versionparser.h"

#include <QRegularExpression>

namespace NvidiaVersionParser {

namespace {

QRegularExpression packageLineExpression(const QString &packageName) {
  return QRegularExpression(QStringLiteral(R"(^\s*%1(?:\.[^\s]+)?\s+(\S+))")
                                .arg(QRegularExpression::escape(packageName)),
                            QRegularExpression::MultilineOption);
}

} // namespace

QStringList parseAvailablePackageVersions(const QString &dnfOutput,
                                          const QString &packageName) {
  QStringList versions;
  const QRegularExpression re = packageLineExpression(packageName);
  auto it = re.globalMatch(dnfOutput);

  while (it.hasNext()) {
    const QString version = it.next().captured(1).trimmed();
    if (!version.isEmpty() && !versions.contains(version)) {
      versions.append(version);
    }
  }

  return versions;
}

QString parseCheckUpdateVersion(const QString &dnfOutput,
                                const QString &packageName) {
  const QStringList versions =
      parseAvailablePackageVersions(dnfOutput, packageName);
  return versions.isEmpty() ? QString() : versions.constFirst();
}

QString packageSpecForVersion(const QString &packageName,
                              const QString &version) {
  const QString trimmedVersion = version.trimmed();
  if (trimmedVersion.isEmpty()) {
    return packageName;
  }

  return QStringLiteral("%1-%2").arg(packageName, trimmedVersion);
}

QStringList buildVersionedPackageSpecs(const QStringList &packageNames,
                                       const QString &version) {
  QStringList specs;
  specs.reserve(packageNames.size());

  for (const QString &packageName : packageNames) {
    specs << packageSpecForVersion(packageName, version);
  }

  return specs;
}

} // namespace NvidiaVersionParser
