#include "uipreferencesmanager.h"

#include <QCoreApplication>
#include <QSettings>

namespace {

struct ThemeModeEntry {
  const char *code;
  const char *label;
};

constexpr ThemeModeEntry kThemeModes[] = {
    {"system", "Follow System"},
    {"light", "Light"},
    {"dark", "Dark"},
};

} // namespace

UiPreferencesManager::UiPreferencesManager(QObject *parent) : QObject(parent) {
  QSettings settings;
  m_themeMode = normalizeThemeMode(
      settings.value(QStringLiteral("ui/themeMode"), m_themeMode).toString());
  m_compactMode =
      settings.value(QStringLiteral("ui/compactMode"), m_compactMode).toBool();
  m_showAdvancedInfo =
      settings.value(QStringLiteral("ui/showAdvancedInfo"), m_showAdvancedInfo)
          .toBool();
}

QString UiPreferencesManager::themeMode() const { return m_themeMode; }

QVariantList UiPreferencesManager::availableThemeModes() const {
  QVariantList modes;
  for (const auto &entry : kThemeModes) {
    QVariantMap mode;
    mode.insert(QStringLiteral("code"), QString::fromLatin1(entry.code));
    mode.insert(
        QStringLiteral("label"),
        QCoreApplication::translate("UiPreferencesManager", entry.label));
    modes.append(mode);
  }
  return modes;
}

bool UiPreferencesManager::compactMode() const { return m_compactMode; }

bool UiPreferencesManager::showAdvancedInfo() const {
  return m_showAdvancedInfo;
}

void UiPreferencesManager::setThemeMode(const QString &themeMode) {
  const QString normalizedThemeMode = normalizeThemeMode(themeMode);
  if (normalizedThemeMode == m_themeMode) {
    return;
  }

  m_themeMode = normalizedThemeMode;
  persistValue(QStringLiteral("ui/themeMode"), m_themeMode);
  emit themeModeChanged();
}

void UiPreferencesManager::setCompactMode(bool compactMode) {
  if (compactMode == m_compactMode) {
    return;
  }

  m_compactMode = compactMode;
  persistValue(QStringLiteral("ui/compactMode"), m_compactMode);
  emit compactModeChanged();
}

void UiPreferencesManager::setShowAdvancedInfo(bool showAdvancedInfo) {
  if (showAdvancedInfo == m_showAdvancedInfo) {
    return;
  }

  m_showAdvancedInfo = showAdvancedInfo;
  persistValue(QStringLiteral("ui/showAdvancedInfo"), m_showAdvancedInfo);
  emit showAdvancedInfoChanged();
}

void UiPreferencesManager::resetToDefaults() {
  setThemeMode(QStringLiteral("system"));
  setCompactMode(false);
  setShowAdvancedInfo(true);
}

QString
UiPreferencesManager::normalizeThemeMode(const QString &themeMode) const {
  const QString normalizedThemeMode = themeMode.trimmed().toLower();
  for (const auto &entry : kThemeModes) {
    if (normalizedThemeMode == QLatin1String(entry.code)) {
      return normalizedThemeMode;
    }
  }

  return QStringLiteral("system");
}

void UiPreferencesManager::persistValue(const QString &key,
                                        const QVariant &value) const {
  QSettings settings;
  settings.setValue(key, value);
}
