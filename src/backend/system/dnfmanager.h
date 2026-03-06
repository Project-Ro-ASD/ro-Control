#pragma once

#include <QObject>
#include <QStringList>

#include "commandrunner.h"

// DNF paket yöneticisi wrapper
class DnfManager : public QObject {
  Q_OBJECT

public:
  explicit DnfManager(QObject *parent = nullptr);

  Q_INVOKABLE bool isAvailable() const;
  Q_INVOKABLE CommandRunner::Result
  checkUpdates(const QStringList &packages = {});
  Q_INVOKABLE CommandRunner::Result
  installPackages(const QStringList &packages);
  Q_INVOKABLE CommandRunner::Result removePackages(const QStringList &packages);
  Q_INVOKABLE CommandRunner::Result
  updatePackages(const QStringList &packages = {});
  Q_INVOKABLE CommandRunner::Result cleanAll();

signals:
  void progressMessage(const QString &message);

private:
  CommandRunner m_runner;
};
