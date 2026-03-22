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
  const char *label;
  const char *nativeLabel;
  bool shipped;
};

constexpr LanguageEntry kSupportedLanguages[] = {
    {"system", "System Default", "System Default", true},
    {"en", "English", "English", true},
    {"de", "German", "Deutsch", true},
    {"es", "Spanish", "Espanol", true},
    {"tr", "Turkish", "Turkce", true},
};

} // namespace

LanguageManager::LanguageManager(QCoreApplication *application,
                                 QQmlEngine *engine, QTranslator *translator,
                                 QObject *parent)
    : QObject(parent), m_application(application), m_engine(engine),
      m_translator(translator) {
  QSettings settings;
  const QString storedLanguage =
      settings.value(QStringLiteral("ui/language"), QStringLiteral("system"))
          .toString();
  setCurrentLanguage(storedLanguage);
}

QString LanguageManager::currentLanguage() const { return m_currentLanguage; }

QString LanguageManager::effectiveLanguage() const {
  return effectiveLanguageCode(m_currentLanguage);
}

QString LanguageManager::currentLanguageLabel() const {
  if (m_currentLanguage == QStringLiteral("system")) {
    return QStringLiteral("%1 (%2)").arg(
        displayNameForLanguage(m_currentLanguage),
        displayNameForLanguage(effectiveLanguage()));
  }

  return displayNameForLanguage(m_currentLanguage);
}

QVariantList LanguageManager::availableLanguages() const {
  QVariantList languages;
  for (const auto &entry : kSupportedLanguages) {
    QVariantMap language;
    language.insert(QStringLiteral("code"), QString::fromLatin1(entry.code));
    language.insert(QStringLiteral("label"), QString::fromLatin1(entry.label));
    language.insert(QStringLiteral("nativeLabel"),
                    QString::fromLatin1(entry.nativeLabel));
    language.insert(QStringLiteral("shipped"), entry.shipped);
    languages.append(language);
  }

  return languages;
}

void LanguageManager::setCurrentLanguage(const QString &languageCode) {
  const QString normalizedLanguage = normalizeLanguageCode(languageCode);
  if (normalizedLanguage == m_currentLanguage &&
      loadLanguage(normalizedLanguage)) {
    return;
  }

  if (!loadLanguage(normalizedLanguage)) {
    return;
  }

  m_currentLanguage = normalizedLanguage;

  QSettings settings;
  settings.setValue(QStringLiteral("ui/language"), m_currentLanguage);

  emit currentLanguageChanged();
}

QString
LanguageManager::displayNameForLanguage(const QString &languageCode) const {
  const QString normalizedLanguage = normalizeLanguageCode(languageCode);
  for (const auto &entry : kSupportedLanguages) {
    if (QString::fromLatin1(entry.code) == normalizedLanguage) {
      return QString::fromLatin1(entry.nativeLabel);
    }
  }

  return normalizedLanguage;
}

QString
LanguageManager::normalizeLanguageCode(const QString &languageCode) const {
  const QString normalizedLanguage = languageCode.trimmed().toLower();
  for (const auto &entry : kSupportedLanguages) {
    if (QString::fromLatin1(entry.code) == normalizedLanguage) {
      return normalizedLanguage;
    }
  }

  return QStringLiteral("system");
}

QString LanguageManager::systemLanguageCode() const {
  return QLocale::system().name().section(QLatin1Char('_'), 0, 0).toLower();
}

QString
LanguageManager::effectiveLanguageCode(const QString &languageCode) const {
  const QString normalizedLanguage = normalizeLanguageCode(languageCode);
  return normalizedLanguage == QStringLiteral("system") ? systemLanguageCode()
                                                        : normalizedLanguage;
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
