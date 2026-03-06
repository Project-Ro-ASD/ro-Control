// DNF paket yoneticisi

#include "dnfmanager.h"

#include <QStandardPaths>

DnfManager::DnfManager(QObject *parent) : QObject(parent) {
  connect(&m_runner, &CommandRunner::outputLine, this,
          &DnfManager::progressMessage);
  connect(&m_runner, &CommandRunner::errorLine, this,
          &DnfManager::progressMessage);
}

bool DnfManager::isAvailable() const {
  return !QStandardPaths::findExecutable(QStringLiteral("dnf")).isEmpty();
}

CommandRunner::Result DnfManager::checkUpdates(const QStringList &packages) {
  QStringList args{QStringLiteral("check-update")};
  args << packages;
  return m_runner.run(QStringLiteral("dnf"), args);
}

CommandRunner::Result DnfManager::installPackages(const QStringList &packages) {
  if (packages.isEmpty()) {
    return CommandRunner::Result{
        .exitCode = -1,
        .stdout = {},
        .stderr = QStringLiteral("No packages provided for install.")};
  }

  QStringList args{QStringLiteral("install"), QStringLiteral("-y")};
  args << packages;
  return m_runner.runAsRoot(QStringLiteral("dnf"), args);
}

CommandRunner::Result DnfManager::removePackages(const QStringList &packages) {
  if (packages.isEmpty()) {
    return CommandRunner::Result{
        .exitCode = -1,
        .stdout = {},
        .stderr = QStringLiteral("No packages provided for remove.")};
  }

  QStringList args{QStringLiteral("remove"), QStringLiteral("-y")};
  args << packages;
  return m_runner.runAsRoot(QStringLiteral("dnf"), args);
}

CommandRunner::Result DnfManager::updatePackages(const QStringList &packages) {
  QStringList args{QStringLiteral("update"), QStringLiteral("-y")};
  args << packages;
  return m_runner.runAsRoot(QStringLiteral("dnf"), args);
}

CommandRunner::Result DnfManager::cleanAll() {
  return m_runner.runAsRoot(QStringLiteral("dnf"),
                            {QStringLiteral("clean"), QStringLiteral("all")});
}
