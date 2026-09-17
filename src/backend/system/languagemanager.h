#pragma once

#include <QEvent>
#include <QObject>
#include <QString>
#include <QVariantList>

class QApplication;
class QCoreApplication;
class QQmlEngine;
class QTranslator;

class LanguageManager : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString currentLanguage READ currentLanguage WRITE
                 setCurrentLanguage NOTIFY currentLanguageChanged)
  Q_PROPERTY(QString effectiveLanguage READ effectiveLanguage NOTIFY
                 currentLanguageChanged)
  Q_PROPERTY(QString currentLanguageLabel READ currentLanguageLabel NOTIFY
                 currentLanguageChanged)
  Q_PROPERTY(QVariantList availableLanguages READ availableLanguages CONSTANT)
  Q_PROPERTY(bool followsSystem READ followsSystem WRITE setFollowsSystem NOTIFY
                 currentLanguageChanged)

public:
  explicit LanguageManager(QCoreApplication *application, QQmlEngine *engine,
                           QTranslator *translator, QObject *parent = nullptr);
  ~LanguageManager() override;

  QString currentLanguage() const;
  QString effectiveLanguage() const;
  QString currentLanguageLabel() const;
  QVariantList availableLanguages() const;
  bool followsSystem() const;

  Q_INVOKABLE void setCurrentLanguage(const QString &languageCode);
  Q_INVOKABLE void setFollowsSystem(bool follow);
  Q_INVOKABLE void applySystemLanguage();
  Q_INVOKABLE QString displayNameForLanguage(const QString &languageCode) const;

signals:
  void currentLanguageChanged();

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

private:
  QString normalizeLanguageCode(const QString &languageCode) const;
  QString systemLanguageCode() const;
  QString effectiveLanguageCode(const QString &languageCode) const;
  bool loadLanguage(const QString &languageCode);

  QCoreApplication *m_application = nullptr;
  QQmlEngine *m_engine = nullptr;
  QTranslator *m_translator = nullptr;
  QString m_currentLanguage = QStringLiteral("en");
  bool m_followsSystem = true;
};
