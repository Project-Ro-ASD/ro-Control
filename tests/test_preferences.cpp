#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QLocale>
#include <QProcess>
#include <QProcessEnvironment>
#include <QQmlEngine>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>
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
  void testLanguageManagerDynamicLocaleChange();
  void testTranslatorNotificationsDoNotReloadSystemLanguage();
  void testSystemTranslationStartup_data();
  void testSystemTranslationStartup();
};

void TestPreferences::init() {
  QCoreApplication::setOrganizationName(
      QStringLiteral("Project-Ro-ASD-TestSuite"));
  QCoreApplication::setApplicationName(
      QStringLiteral("ro-control-preferences"));

  QSettings settings;
  settings.clear();
  settings.sync();
}

void TestPreferences::testUiPreferencesDefaults() {
  UiPreferencesManager preferences;

  QVERIFY(preferences.themeMode() == QStringLiteral("light") ||
          preferences.themeMode() == QStringLiteral("dark"));
  QCOMPARE(preferences.showAdvancedInfo(), true);
  QCOMPARE(preferences.availableThemeModes().size(), 3);
  QCOMPARE(preferences.selectedThemeMode(), QStringLiteral("system"));
}

void TestPreferences::testUiPreferencesPersistChanges() {
  UiPreferencesManager preferences;
  QSignalSpy themeSpy(&preferences, &UiPreferencesManager::themeModeChanged);
  QSignalSpy advancedSpy(&preferences,
                         &UiPreferencesManager::showAdvancedInfoChanged);

  preferences.setThemeMode(QStringLiteral("dark"));
  preferences.setShowAdvancedInfo(false);

  QCOMPARE(themeSpy.count(), 1);
  QCOMPARE(advancedSpy.count(), 1);

  UiPreferencesManager reloadedPreferences;
  QCOMPARE(reloadedPreferences.selectedThemeMode(), QStringLiteral("dark"));
  QCOMPARE(reloadedPreferences.showAdvancedInfo(), false);
}

void TestPreferences::testUiPreferencesNormalizesInvalidThemeMode() {
  UiPreferencesManager preferences;

  preferences.setThemeMode(QStringLiteral("midnight"));
  QCOMPARE(preferences.selectedThemeMode(), QStringLiteral("system"));
}

void TestPreferences::testLanguageManagerExposesEffectiveLanguageMetadata() {
  QQmlEngine engine;
  QTranslator translator;
  LanguageManager manager(QCoreApplication::instance(), &engine, &translator);

  const QVariantList languages = manager.availableLanguages();
  QVERIFY(!languages.isEmpty());
  QCOMPARE(languages.first().toMap().value(QStringLiteral("code")).toString(),
           QStringLiteral("en"));
  QVERIFY(languages.first().toMap().contains(QStringLiteral("nativeLabel")));

  manager.setCurrentLanguage(QStringLiteral("system"));
  QVERIFY(manager.currentLanguage() != QStringLiteral("system"));
  QCOMPARE(manager.currentLanguage(), manager.effectiveLanguage());

  manager.setCurrentLanguage(QStringLiteral("tr"));
  QCOMPARE(manager.currentLanguage(), QStringLiteral("tr"));
  QCOMPARE(manager.effectiveLanguage(), QStringLiteral("tr"));
  QCOMPARE(manager.currentLanguageLabel(), QStringLiteral("Türkçe"));

  QStringList languageCodes;
  for (const QVariant &language : languages) {
    languageCodes.append(
        language.toMap().value(QStringLiteral("code")).toString());
  }
  QVERIFY(languageCodes.contains(QStringLiteral("de")));
  QVERIFY(languageCodes.contains(QStringLiteral("es")));
}

void TestPreferences::testLanguageManagerDynamicLocaleChange() {
  QQmlEngine engine;
  QTranslator translator;
  LanguageManager manager(QCoreApplication::instance(), &engine, &translator);

  // By default (or when setting system), followsSystem is true
  manager.setCurrentLanguage(QStringLiteral("system"));
  QCOMPARE(manager.followsSystem(), true);

  // Setting specific language sets followsSystem to false
  manager.setCurrentLanguage(QStringLiteral("es"));
  QCOMPARE(manager.followsSystem(), false);
  QCOMPARE(manager.currentLanguage(), QStringLiteral("es"));

  // Setting back to system
  manager.setFollowsSystem(true);
  QCOMPARE(manager.followsSystem(), true);

  // Triggering LocaleChange event via QCoreApplication
  QEvent localeEvent(QEvent::LocaleChange);
  QCoreApplication::sendEvent(QCoreApplication::instance(), &localeEvent);

  QEvent langEvent(QEvent::LanguageChange);
  QCoreApplication::sendEvent(QCoreApplication::instance(), &langEvent);

  QVERIFY(!manager.currentLanguage().isEmpty());
}

void TestPreferences::testTranslatorNotificationsDoNotReloadSystemLanguage() {
  QQmlEngine engine;
  QTranslator translator;
  LanguageManager manager(QCoreApplication::instance(), &engine, &translator);
  if (!translator.load(QStringLiteral(":/i18n/ro-control_tr.qm")))
    QSKIP(
        "Qt Linguist translation fixtures are unavailable in this local build");
  const QString translated = translator.translate("Main", "Driver");
  QVERIFY(!translated.isEmpty());
  QVERIFY(translated != QStringLiteral("Driver"));
  QVERIFY(QCoreApplication::installTranslator(&translator));
  QCOMPARE(QCoreApplication::translate("Main", "Driver"), translated);
  QEvent event(QEvent::LanguageChange);
  QCoreApplication::sendEvent(QCoreApplication::instance(), &event);
  QCOMPARE(QCoreApplication::translate("Main", "Driver"), translated);
  QCoreApplication::removeTranslator(&translator);
}

void TestPreferences::testSystemTranslationStartup_data() {
  QTest::addColumn<QString>("locale");
  QTest::newRow("Turkish") << QStringLiteral("tr_TR.UTF-8");
  QTest::newRow("German") << QStringLiteral("de_DE.UTF-8");
  QTest::newRow("Spanish") << QStringLiteral("es_ES.UTF-8");
}

void TestPreferences::testSystemTranslationStartup() {
  QFETCH(QString, locale);
  if (!QFile::exists(QStringLiteral(":/i18n/ro-control_tr.qm")))
    QSKIP(
        "Qt Linguist translation fixtures are unavailable in this local build");
  QProcess child;
  auto environment = QProcessEnvironment::systemEnvironment();
  for (const QString &key : {QStringLiteral("LANG"), QStringLiteral("LC_ALL"),
                             QStringLiteral("LC_MESSAGES")})
    environment.insert(key, locale);
  environment.insert(QStringLiteral("LANGUAGE"), locale.left(2));
  child.setProcessEnvironment(environment);
  child.start(QCoreApplication::applicationFilePath(),
              {QStringLiteral("--translation-startup"), locale.left(2)});
  QVERIFY(child.waitForStarted());
  QVERIFY2(child.waitForFinished(10000), "Localized startup did not terminate");
#ifndef Q_OS_LINUX
  if (child.exitCode() == 77)
    QSKIP("This platform ignores POSIX locale overrides; Linux CI runs this "
          "case");
#endif
  const QByteArray output = child.readAllStandardError();
  QVERIFY2(child.exitStatus() == QProcess::NormalExit, output.constData());
  QVERIFY2(child.exitCode() == 0, output.constData());
}

int main(int argc, char **argv) {
  QApplication application(argc, argv);
  if (application.arguments().size() == 3 &&
      application.arguments()[1] == QStringLiteral("--translation-startup")) {
    const QString language = application.arguments()[2];
    if (QLocale::system().name().left(2) != language) {
      QTextStream(stderr) << "Requested locale " << language << "; actual "
                          << QLocale::system().name() << Qt::endl;
      return 77;
    }
    QTemporaryDir directory;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                       directory.path());
    application.setOrganizationName("Project-Ro-ASD-TestSuite");
    application.setApplicationName("language-startup");
    QQmlEngine engine;
    QTranslator translator;
    LanguageManager manager(&application, &engine, &translator);
    if (translator.isEmpty() || manager.currentLanguage() != language ||
        engine.uiLanguage() != language)
      return 1;
    for (int i = 0; i < 3; ++i) {
      QEvent event(QEvent::LocaleChange);
      QCoreApplication::sendEvent(&application, &event);
      application.processEvents();
    }
    return manager.followsSystem() && manager.currentLanguage() == language ? 0
                                                                            : 2;
  }
  TestPreferences test;
  return QTest::qExec(&test, argc, argv);
}

#include "test_preferences.moc"
