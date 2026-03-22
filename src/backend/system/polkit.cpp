// Yetki yukseltme

#include "polkit.h"

#include "capabilityprobe.h"

PolkitHelper::PolkitHelper(QObject *parent) : QObject(parent) {
  connect(&m_runner, &CommandRunner::outputLine, this,
          &PolkitHelper::progressMessage);
  connect(&m_runner, &CommandRunner::errorLine, this,
          &PolkitHelper::progressMessage);
}

bool PolkitHelper::isPkexecAvailable() const {
  return CapabilityProbe::isToolAvailable(QStringLiteral("pkexec"));
}

CommandRunner::Result PolkitHelper::runPrivileged(const QString &program,
                                                  const QStringList &args) {
  if (!isPkexecAvailable()) {
    return CommandRunner::Result{.exitCode = -1,
                                 .stdout = {},
                                 .stderr = QStringLiteral("pkexec not found.")};
  }

  return m_runner.runAsRoot(program, args);
}

bool PolkitHelper::canAcquirePrivilege() {
  if (!isPkexecAvailable()) {
    return false;
  }

  CommandRunner::RunOptions options;
  options.timeoutMs = 4000;
  const auto result = m_runner.run(
      QStringLiteral("pkexec"),
      {QStringLiteral("--disable-internal-agent"), QStringLiteral("true")},
      options);

  // Success means privilege is available, but interactive-denied/auth-failed
  // cases can return non-zero and still indicate pkexec is functional.
  return result.exitCode == 0 || result.exitCode == 126 ||
         result.exitCode == 127;
}
