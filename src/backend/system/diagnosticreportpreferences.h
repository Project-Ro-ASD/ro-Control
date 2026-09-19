#pragma once

#include <QString>

struct DiagnosticReportPreferences {
  QString format = QStringLiteral("markdown");
  QString destination = QStringLiteral("preview");
};

class DiagnosticReportPreferenceStore {
public:
  static DiagnosticReportPreferences load();
  static void save(const DiagnosticReportPreferences &preferences);
  static QString normalizedFormat(const QString &format);
  static QString normalizedDestination(const QString &destination);
};
