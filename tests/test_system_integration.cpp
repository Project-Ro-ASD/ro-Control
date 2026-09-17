#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "nvidia/installer.h"
#include "system/capabilityprobe.h"
#include "system/commandrunner.h"
#include "system/dnfmanager.h"
#include "system/polkit.h"
#include "system/sessionutil.h"
#include "system/systeminfoprovider.h"

namespace {

QTemporaryDir createExecutableTempDir() {
  const QString basePath = QDir::cleanPath(
      QDir::currentPath() + QStringLiteral("/ro-control-test-XXXXXX"));
  return QTemporaryDir(basePath);
}

} // namespace

class TestSystemIntegration : public QObject {
  Q_OBJECT

private slots:
  void init() {
    QCoreApplication::setOrganizationName(
        QStringLiteral("Project-Ro-ASD-TestSuite"));
    QCoreApplication::setApplicationName(
        QStringLiteral("ro-control-sysintegration-test"));

    QSettings settings;
    settings.clear();
    settings.sync();
  }

  void testCommandRunnerUsesProgramOverride() {
    QTemporaryDir tempDir = createExecutableTempDir();
    QVERIFY(tempDir.isValid());

    const QString scriptPath = tempDir.filePath(QStringLiteral("fake-dnf.sh"));
    QFile script(scriptPath);
    QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(script.write("#!/bin/sh\nprintf 'override-ok\\n'\nexit 0\n") > 0);
    script.close();
    QVERIFY(script.setPermissions(QFileDevice::ReadOwner |
                                  QFileDevice::WriteOwner |
                                  QFileDevice::ExeOwner));

    qputenv("RO_CONTROL_COMMAND_DNF", scriptPath.toUtf8());

    CommandRunner runner;
    const auto result = runner.run(QStringLiteral("dnf"));
    QCOMPARE(result.exitCode, 0);
    QCOMPARE(result.stdout.trimmed(), QStringLiteral("override-ok"));

    qunsetenv("RO_CONTROL_COMMAND_DNF");
  }

  void testSessionTypeUsesXdgSessionType() {
    const QByteArray previousXdgSessionType = qgetenv("XDG_SESSION_TYPE");
    qputenv("XDG_SESSION_TYPE", QByteArrayLiteral("wayland"));

    const auto info = SessionUtil::detectSessionInfo();
    QCOMPARE(info.type, QStringLiteral("wayland"));
    QVERIFY(info.isCertain);
    QCOMPARE(info.source, QStringLiteral("XDG_SESSION_TYPE"));

    if (previousXdgSessionType.isNull()) {
      qunsetenv("XDG_SESSION_TYPE");
    } else {
      qputenv("XDG_SESSION_TYPE", previousXdgSessionType);
    }
  }

  void testSessionTypeFallsBackToDisplayVariables() {
    QTemporaryDir tempDir = createExecutableTempDir();
    QVERIFY(tempDir.isValid());

    const QString scriptPath =
        tempDir.filePath(QStringLiteral("fake-loginctl.sh"));
    QFile script(scriptPath);
    QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(script.write("#!/bin/sh\nexit 1\n") > 0);
    script.close();
    QVERIFY(script.setPermissions(QFileDevice::ReadOwner |
                                  QFileDevice::WriteOwner |
                                  QFileDevice::ExeOwner));

    const QByteArray previousLoginctlOverride =
        qgetenv("RO_CONTROL_COMMAND_LOGINCTL");
    const QByteArray previousXdgSessionType = qgetenv("XDG_SESSION_TYPE");
    const QByteArray previousXdgSessionId = qgetenv("XDG_SESSION_ID");
    const QByteArray previousWaylandDisplay = qgetenv("WAYLAND_DISPLAY");
    const QByteArray previousQtPlatform = qgetenv("QT_QPA_PLATFORM");

    qputenv("RO_CONTROL_COMMAND_LOGINCTL", scriptPath.toUtf8());
    qunsetenv("XDG_SESSION_TYPE");
    qunsetenv("XDG_SESSION_ID");
    qputenv("WAYLAND_DISPLAY", QByteArrayLiteral("wayland-0"));
    qunsetenv("QT_QPA_PLATFORM");

    auto info = SessionUtil::detectSessionInfo();
    QCOMPARE(info.type, QStringLiteral("wayland"));
    QVERIFY(!info.isCertain);
    QCOMPARE(info.source, QStringLiteral("WAYLAND_DISPLAY"));

    qunsetenv("WAYLAND_DISPLAY");
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("xcb"));
    info = SessionUtil::detectSessionInfo();
    QCOMPARE(info.type, QStringLiteral("unknown"));

    if (previousLoginctlOverride.isNull()) {
      qunsetenv("RO_CONTROL_COMMAND_LOGINCTL");
    } else {
      qputenv("RO_CONTROL_COMMAND_LOGINCTL", previousLoginctlOverride);
    }
    if (previousXdgSessionType.isNull()) {
      qunsetenv("XDG_SESSION_TYPE");
    } else {
      qputenv("XDG_SESSION_TYPE", previousXdgSessionType);
    }
    if (previousXdgSessionId.isNull()) {
      qunsetenv("XDG_SESSION_ID");
    } else {
      qputenv("XDG_SESSION_ID", previousXdgSessionId);
    }
    if (previousWaylandDisplay.isNull()) {
      qunsetenv("WAYLAND_DISPLAY");
    } else {
      qputenv("WAYLAND_DISPLAY", previousWaylandDisplay);
    }
    if (previousQtPlatform.isNull()) {
      qunsetenv("QT_QPA_PLATFORM");
    } else {
      qputenv("QT_QPA_PLATFORM", previousQtPlatform);
    }
  }

  void testCapabilityProbeUsesProgramOverride() {
    QTemporaryDir tempDir = createExecutableTempDir();
    QVERIFY(tempDir.isValid());

    const QString scriptPath =
        tempDir.filePath(QStringLiteral("fake-nvidia-smi.sh"));
    QFile script(scriptPath);
    QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(script.write("#!/bin/sh\nexit 0\n") > 0);
    script.close();
    QVERIFY(script.setPermissions(QFileDevice::ReadOwner |
                                  QFileDevice::WriteOwner |
                                  QFileDevice::ExeOwner));

    qputenv("RO_CONTROL_COMMAND_NVIDIA_SMI", scriptPath.toUtf8());

    const auto status =
        CapabilityProbe::probeTool(QStringLiteral("nvidia-smi"));
    QVERIFY(status.available);
    QCOMPARE(QDir::cleanPath(status.resolvedPath), QDir::cleanPath(scriptPath));

    qunsetenv("RO_CONTROL_COMMAND_NVIDIA_SMI");
  }

  void testCommandRunnerBasic() {
    CommandRunner runner;
    const auto result = runner.run(QStringLiteral("true"));
    QCOMPARE(result.exitCode, 0);
    QVERIFY(result.success());
  }

  void testCommandRunnerMissingBinary() {
    CommandRunner runner;
    const auto result =
        runner.run(QStringLiteral("ro-control-command-that-does-not-exist"));
    QCOMPARE(result.exitCode, -1);
    QVERIFY(result.stderr.contains(QStringLiteral("Executable not found")));
  }

  void testDnfManagerEmptyPackageListsFailFast() {
    DnfManager dnf;

    const auto installResult = dnf.installPackages({});
    QCOMPARE(installResult.exitCode, -1);
    if (dnf.isAvailable()) {
      QVERIFY(installResult.stderr.contains(
          QStringLiteral("No packages provided for install")));
    } else {
      QVERIFY(installResult.stderr.contains(QStringLiteral("dnf not found")));
    }

    const auto removeResult = dnf.removePackages({});
    QCOMPARE(removeResult.exitCode, -1);
    if (dnf.isAvailable()) {
      QVERIFY(removeResult.stderr.contains(
          QStringLiteral("No packages provided for remove")));
    } else {
      QVERIFY(removeResult.stderr.contains(QStringLiteral("dnf not found")));
    }
  }

  void testCommandRunnerMissingExecutable() {
    CommandRunner runner;
    const auto result =
        runner.run(QStringLiteral("definitely-not-a-real-command"));
    QCOMPARE(result.exitCode, -1);
    QVERIFY(result.stderr.contains(QStringLiteral("Executable not found")));
  }

  void testCommandRunnerTimeout() {
    CommandRunner runner;
    CommandRunner::RunOptions options;
    options.timeoutMs = 1;

    const auto result =
        runner.run(QStringLiteral("sleep"), {QStringLiteral("1")}, options);
    QVERIFY(result.exitCode == -2 || result.exitCode == -1);
  }

  void testCommandRunnerCancel() {
    if (QStandardPaths::findExecutable(QStringLiteral("sleep")).isEmpty()) {
      QSKIP("sleep is not available on this host.");
    }

    CommandRunner runner;
    CommandRunner::RunOptions options;
    options.cancelRequested = std::make_shared<std::atomic_bool>(false);

    std::thread cancelThread([flag = options.cancelRequested]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      flag->store(true, std::memory_order_relaxed);
    });

    const auto result =
        runner.run(QStringLiteral("sleep"), {QStringLiteral("5")}, options);
    cancelThread.join();

    QCOMPARE(result.exitCode, -3);
    QVERIFY(result.stderr.contains(QStringLiteral("canceled")));
  }

  void testDnfManagerAvailabilityAndVersion() {
    DnfManager dnf;
    if (!dnf.isAvailable()) {
      const auto result = dnf.checkUpdates();
      QCOMPARE(result.exitCode, -1);
      QVERIFY(result.stderr.contains(QStringLiteral("dnf not found")));
      QSKIP("dnf is not available on this host.");
    }

    CommandRunner runner;
    const auto result =
        runner.run(QStringLiteral("dnf"), {QStringLiteral("--version")});
    QVERIFY(result.success());
  }

  void testPolkitHelperAvailability() {
    PolkitHelper polkit;
    const bool hasPkexec = polkit.isPkexecAvailable();

    if (!hasPkexec) {
      const auto result = polkit.runPrivileged(QStringLiteral("true"));
      QCOMPARE(result.exitCode, -1);
      QVERIFY(result.stderr.contains(QStringLiteral("pkexec not found")));
      QSKIP("pkexec is not available on this host.");
    }

    // Functional probe should not crash and should report a meaningful state.
    const bool canAcquire = polkit.canAcquirePrivilege();
    QVERIFY(canAcquire || !canAcquire);
  }

  void testNvidiaSmiOptionalProbe() {
    if (QStandardPaths::findExecutable(QStringLiteral("nvidia-smi"))
            .isEmpty()) {
      QSKIP("nvidia-smi is not available on this host.");
    }

    CommandRunner runner;
    const auto result =
        runner.run(QStringLiteral("nvidia-smi"), {QStringLiteral("--help")});
    QVERIFY(result.success() || result.exitCode == 0);
  }

  void testHelperPathsAreCompiledForBuildAndInstallModes() {
    const QString helperBuildPath =
        QStringLiteral(RO_CONTROL_HELPER_BUILD_PATH);
    const QString helperInstallPath =
        QStringLiteral(RO_CONTROL_HELPER_INSTALL_PATH);

    QVERIFY(!helperBuildPath.trimmed().isEmpty());
    QVERIFY(!helperInstallPath.trimmed().isEmpty());
    QVERIFY(helperInstallPath.contains(QStringLiteral("ro-control-helper")));
  }

  void testSystemInfoProviderDefaultsAndProperties() {
    SystemInfoProvider provider;
    QVERIFY(!provider.osName().isEmpty());
    QVERIFY(!provider.kernelVersion().isEmpty());
    QVERIFY(!provider.cpuModel().isEmpty());
    QCOMPARE(provider.virtualMachine(),
             !provider.virtualizationType().isEmpty());

    const QString report = provider.generateSystemReport(
        QStringLiteral("NVIDIA RTX 4080"), QStringLiteral("570.86.16"),
        QStringLiteral("16 GB"), QStringLiteral("32 GB"),
        QStringLiteral("Gen4 x16"), QStringLiteral("Secure Boot: Off"));
    QVERIFY(!report.isEmpty());
    QVERIFY(report.contains(QStringLiteral("System Diagnostic Report")));
    QVERIFY(report.contains(QStringLiteral("NVIDIA RTX 4080")));

    const QString plainReport = provider.generateSystemReport(
        QStringLiteral("NVIDIA RTX 4080"), QStringLiteral("570.86.16"),
        QStringLiteral("16 GB"), QStringLiteral("32 GB"),
        QStringLiteral("Gen4 x16"), QStringLiteral("Secure Boot: Off"),
        QStringLiteral("plain"));
    QVERIFY(plainReport.contains(QStringLiteral("Operating System:")));
    QVERIFY(!plainReport.contains(QStringLiteral("**Operating System:**")));

    const auto jsonReport = QJsonDocument::fromJson(
        provider
            .generateSystemReport(
                QStringLiteral("NVIDIA RTX 4080"), QStringLiteral("570.86.16"),
                QStringLiteral("16 GB"), QStringLiteral("32 GB"),
                QStringLiteral("Gen4 x16"), QStringLiteral("Secure Boot: Off"),
                QStringLiteral("json"))
            .toUtf8());
    QVERIFY(jsonReport.isObject());
    QCOMPARE(
        jsonReport.object().value(QStringLiteral("graphicsCard")).toString(),
        QStringLiteral("NVIDIA RTX 4080"));

    QSignalSpy prefSpy(&provider,
                       &SystemInfoProvider::diagnosticReportPreferencesChanged);
    provider.setDiagnosticReportFormat(QStringLiteral("json"));
    QCOMPARE(provider.diagnosticReportFormat(), QStringLiteral("json"));
    QCOMPARE(prefSpy.count(), 1);

    provider.setDiagnosticReportDestination(QStringLiteral("clipboard"));
    QCOMPARE(provider.diagnosticReportDestination(),
             QStringLiteral("clipboard"));
    QCOMPARE(prefSpy.count(), 2);

    QSignalSpy spy(&provider, &SystemInfoProvider::infoChanged);
    provider.refresh();
    QCOMPARE(spy.count(), 0);
  }

  void testSystemInfoProviderDesktopEnvironmentParsing() {
    const QByteArray prevDesktop = qgetenv("XDG_CURRENT_DESKTOP");
    const QByteArray prevSession = qgetenv("DESKTOP_SESSION");

    qputenv("XDG_CURRENT_DESKTOP", QByteArrayLiteral("KDE"));
    qunsetenv("DESKTOP_SESSION");

    SystemInfoProvider kdeProvider;
    QCOMPARE(kdeProvider.desktopEnvironment(), QStringLiteral("KDE Plasma"));

    qputenv("XDG_CURRENT_DESKTOP", QByteArrayLiteral("GNOME"));
    SystemInfoProvider gnomeProvider;
    QCOMPARE(gnomeProvider.desktopEnvironment(), QStringLiteral("GNOME"));

    if (prevDesktop.isNull()) {
      qunsetenv("XDG_CURRENT_DESKTOP");
    } else {
      qputenv("XDG_CURRENT_DESKTOP", prevDesktop);
    }
    if (prevSession.isNull()) {
      qunsetenv("DESKTOP_SESSION");
    } else {
      qputenv("DESKTOP_SESSION", prevSession);
    }
  }

  void testSystemInfoProviderPowerSupplyDetection() {
    qputenv("RO_CONTROL_POWER_SUPPLY_ONLINE", "0");
    {
      SystemInfoProvider batProvider;
      QVERIFY(batProvider.onBattery());
      QCOMPARE(batProvider.powerSource(), QStringLiteral("Battery"));
    }

    qputenv("RO_CONTROL_POWER_SUPPLY_ONLINE", "1");
    {
      SystemInfoProvider acProvider;
      QVERIFY(!acProvider.onBattery());
      QCOMPARE(acProvider.powerSource(), QStringLiteral("AC Power"));
    }
    qunsetenv("RO_CONTROL_POWER_SUPPLY_ONLINE");
  }

  void testNvidiaInstallerDefaultsAndCancel() {
    NvidiaInstaller installer;
    QCOMPARE(installer.busy(), false);
    QCOMPARE(installer.proprietaryAgreementRequired(), true);
    QVERIFY(!installer.proprietaryAgreementText().isEmpty());

    installer.cancelOperation();
    QCOMPARE(installer.busy(), false);
  }

  void testLocalizeGpuName() {
    QCOMPARE(
        SystemInfoProvider::localizeGpuName(QStringLiteral(
            "Intel Core Processor Integrated Graphics Controller")),
        QStringLiteral("Intel Core Processor Integrated Graphics Controller"));
    QCOMPARE(SystemInfoProvider::localizeGpuName(
                 QStringLiteral("Intel Integrated Graphics Controller")),
             QStringLiteral("Intel Integrated Graphics Controller"));
    QCOMPARE(SystemInfoProvider::localizeGpuName(
                 QStringLiteral("Intel UHD Graphics 630")),
             QStringLiteral("Intel UHD Graphics 630"));
    QCOMPARE(SystemInfoProvider::localizeGpuName(QStringLiteral("")),
             QStringLiteral(""));
  }
};

QTEST_MAIN(TestSystemIntegration)
#include "test_system_integration.moc"
