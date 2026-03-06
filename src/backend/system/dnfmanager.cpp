#include "dnfmanager.h"
#include "commandrunner.h"

DnfManager::DnfManager(QObject *parent) : QObject(parent) {}

bool DnfManager::isInstalled(const QString &packageName) const {
  CommandRunner runner;

  const auto result =
      runner.run(QStringLiteral("rpm"), {QStringLiteral("-q"), packageName});

  return result.success();
}

bool DnfManager::install(const QString &packageName) {
  return install(QStringList{packageName});
}

bool DnfManager::install(const QStringList &packages) {
  CommandRunner runner;

  connect(&runner, &CommandRunner::outputLine, this, &DnfManager::outputLine);

  QStringList args;
  args << QStringLiteral("install") << QStringLiteral("-y") << packages;

  const auto result = runner.runAsRoot(QStringLiteral("dnf"), args);
  return result.success();
}

bool DnfManager::remove(const QString &packageName) {
  return remove(QStringList{packageName});
}

bool DnfManager::remove(const QStringList &packages) {
  CommandRunner runner;

  connect(&runner, &CommandRunner::outputLine, this, &DnfManager::outputLine);

  QStringList args;
  args << QStringLiteral("remove") << QStringLiteral("-y") << packages;

  const auto result = runner.runAsRoot(QStringLiteral("dnf"), args);
  return result.success();
}

bool DnfManager::enableRepo(const QString &repoUrl) {
  CommandRunner runner;

  connect(&runner, &CommandRunner::outputLine, this, &DnfManager::outputLine);

  const auto result =
      runner.runAsRoot(QStringLiteral("dnf"), {QStringLiteral("install"),
                                               QStringLiteral("-y"), repoUrl});

  return result.success();
}

DnfManager::PackageInfo
DnfManager::queryPackage(const QString &packageName) const {
  CommandRunner runner;

  // rpm -qi ile paket bilgisini al
  const auto result =
      runner.run(QStringLiteral("rpm"), {QStringLiteral("-qi"), packageName});

  PackageInfo info;
  if (!result.success())
    return info;

  info.name = packageName;

  const QStringList lines = result.stdout.split(QLatin1Char('\n'));
  for (const QString &line : lines) {
    if (line.startsWith(QStringLiteral("Version"))) {
      const int colon = line.indexOf(QLatin1Char(':'));
      if (colon >= 0)
        info.version = line.mid(colon + 1).trimmed();
    } else if (line.startsWith(QStringLiteral("Summary"))) {
      const int colon = line.indexOf(QLatin1Char(':'));
      if (colon >= 0)
        info.summary = line.mid(colon + 1).trimmed();
    }
  }

  return info;
}

bool DnfManager::cleanCache() {
  CommandRunner runner;

  connect(&runner, &CommandRunner::outputLine, this, &DnfManager::outputLine);

  const auto result = runner.runAsRoot(
      QStringLiteral("dnf"), {QStringLiteral("clean"), QStringLiteral("all")});

  return result.success();
}
