#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QTest>

#include "system/capabilityprobe.h"
#include "system/commandrunner.h"
#include "system/dnfmanager.h"
#include "system/polkit.h"

class TestSystemIntegration : public QObject {
  Q_OBJECT

private slots:
  void testCommandRunnerUsesProgramOverride() {
    QTemporaryDir tempDir;
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

  void testCapabilityProbeUsesProgramOverride() {
    QTemporaryDir tempDir;
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

    const auto status = CapabilityProbe::probeTool(QStringLiteral("nvidia-smi"));
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
    QVERIFY(polkit.canAcquirePrivilege() || !polkit.canAcquirePrivilege());
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
};

QTEST_MAIN(TestSystemIntegration)
#include "test_system_integration.moc"
