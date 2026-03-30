#include <QFile>
#include <QRegularExpression>
#include <QTest>

namespace {

QString readFile(const QString &relativePath) {
  QFile file(QStringLiteral(RO_CONTROL_SOURCE_DIR) + QLatin1Char('/') +
             relativePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }

  return QString::fromUtf8(file.readAll());
}

} // namespace

class TestMetadata : public QObject {
  Q_OBJECT

private slots:
  void testDesktopEntryContainsCoreFields() {
    const QString desktop = readFile(
        QStringLiteral("data/icons/io.github.projectroasd.rocontrol.desktop"));
    QVERIFY(!desktop.isEmpty());
    QVERIFY(desktop.contains(QStringLiteral("[Desktop Entry]")));
    QVERIFY(desktop.contains(QStringLiteral("Exec=ro-control")));
    QVERIFY(desktop.contains(QStringLiteral("Icon=ro-control")));
    QVERIFY(desktop.contains(QStringLiteral("Categories=System;Settings;HardwareSettings;")));
    QVERIFY(desktop.contains(QStringLiteral("Name[tr]=ro-Control")));
  }

  void testAppStreamContainsExpectedIdsAndUrls() {
    const QString metainfo = readFile(QStringLiteral(
        "data/icons/io.github.projectroasd.rocontrol.metainfo.xml"));
    QVERIFY(!metainfo.isEmpty());
    QVERIFY(metainfo.contains(
        QStringLiteral("<id>io.github.projectroasd.rocontrol.desktop</id>")));
    QVERIFY(metainfo.contains(QStringLiteral(
        "<launchable type=\"desktop-id\">io.github.projectroasd.rocontrol.desktop</launchable>")));
    QVERIFY(metainfo.contains(QStringLiteral("<binary>ro-control</binary>")));
    QVERIFY(metainfo.contains(QStringLiteral("<url type=\"homepage\">https://github.com/Project-Ro-ASD/ro-Control</url>")));
    QVERIFY(metainfo.contains(QStringLiteral("<url type=\"bugtracker\">https://github.com/Project-Ro-ASD/ro-Control/issues</url>")));
  }

  void testPolicyContainsExpectedActionIds() {
    const QString policy = readFile(QStringLiteral("data/polkit/io.github.ProjectRoASD.rocontrol.policy.in"));
    QVERIFY(!policy.isEmpty());
    QVERIFY(policy.contains(QStringLiteral("@RO_CONTROL_POLICY_ID@")));
    QVERIFY(policy.contains(QStringLiteral("@RO_CONTROL_HELPER_INSTALL_PATH@")));
    QVERIFY(policy.contains(QStringLiteral("<icon_name>ro-control</icon_name>")));
  }

  void testDesktopAndAppStreamIdsStayAligned() {
    const QString desktop = readFile(
        QStringLiteral("data/icons/io.github.projectroasd.rocontrol.desktop"));
    const QString metainfo = readFile(QStringLiteral(
        "data/icons/io.github.projectroasd.rocontrol.metainfo.xml"));

    QVERIFY(!desktop.isEmpty());
    QVERIFY(!metainfo.isEmpty());

    QVERIFY(desktop.contains(QStringLiteral("Exec=ro-control")));
    QVERIFY(metainfo.contains(
        QStringLiteral("<id>io.github.projectroasd.rocontrol.desktop</id>")));

    const QRegularExpression screenshotRe(
        QStringLiteral(R"(<image>https://raw\.githubusercontent\.com/.+/docs/screenshots/.+</image>)"));
    QVERIFY(screenshotRe.match(metainfo).hasMatch());
  }

  void testAppStreamContainsCurrentReleaseVersion() {
    const QString metainfo = readFile(QStringLiteral(
        "data/icons/io.github.projectroasd.rocontrol.metainfo.xml"));
    QVERIFY(!metainfo.isEmpty());
    QVERIFY(metainfo.contains(
        QStringLiteral("<release version=\"0.2.0\" date=\"2026-03-30\" />")));
  }

  void testCliDocumentationAssetsExist() {
    const QString manPage = readFile(QStringLiteral("docs/man/ro-control.1"));
    const QString bashCompletion =
        readFile(QStringLiteral("data/completions/ro-control.bash"));
    const QString zshCompletion =
        readFile(QStringLiteral("data/completions/_ro-control"));
    const QString fishCompletion =
        readFile(QStringLiteral("data/completions/ro-control.fish"));

    QVERIFY(!manPage.isEmpty());
    QVERIFY(manPage.contains(QStringLiteral(".TH RO-CONTROL 1")));
    QVERIFY(manPage.contains(QStringLiteral("driver install")));
    QVERIFY(manPage.contains(QStringLiteral("status")));

    QVERIFY(!bashCompletion.isEmpty());
    QVERIFY(bashCompletion.contains(QStringLiteral("driver_commands")));
    QVERIFY(bashCompletion.contains(QStringLiteral("deep-clean")));

    QVERIFY(!zshCompletion.isEmpty());
    QVERIFY(zshCompletion.contains(QStringLiteral("#compdef ro-control")));
    QVERIFY(zshCompletion.contains(QStringLiteral("diagnostics")));

    QVERIFY(!fishCompletion.isEmpty());
    QVERIFY(fishCompletion.contains(QStringLiteral("complete -c ro-control")));
    QVERIFY(fishCompletion.contains(QStringLiteral("accept-license")));
  }
};

QTEST_MAIN(TestMetadata)
#include "test_metadata.moc"
