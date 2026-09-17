#include "versionparser.h"

#include <QRegularExpression>

namespace NvidiaVersionParser {

namespace {

QRegularExpression packageLineExpression(const QString &packageName) {
  return QRegularExpression(QStringLiteral(R"(^\s*%1(?:\.[^\s]+)?\s+(\S+))")
                                .arg(QRegularExpression::escape(packageName)),
                            QRegularExpression::MultilineOption);
}

QString unixDriverSectionLabel(const QString &architecture) {
  const QString normalized = architecture.trimmed().toLower();
  if (normalized == QStringLiteral("x86_64") ||
      normalized == QStringLiteral("amd64")) {
    return QStringLiteral("Linux x86_64/AMD64/EM64T");
  }
  if (normalized == QStringLiteral("aarch64") ||
      normalized == QStringLiteral("arm64")) {
    return QStringLiteral("Linux aarch64");
  }

  return QStringLiteral("Linux x86_64/AMD64/EM64T");
}

QString plainTextFromHtml(const QString &text) {
  QString plainText = text;
  plainText.remove(
      QRegularExpression(QStringLiteral(R"(<script\b[^>]*>[\s\S]*?</script>)"),
                         QRegularExpression::CaseInsensitiveOption));
  plainText.remove(
      QRegularExpression(QStringLiteral(R"(<style\b[^>]*>[\s\S]*?</style>)"),
                         QRegularExpression::CaseInsensitiveOption));
  plainText.replace(QRegularExpression(QStringLiteral(R"(<[^>]+>)")),
                    QStringLiteral(" "));
  plainText.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
  plainText.replace(QRegularExpression(QStringLiteral(R"(\s+)")),
                    QStringLiteral(" "));
  return plainText.trimmed();
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

QStringList parseOfficialUnixDriverVersions(const QString &pageText,
                                            const QString &architecture) {
  const QString plainText = plainTextFromHtml(pageText);
  const QString sectionLabel = unixDriverSectionLabel(architecture);
  const int sectionStart = plainText.indexOf(sectionLabel);
  if (sectionStart < 0) {
    return {};
  }

  const QStringList sectionBoundaries = {
      QStringLiteral("Linux x86_64/AMD64/EM64T"),
      QStringLiteral("Linux aarch64"),
      QStringLiteral("FreeBSD x64"),
      QStringLiteral("Solaris x64/x86"),
  };

  int sectionEnd = plainText.size();
  for (const QString &boundary : sectionBoundaries) {
    if (boundary == sectionLabel) {
      continue;
    }

    const int nextIndex = plainText.indexOf(boundary, sectionStart + 1);
    if (nextIndex >= 0 && nextIndex < sectionEnd) {
      sectionEnd = nextIndex;
    }
  }

  const QString sectionText =
      plainText.mid(sectionStart, sectionEnd - sectionStart);
  static const QRegularExpression versionPattern(
      QStringLiteral(R"(Latest [^:]+:\s*([0-9]+(?:\.[0-9]+)+))"));

  QStringList versions;
  auto it = versionPattern.globalMatch(sectionText);
  while (it.hasNext()) {
    const QString version = it.next().captured(1).trimmed();
    if (!version.isEmpty() && !versions.contains(version)) {
      versions.append(version);
    }
  }

  return versions;
}

QString normalizedDriverVersion(const QString &version) {
  QString normalized = version.trimmed();
  const int epochSeparator = normalized.indexOf(QLatin1Char(':'));
  if (epochSeparator >= 0) {
    normalized = normalized.mid(epochSeparator + 1);
  }

  static const QRegularExpression versionPattern(
      QStringLiteral(R"(([0-9]+(?:\.[0-9]+)+))"));
  const auto match = versionPattern.match(normalized);
  return match.hasMatch() ? match.captured(1) : QString();
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
