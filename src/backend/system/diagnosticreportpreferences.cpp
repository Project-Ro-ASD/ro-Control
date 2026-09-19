#include "diagnosticreportpreferences.h"

#include <QSettings>

QString
DiagnosticReportPreferenceStore::normalizedFormat(const QString &format) {
  const QString normalized = format.trimmed().toLower();
  return (normalized == QStringLiteral("plain") ||
          normalized == QStringLiteral("json"))
             ? normalized
             : QStringLiteral("markdown");
}

QString DiagnosticReportPreferenceStore::normalizedDestination(
    const QString &destination) {
  return destination.trimmed().toLower() == QStringLiteral("clipboard")
             ? QStringLiteral("clipboard")
             : QStringLiteral("preview");
}

DiagnosticReportPreferences DiagnosticReportPreferenceStore::load() {
  QSettings settings;
  settings.beginGroup(QStringLiteral("DiagnosticReport"));
  const DiagnosticReportPreferences preferences{
      .format = normalizedFormat(
          settings.value(QStringLiteral("format"), QStringLiteral("markdown"))
              .toString()),
      .destination = normalizedDestination(
          settings
              .value(QStringLiteral("destination"), QStringLiteral("preview"))
              .toString())};
  settings.endGroup();
  return preferences;
}

void DiagnosticReportPreferenceStore::save(
    const DiagnosticReportPreferences &preferences) {
  QSettings settings;
  settings.beginGroup(QStringLiteral("DiagnosticReport"));
  settings.setValue(QStringLiteral("format"),
                    normalizedFormat(preferences.format));
  settings.setValue(QStringLiteral("destination"),
                    normalizedDestination(preferences.destination));
  settings.endGroup();
}
