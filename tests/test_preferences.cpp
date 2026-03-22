#include <QCoreApplication>
#include <QQmlEngine>
#include <QSettings>
#include <QSignalSpy>
#include <QTest>
#include <QTranslator>

#include "backend/system/languagemanager.h"
#include "backend/system/uipreferencesmanager.h"

class TestPreferences : public QObject {
  Q_OBJECT

private slots:
  void init();
  void testUiPreferencesDefaults();
  void testUiPreferencesPersistChanges();
  void testUiPreferencesNormalizesInvalidThemeMode();
  void testLanguageManagerExposesEffectiveLanguageMetadata();
};

void TestPreferences::init() {
  QCoreApplication::setOrganizationName(
      QStringLiteral("Project-Ro-ASD-TestSuite"));
  QCoreApplication::setApplicationName(QStringLiteral("ro-control-preferences"));

  QSettings settings;
  settings.clear();
  settings.sync();
}

void TestPreferences::testUiPreferencesDefaults() {
  UiPreferencesManager preferences;

  QCOMPARE(preferences.themeMode(), QStringLiteral("system"));
  QCOMPARE(preferences.compactMode(), false);
  QCOMPARE(preferences.showAdvancedInfo(), true);
  QCOMPARE(preferences.availableThemeModes().size(), 3);
}

void TestPreferences::testUiPreferencesPersistChanges() {
  UiPreferencesManager preferences;
  QSignalSpy themeSpy(&preferences, &UiPreferencesManager::themeModeChanged);
  QSignalSpy compactSpy(&preferences, &UiPreferencesManager::compactModeChanged);
  QSignalSpy advancedSpy(&preferences,
                         &UiPreferencesManager::showAdvancedInfoChanged);

  preferences.setThemeMode(QStringLiteral("dark"));
  preferences.setCompactMode(true);
  preferences.setShowAdvancedInfo(false);

  QCOMPARE(themeSpy.count(), 1);
  QCOMPARE(compactSpy.count(), 1);
  QCOMPARE(advancedSpy.count(), 1);

  UiPreferencesManager reloadedPreferences;
  QCOMPARE(reloadedPreferences.themeMode(), QStringLiteral("dark"));
  QCOMPARE(reloadedPreferences.compactMode(), true);
  QCOMPARE(reloadedPreferences.showAdvancedInfo(), false);
}

void TestPreferences::testUiPreferencesNormalizesInvalidThemeMode() {
  UiPreferencesManager preferences;

  preferences.setThemeMode(QStringLiteral("midnight"));
  QCOMPARE(preferences.themeMode(), QStringLiteral("system"));
}

void TestPreferences::testLanguageManagerExposesEffectiveLanguageMetadata() {
  QQmlEngine engine;
  QTranslator translator;
  LanguageManager manager(QCoreApplication::instance(), &engine, &translator);

  const QVariantList languages = manager.availableLanguages();
  QVERIFY(!languages.isEmpty());
  QCOMPARE(languages.first().toMap().value(QStringLiteral("code")).toString(),
           QStringLiteral("system"));
  QVERIFY(languages.first()
              .toMap()
              .contains(QStringLiteral("nativeLabel")));

  manager.setCurrentLanguage(QStringLiteral("system"));
  QVERIFY(
      manager.currentLanguageLabel().startsWith(QStringLiteral("System Default")));

  manager.setCurrentLanguage(QStringLiteral("tr"));
  QCOMPARE(manager.currentLanguage(), QStringLiteral("tr"));
  QCOMPARE(manager.effectiveLanguage(), QStringLiteral("tr"));
  QCOMPARE(manager.currentLanguageLabel(), QStringLiteral("Turkce"));
}

QTEST_GUILESS_MAIN(TestPreferences)

#include "test_preferences.moc"
