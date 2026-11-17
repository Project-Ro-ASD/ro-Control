#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class UiPreferencesManager : public QObject {
  Q_OBJECT

  Q_PROPERTY(
      qreal animationDurationFactor READ animationDurationFactor CONSTANT)

  Q_PROPERTY(QString themeMode READ themeMode WRITE setThemeMode NOTIFY
                 themeModeChanged)
  Q_PROPERTY(
      QString selectedThemeMode READ selectedThemeMode NOTIFY themeModeChanged)
  Q_PROPERTY(QVariantList availableThemeModes READ availableThemeModes CONSTANT)
  Q_PROPERTY(bool showAdvancedInfo READ showAdvancedInfo WRITE
                 setShowAdvancedInfo NOTIFY showAdvancedInfoChanged)
  Q_PROPERTY(int monitorRefreshInterval READ monitorRefreshInterval WRITE
                 setMonitorRefreshInterval NOTIFY monitorRefreshIntervalChanged)
  Q_PROPERTY(int processRefreshInterval READ processRefreshInterval WRITE
                 setProcessRefreshInterval NOTIFY processRefreshIntervalChanged)
  Q_PROPERTY(int processListLimit READ processListLimit WRITE
                 setProcessListLimit NOTIFY processListLimitChanged)

public:
  explicit UiPreferencesManager(QObject *parent = nullptr);

  qreal animationDurationFactor() const;
  QString themeMode() const;
  QString selectedThemeMode() const;
  QVariantList availableThemeModes() const;

  bool showAdvancedInfo() const;

  int monitorRefreshInterval() const;
  int processRefreshInterval() const;
  int processListLimit() const;

  Q_INVOKABLE void setThemeMode(const QString &themeMode);
  Q_INVOKABLE void setShowAdvancedInfo(bool showAdvancedInfo);
  Q_INVOKABLE void setMonitorRefreshInterval(int intervalMs);
  Q_INVOKABLE void setProcessRefreshInterval(int intervalMs);
  Q_INVOKABLE void setProcessListLimit(int limit);
  Q_INVOKABLE void resetToDefaults();

signals:
  void themeModeChanged();
  void showAdvancedInfoChanged();
  void monitorRefreshIntervalChanged();
  void processRefreshIntervalChanged();
  void processListLimitChanged();

private:
  QString normalizeThemeMode(const QString &themeMode) const;
  bool detectSystemDarkMode() const;
  QString systemThemeMode() const;
  void persistValue(const QString &key, const QVariant &value) const;

  QString m_themeMode = QStringLiteral("light");
  bool m_systemDarkMode = false;
  bool m_showAdvancedInfo = true;
  int m_monitorRefreshInterval = 1000;
  int m_processRefreshInterval = 2000;
  int m_processListLimit = 50;
};
