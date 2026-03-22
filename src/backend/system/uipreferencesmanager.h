#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class UiPreferencesManager : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString themeMode READ themeMode WRITE setThemeMode NOTIFY
                 themeModeChanged)
  Q_PROPERTY(QVariantList availableThemeModes READ availableThemeModes CONSTANT)
  Q_PROPERTY(bool compactMode READ compactMode WRITE setCompactMode NOTIFY
                 compactModeChanged)
  Q_PROPERTY(bool showAdvancedInfo READ showAdvancedInfo WRITE
                 setShowAdvancedInfo NOTIFY showAdvancedInfoChanged)

public:
  explicit UiPreferencesManager(QObject *parent = nullptr);

  QString themeMode() const;
  QVariantList availableThemeModes() const;

  bool compactMode() const;
  bool showAdvancedInfo() const;

  Q_INVOKABLE void setThemeMode(const QString &themeMode);
  Q_INVOKABLE void setCompactMode(bool compactMode);
  Q_INVOKABLE void setShowAdvancedInfo(bool showAdvancedInfo);
  Q_INVOKABLE void resetToDefaults();

signals:
  void themeModeChanged();
  void compactModeChanged();
  void showAdvancedInfoChanged();

private:
  QString normalizeThemeMode(const QString &themeMode) const;
  void persistValue(const QString &key, const QVariant &value) const;

  QString m_themeMode = QStringLiteral("system");
  bool m_compactMode = false;
  bool m_showAdvancedInfo = true;
};
