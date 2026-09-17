#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class UiPreferencesManager : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString themeMode READ themeMode WRITE setThemeMode NOTIFY
                 themeModeChanged)
  Q_PROPERTY(
      QString selectedThemeMode READ selectedThemeMode NOTIFY themeModeChanged)
  Q_PROPERTY(QVariantList availableThemeModes READ availableThemeModes CONSTANT)
  Q_PROPERTY(bool showAdvancedInfo READ showAdvancedInfo WRITE
                 setShowAdvancedInfo NOTIFY showAdvancedInfoChanged)

public:
  explicit UiPreferencesManager(QObject *parent = nullptr);

  QString themeMode() const;
  QString selectedThemeMode() const;
  QVariantList availableThemeModes() const;

  bool showAdvancedInfo() const;

  Q_INVOKABLE void setThemeMode(const QString &themeMode);
  Q_INVOKABLE void setShowAdvancedInfo(bool showAdvancedInfo);
  Q_INVOKABLE void resetToDefaults();

signals:
  void themeModeChanged();
  void showAdvancedInfoChanged();

private:
  QString normalizeThemeMode(const QString &themeMode) const;
  bool detectSystemDarkMode() const;
  QString systemThemeMode() const;
  void persistValue(const QString &key, const QVariant &value) const;

  QString m_themeMode = QStringLiteral("light");
  bool m_systemDarkMode = false;
  bool m_showAdvancedInfo = true;
};
