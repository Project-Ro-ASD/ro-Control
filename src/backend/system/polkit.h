#pragma once

#include <QObject>
#include <QStringList>

#include "commandrunner.h"

// PolicyKit yetki yükseltme
class PolkitHelper : public QObject {
  Q_OBJECT

public:
  explicit PolkitHelper(QObject *parent = nullptr);

  Q_INVOKABLE bool isPkexecAvailable() const;
  Q_INVOKABLE CommandRunner::Result runPrivileged(const QString &program,
                                                  const QStringList &args = {});
  Q_INVOKABLE bool canAcquirePrivilege();

signals:
  void progressMessage(const QString &message);

private:
  CommandRunner m_runner;
};
