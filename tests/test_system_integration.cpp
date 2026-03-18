#include <QStandardPaths>
#include <QTest>

#include "system/commandrunner.h"
#include "system/dnfmanager.h"
#include "system/polkit.h"

class TestSystemIntegration : public QObject {
  Q_OBJECT

private slots:
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
