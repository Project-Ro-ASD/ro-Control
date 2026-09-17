#include "languagemanager.h"

#include <QCoreApplication>
#include <QLocale>
#include <QQmlEngine>
#include <QSettings>
#include <QTranslator>
#include <utility>

namespace {

struct LanguageEntry {
  const char *code;
  const char *nativeLabel;
  bool shipped;
};

constexpr LanguageEntry kSupportedLanguages[] = {
    {"en", "English", true},
    {"de", "Deutsch", true},
    {"es", "Español", true},
    {"tr", "Türkçe", true},
};

QString localizedLanguageLabel(const QString &code) {
  if (code == QStringLiteral("en")) {
    return QCoreApplication::translate("LanguageManager", "English");
  }
  if (code == QStringLiteral("de")) {
    return QCoreApplication::translate("LanguageManager", "German");
  }
  if (code == QStringLiteral("es")) {
    return QCoreApplication::translate("LanguageManager", "Spanish");
  }
  if (code == QStringLiteral("tr")) {
    return QCoreApplication::translate("LanguageManager", "Turkish");
  }
  return code;
}

bool isSupportedLanguage(const QString &languageCode) {
  for (const auto &entry : kSupportedLanguages) {
    if (QString::fromLatin1(entry.code) == languageCode && entry.shipped) {
      return true;
    }
  }

  return false;
}

} // namespace

LanguageManager::LanguageManager(QCoreApplication *application,
                                 QQmlEngine *engine, QTranslator *translator,
                                 QObject *parent)
    : QObject(parent), m_application(application), m_engine(engine),
      m_translator(translator) {
  if (m_application != nullptr) {
    m_application->installEventFilter(this);
  }

  QSettings settings;
  m_followsSystem =
      settings.value(QStringLiteral("ui/follow_system_language"), true)
          .toBool();

  if (m_followsSystem) {
    const QString systemLanguage = normalizeLanguageCode(systemLanguageCode());
    loadLanguage(systemLanguage);
    m_currentLanguage = systemLanguage;
  } else {
    const QString savedLanguage =
        settings.value(QStringLiteral("ui/language")).toString();
    const QString normalizedLanguage = normalizeLanguageCode(savedLanguage);
    loadLanguage(normalizedLanguage);
    m_currentLanguage = normalizedLanguage;
  }
}

LanguageManager::~LanguageManager() {
  if (m_application != nullptr) {
    m_application->removeEventFilter(this);
  }
}

bool LanguageManager::eventFilter(QObject *watched, QEvent *event) {
  if (event != nullptr && (event->type() == QEvent::LocaleChange ||
                           event->type() == QEvent::LanguageChange)) {
    if (m_followsSystem) {
      applySystemLanguage();
    }
  }
  return QObject::eventFilter(watched, event);
}

void LanguageManager::applySystemLanguage() {
  const QString systemLanguage = normalizeLanguageCode(systemLanguageCode());
  if (!loadLanguage(systemLanguage)) {
    return;
  }

  const bool changed = (m_currentLanguage != systemLanguage);
  m_currentLanguage = systemLanguage;

  if (changed) {
    emit currentLanguageChanged();
  }
}

bool LanguageManager::followsSystem() const { return m_followsSystem; }

void LanguageManager::setFollowsSystem(bool follow) {
  if (m_followsSystem == follow) {
    return;
  }

  m_followsSystem = follow;
  QSettings settings;
  settings.setValue(QStringLiteral("ui/follow_system_language"),
                    m_followsSystem);

  if (m_followsSystem) {
    applySystemLanguage();
  }
}

QString LanguageManager::currentLanguage() const { return m_currentLanguage; }

QString LanguageManager::effectiveLanguage() const {
  return effectiveLanguageCode(m_currentLanguage);
}

QString LanguageManager::currentLanguageLabel() const {
  return displayNameForLanguage(m_currentLanguage);
}

QVariantList LanguageManager::availableLanguages() const {
  QVariantList languages;
  for (const auto &entry : kSupportedLanguages) {
    if (!entry.shipped) {
      continue;
    }

    const QString code = QString::fromLatin1(entry.code);
    QVariantMap language;
    language.insert(QStringLiteral("code"), code);
    language.insert(QStringLiteral("label"), localizedLanguageLabel(code));
    language.insert(QStringLiteral("nativeLabel"),
                    QString::fromUtf8(entry.nativeLabel));
    language.insert(QStringLiteral("shipped"), entry.shipped);
    languages.append(language);
  }

  return languages;
}

void LanguageManager::setCurrentLanguage(const QString &languageCode) {
  const QString trimmed = languageCode.trimmed().toLower();
  if (trimmed == QStringLiteral("system") ||
      trimmed == QStringLiteral("auto") || trimmed.isEmpty()) {
    m_followsSystem = true;
    QSettings settings;
    settings.setValue(QStringLiteral("ui/follow_system_language"), true);
    settings.remove(QStringLiteral("ui/language"));

    const QString systemLanguage = normalizeLanguageCode(systemLanguageCode());
    const bool changed = (m_currentLanguage != systemLanguage);

    if (!loadLanguage(systemLanguage)) {
      return;
    }

    m_currentLanguage = systemLanguage;
    if (changed) {
      emit currentLanguageChanged();
    }
    return;
  }

  const QString normalizedLanguage = normalizeLanguageCode(languageCode);
  m_followsSystem = false;

  QSettings settings;
  settings.setValue(QStringLiteral("ui/follow_system_language"), false);
  settings.setValue(QStringLiteral("ui/language"), normalizedLanguage);

  if (normalizedLanguage == m_currentLanguage) {
    return;
  }

  if (!loadLanguage(normalizedLanguage)) {
    return;
  }

  m_currentLanguage = normalizedLanguage;

  emit currentLanguageChanged();
}

QString
LanguageManager::displayNameForLanguage(const QString &languageCode) const {
  const QString trimmedLanguage = languageCode.trimmed().toLower();
  for (const auto &entry : kSupportedLanguages) {
    if (QString::fromLatin1(entry.code) == trimmedLanguage) {
      return QString::fromUtf8(entry.nativeLabel);
    }
  }

  const QLocale locale(trimmedLanguage);
  const QString nativeLanguageName = locale.nativeLanguageName();
  if (!nativeLanguageName.isEmpty()) {
    return nativeLanguageName;
  }

  return trimmedLanguage;
}

QString
LanguageManager::normalizeLanguageCode(const QString &languageCode) const {
  const QString normalizedLanguage = languageCode.trimmed().toLower();
  if (isSupportedLanguage(normalizedLanguage)) {
    return normalizedLanguage;
  }

  const QString systemLanguage = systemLanguageCode();
  if (isSupportedLanguage(systemLanguage)) {
    return systemLanguage;
  }

  return QStringLiteral("en");
}

QString LanguageManager::systemLanguageCode() const {
  return QLocale::system().name().section(QLatin1Char('_'), 0, 0).toLower();
}

QString
LanguageManager::effectiveLanguageCode(const QString &languageCode) const {
  return normalizeLanguageCode(languageCode);
}

bool LanguageManager::loadLanguage(const QString &languageCode) {
  if (m_application == nullptr || m_engine == nullptr ||
      m_translator == nullptr) {
    return false;
  }

  const QString effectiveLanguage = effectiveLanguageCode(languageCode);

  m_application->removeTranslator(m_translator);

  bool loaded = false;
  if (effectiveLanguage != QStringLiteral("en")) {
    loaded = m_translator->load(
        QStringLiteral(":/i18n/ro-control_%1.qm").arg(effectiveLanguage));
  }

  if (loaded) {
    m_application->installTranslator(m_translator);
  }

  m_engine->setUiLanguage(effectiveLanguage);
  m_engine->retranslate();
  return true;
}
