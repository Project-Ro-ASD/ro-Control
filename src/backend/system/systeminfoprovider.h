#pragma once

#include <QObject>
#include <QString>

class SystemInfoProvider : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString osName READ osName NOTIFY infoChanged)
  Q_PROPERTY(QString desktopEnvironment READ desktopEnvironment NOTIFY infoChanged)
  Q_PROPERTY(QString kernelVersion READ kernelVersion NOTIFY infoChanged)
  Q_PROPERTY(QString cpuModel READ cpuModel NOTIFY infoChanged)

public:
  explicit SystemInfoProvider(QObject *parent = nullptr);

  QString osName() const { return m_osName; }
  QString desktopEnvironment() const { return m_desktopEnvironment; }
  QString kernelVersion() const { return m_kernelVersion; }
  QString cpuModel() const { return m_cpuModel; }

  Q_INVOKABLE void refresh();

signals:
  void infoChanged();

private:
  QString detectOsName() const;
  QString detectKernelVersion() const;
  QString detectCpuModel() const;
  QString detectDesktopEnvironment() const;

  QString m_osName;
  QString m_desktopEnvironment;
  QString m_kernelVersion;
  QString m_cpuModel;
};
