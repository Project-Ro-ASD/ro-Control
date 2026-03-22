#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QDir>
#include <QFile>
#include <QScopedPointer>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QtQuickControls2/QQuickStyle>

class DetectorMock : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool gpuFound READ gpuFound WRITE setGpuFound NOTIFY infoChanged)
  Q_PROPERTY(QString gpuName READ gpuName WRITE setGpuName NOTIFY infoChanged)
  Q_PROPERTY(QString driverVersion READ driverVersion WRITE setDriverVersion
                 NOTIFY infoChanged)
  Q_PROPERTY(bool driverLoaded READ driverLoaded WRITE setDriverLoaded NOTIFY
                 infoChanged)
  Q_PROPERTY(bool nouveauActive READ nouveauActive WRITE setNouveauActive
                 NOTIFY infoChanged)
  Q_PROPERTY(bool secureBootEnabled READ secureBootEnabled WRITE
                 setSecureBootEnabled NOTIFY infoChanged)
  Q_PROPERTY(bool waylandSession READ waylandSession WRITE setWaylandSession
                 NOTIFY infoChanged)
  Q_PROPERTY(QString sessionType READ sessionType WRITE setSessionType NOTIFY
                 infoChanged)
  Q_PROPERTY(QString activeDriver READ activeDriver WRITE setActiveDriver NOTIFY
                 infoChanged)
  Q_PROPERTY(QString verificationReport READ verificationReport WRITE
                 setVerificationReport NOTIFY infoChanged)

public:
  bool gpuFound() const { return m_gpuFound; }
  QString gpuName() const { return m_gpuName; }
  QString driverVersion() const { return m_driverVersion; }
  bool driverLoaded() const { return m_driverLoaded; }
  bool nouveauActive() const { return m_nouveauActive; }
  bool secureBootEnabled() const { return m_secureBootEnabled; }
  bool waylandSession() const { return m_waylandSession; }
  QString sessionType() const { return m_sessionType; }
  QString activeDriver() const { return m_activeDriver; }
  QString verificationReport() const { return m_verificationReport; }

  void setGpuFound(bool value) {
    if (m_gpuFound == value) {
      return;
    }
    m_gpuFound = value;
    emit infoChanged();
  }

  void setGpuName(const QString &value) {
    if (m_gpuName == value) {
      return;
    }
    m_gpuName = value;
    emit infoChanged();
  }

  void setDriverVersion(const QString &value) {
    if (m_driverVersion == value) {
      return;
    }
    m_driverVersion = value;
    emit infoChanged();
  }

  void setDriverLoaded(bool value) {
    if (m_driverLoaded == value) {
      return;
    }
    m_driverLoaded = value;
    emit infoChanged();
  }

  void setNouveauActive(bool value) {
    if (m_nouveauActive == value) {
      return;
    }
    m_nouveauActive = value;
    emit infoChanged();
  }

  void setSecureBootEnabled(bool value) {
    if (m_secureBootEnabled == value) {
      return;
    }
    m_secureBootEnabled = value;
    emit infoChanged();
  }

  void setWaylandSession(bool value) {
    if (m_waylandSession == value) {
      return;
    }
    m_waylandSession = value;
    emit infoChanged();
  }

  void setSessionType(const QString &value) {
    if (m_sessionType == value) {
      return;
    }
    m_sessionType = value;
    emit infoChanged();
  }

  void setActiveDriver(const QString &value) {
    if (m_activeDriver == value) {
      return;
    }
    m_activeDriver = value;
    emit infoChanged();
  }

  void setVerificationReport(const QString &value) {
    if (m_verificationReport == value) {
      return;
    }
    m_verificationReport = value;
    emit infoChanged();
  }

  Q_INVOKABLE void refresh() { emit infoChanged(); }

signals:
  void infoChanged();

private:
  bool m_gpuFound = false;
  QString m_gpuName;
  QString m_driverVersion;
  bool m_driverLoaded = false;
  bool m_nouveauActive = false;
  bool m_secureBootEnabled = false;
  bool m_waylandSession = false;
  QString m_sessionType = QStringLiteral("unknown");
  QString m_activeDriver = QStringLiteral("Not Installed / Unknown");
  QString m_verificationReport = QStringLiteral("GPU: None");
};

class InstallerMock : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool proprietaryAgreementRequired READ proprietaryAgreementRequired
                 WRITE setProprietaryAgreementRequired NOTIFY
                     proprietaryAgreementChanged)
  Q_PROPERTY(QString proprietaryAgreementText READ proprietaryAgreementText
                 WRITE setProprietaryAgreementText NOTIFY
                     proprietaryAgreementChanged)
  Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)

public:
  bool proprietaryAgreementRequired() const {
    return m_proprietaryAgreementRequired;
  }
  QString proprietaryAgreementText() const { return m_proprietaryAgreementText; }
  bool busy() const { return m_busy; }

  void setProprietaryAgreementRequired(bool value) {
    if (m_proprietaryAgreementRequired == value) {
      return;
    }
    m_proprietaryAgreementRequired = value;
    emit proprietaryAgreementChanged();
  }

  void setProprietaryAgreementText(const QString &value) {
    if (m_proprietaryAgreementText == value) {
      return;
    }
    m_proprietaryAgreementText = value;
    emit proprietaryAgreementChanged();
  }

  void setBusy(bool value) {
    if (m_busy == value) {
      return;
    }
    m_busy = value;
    emit busyChanged();
  }

  Q_INVOKABLE void refreshProprietaryAgreement() {}
  Q_INVOKABLE void installProprietary(bool) {}
  Q_INVOKABLE void installOpenSource() {}
  Q_INVOKABLE void remove() {}
  Q_INVOKABLE void deepClean() {}

signals:
  void progressMessage(const QString &message);
  void proprietaryAgreementChanged();
  void busyChanged();
  void installFinished(bool success, const QString &message);
  void removeFinished(bool success, const QString &message);

private:
  bool m_proprietaryAgreementRequired = false;
  QString m_proprietaryAgreementText;
  bool m_busy = false;
};

class UpdaterMock : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool updateAvailable READ updateAvailable WRITE setUpdateAvailable
                 NOTIFY updateAvailableChanged)
  Q_PROPERTY(QString currentVersion READ currentVersion WRITE setCurrentVersion
                 NOTIFY currentVersionChanged)
  Q_PROPERTY(QString latestVersion READ latestVersion WRITE setLatestVersion
                 NOTIFY latestVersionChanged)
  Q_PROPERTY(QStringList availableVersions READ availableVersions WRITE
                 setAvailableVersions NOTIFY availableVersionsChanged)
  Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)

public:
  bool updateAvailable() const { return m_updateAvailable; }
  QString currentVersion() const { return m_currentVersion; }
  QString latestVersion() const { return m_latestVersion; }
  QStringList availableVersions() const { return m_availableVersions; }
  bool busy() const { return m_busy; }

  void setUpdateAvailable(bool value) {
    if (m_updateAvailable == value) {
      return;
    }
    m_updateAvailable = value;
    emit updateAvailableChanged();
  }

  void setCurrentVersion(const QString &value) {
    if (m_currentVersion == value) {
      return;
    }
    m_currentVersion = value;
    emit currentVersionChanged();
  }

  void setLatestVersion(const QString &value) {
    if (m_latestVersion == value) {
      return;
    }
    m_latestVersion = value;
    emit latestVersionChanged();
  }

  void setAvailableVersions(const QStringList &value) {
    if (m_availableVersions == value) {
      return;
    }
    m_availableVersions = value;
    emit availableVersionsChanged();
  }

  void setBusy(bool value) {
    if (m_busy == value) {
      return;
    }
    m_busy = value;
    emit busyChanged();
  }

  Q_INVOKABLE void checkForUpdate() {}
  Q_INVOKABLE void applyUpdate() {}
  Q_INVOKABLE void applyVersion(const QString &) {}
  Q_INVOKABLE void refreshAvailableVersions() {}

signals:
  void updateAvailableChanged();
  void currentVersionChanged();
  void latestVersionChanged();
  void availableVersionsChanged();
  void busyChanged();
  void progressMessage(const QString &message);
  void updateFinished(bool success, const QString &message);

private:
  bool m_updateAvailable = false;
  QString m_currentVersion;
  QString m_latestVersion;
  QStringList m_availableVersions;
  bool m_busy = false;
};

class TestDriverPage : public QObject {
  Q_OBJECT

private slots:
  void testDriverInstalledLocallyUsesDetectorVersion();
  void testOperationRunningStillTracksBackendBusyAfterManualStateChanges();

private:
  QObject *createPage(DetectorMock *detector, InstallerMock *installer,
                      UpdaterMock *updater, QQmlEngine *engine) const;
};

QObject *TestDriverPage::createPage(DetectorMock *detector,
                                    InstallerMock *installer,
                                    UpdaterMock *updater,
                                    QQmlEngine *engine) const {
  engine->rootContext()->setContextProperty("nvidiaDetector", detector);
  engine->rootContext()->setContextProperty("nvidiaInstaller", installer);
  engine->rootContext()->setContextProperty("nvidiaUpdater", updater);

  const QString sourceRoot = QStringLiteral(RO_CONTROL_SOURCE_DIR);
  const QString sourcePagePath =
      QDir(sourceRoot).filePath(QStringLiteral("src/qml/pages/DriverPage.qml"));
  QFile sourceFile(sourcePagePath);
  if (!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qFatal("Failed to open DriverPage.qml from source tree");
  }

  QString pageSource = QString::fromUtf8(sourceFile.readAll());
  const QString importLine = QStringLiteral("import QtQuick.Layouts\n");
  const QString injectedImport =
      QStringLiteral("import QtQuick.Layouts\nimport \"../components\"\n");
  if (!pageSource.contains(importLine)) {
    qFatal("DriverPage.qml did not contain the expected import anchor");
  }
  pageSource.replace(importLine, injectedImport);

  QTemporaryDir tempDir;
  if (!tempDir.isValid()) {
    qFatal("Failed to create temporary directory for QML test");
  }

  const QString qmlRoot = tempDir.path() + QStringLiteral("/qml");
  const QString pagesDir = qmlRoot + QStringLiteral("/pages");
  const QString componentsDir = qmlRoot + QStringLiteral("/components");
  if (!QDir().mkpath(pagesDir) || !QDir().mkpath(componentsDir)) {
    qFatal("Failed to create temporary QML fixture directories");
  }

  const QString tempPagePath = pagesDir + QStringLiteral("/DriverPage.qml");
  QFile tempPageFile(tempPagePath);
  if (!tempPageFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qFatal("Failed to create temporary DriverPage.qml");
  }
  tempPageFile.write(pageSource.toUtf8());
  tempPageFile.close();

  const QStringList componentFiles = {QStringLiteral("ActionButton.qml"),
                                      QStringLiteral("InfoBadge.qml"),
                                      QStringLiteral("SectionPanel.qml"),
                                      QStringLiteral("StatusBanner.qml"),
                                      QStringLiteral("StatCard.qml")};
  for (const QString &fileName : componentFiles) {
    const QString sourceComponentPath =
        QDir(sourceRoot).filePath(QStringLiteral("src/qml/components/") +
                                  fileName);
    const QString targetComponentPath = componentsDir + QLatin1Char('/') + fileName;
    if (!QFile::copy(sourceComponentPath, targetComponentPath)) {
      qFatal("Failed to copy component fixture for DriverPage test");
    }
  }

  const QString sourceQmldirPath =
      QDir(sourceRoot).filePath(QStringLiteral("src/qml/components/qmldir"));
  const QString targetQmldirPath = componentsDir + QStringLiteral("/qmldir");
  if (!QFile::copy(sourceQmldirPath, targetQmldirPath)) {
    qFatal("Failed to copy qmldir fixture for DriverPage test");
  }

  QQmlComponent component(engine, QUrl::fromLocalFile(tempPagePath));

  if (!component.isReady()) {
    qFatal("%s", qPrintable(component.errorString()));
  }

  QVariantMap theme;
  theme.insert(QStringLiteral("accentA"), QStringLiteral("#1f6feb"));
  theme.insert(QStringLiteral("accentB"), QStringLiteral("#2da44e"));
  theme.insert(QStringLiteral("accentC"), QStringLiteral("#bf8700"));
  theme.insert(QStringLiteral("warning"), QStringLiteral("#9a6700"));
  theme.insert(QStringLiteral("success"), QStringLiteral("#1a7f37"));
  theme.insert(QStringLiteral("danger"), QStringLiteral("#cf222e"));
  theme.insert(QStringLiteral("warningBg"), QStringLiteral("#fff8c5"));
  theme.insert(QStringLiteral("successBg"), QStringLiteral("#dafbe1"));
  theme.insert(QStringLiteral("dangerBg"), QStringLiteral("#ffebe9"));
  theme.insert(QStringLiteral("infoBg"), QStringLiteral("#ddf4ff"));
  theme.insert(QStringLiteral("card"), QStringLiteral("#ffffff"));
  theme.insert(QStringLiteral("cardStrong"), QStringLiteral("#f6f8fa"));
  theme.insert(QStringLiteral("border"), QStringLiteral("#d0d7de"));
  theme.insert(QStringLiteral("text"), QStringLiteral("#1f2328"));
  theme.insert(QStringLiteral("textMuted"), QStringLiteral("#656d76"));
  theme.insert(QStringLiteral("textSoft"), QStringLiteral("#57606a"));

  QVariantMap initialProperties;
  initialProperties.insert(QStringLiteral("width"), 1280);
  initialProperties.insert(QStringLiteral("height"), 900);
  initialProperties.insert(QStringLiteral("theme"), theme);

  QObject *object = component.createWithInitialProperties(initialProperties);
  if (object == nullptr) {
    qFatal("Failed to create DriverPage test component");
  }

  return object;
}

void TestDriverPage::testDriverInstalledLocallyUsesDetectorVersion() {
  QQmlEngine engine;
  DetectorMock detector;
  InstallerMock installer;
  UpdaterMock updater;

  detector.setDriverVersion(QStringLiteral("580.126.18"));
  updater.setCurrentVersion(QString());

  QScopedPointer<QObject> page(
      createPage(&detector, &installer, &updater, &engine));

  QTRY_VERIFY(page->property("driverInstalledLocally").toBool());
  QCOMPARE(page->property("installedVersionLabel").toString(),
           QStringLiteral("580.126.18"));
}

void TestDriverPage::
    testOperationRunningStillTracksBackendBusyAfterManualStateChanges() {
  QQmlEngine engine;
  DetectorMock detector;
  InstallerMock installer;
  UpdaterMock updater;

  QScopedPointer<QObject> page(
      createPage(&detector, &installer, &updater, &engine));

  QVERIFY(!page->property("operationRunning").toBool());

  QVERIFY(QMetaObject::invokeMethod(page.get(), "setOperationState",
                                    Q_ARG(QVariant, QVariant(QStringLiteral("Updater"))),
                                    Q_ARG(QVariant, QVariant(QStringLiteral("Checking"))),
                                    Q_ARG(QVariant, QVariant(QStringLiteral("info"))),
                                    Q_ARG(QVariant, QVariant(true))));
  QTRY_VERIFY(page->property("operationRunning").toBool());

  QVERIFY(QMetaObject::invokeMethod(page.get(), "finishOperation",
                                    Q_ARG(QVariant, QVariant(QStringLiteral("Updater"))),
                                    Q_ARG(QVariant, QVariant(true)),
                                    Q_ARG(QVariant, QVariant(QStringLiteral("Done")))));
  QTRY_VERIFY(!page->property("operationRunning").toBool());

  updater.setBusy(true);
  QTRY_VERIFY(page->property("operationRunning").toBool());

  updater.setBusy(false);
  QTRY_VERIFY(!page->property("operationRunning").toBool());
}

int main(int argc, char **argv) {
  qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
  qputenv("QT_QUICK_CONTROLS_STYLE", QByteArrayLiteral("Basic"));

  QQuickStyle::setStyle(QStringLiteral("Basic"));

  QGuiApplication app(argc, argv);
  TestDriverPage testCase;
  return QTest::qExec(&testCase, argc, argv);
}
#include "test_driver_page.moc"
