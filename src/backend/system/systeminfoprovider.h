#pragma once

#include <QObject>
#include <QString>

class SystemInfoProvider : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString osName READ osName NOTIFY infoChanged)
  Q_PROPERTY(
      QString desktopEnvironment READ desktopEnvironment NOTIFY infoChanged)
  Q_PROPERTY(QString kernelVersion READ kernelVersion NOTIFY infoChanged)
  Q_PROPERTY(QString cpuModel READ cpuModel NOTIFY infoChanged)
  Q_PROPERTY(QString deviceType READ deviceType NOTIFY infoChanged)
  Q_PROPERTY(bool virtualMachine READ virtualMachine NOTIFY infoChanged)
  Q_PROPERTY(
      QString virtualizationType READ virtualizationType NOTIFY infoChanged)
  Q_PROPERTY(QString motherboardModel READ motherboardModel NOTIFY infoChanged)
  Q_PROPERTY(QString biosVersion READ biosVersion NOTIFY infoChanged)
  Q_PROPERTY(QString cudaVersion READ cudaVersion NOTIFY infoChanged)
  Q_PROPERTY(
      QString graphicsApiSummary READ graphicsApiSummary NOTIFY infoChanged)
  Q_PROPERTY(bool onBattery READ onBattery NOTIFY infoChanged)
  Q_PROPERTY(QString powerSource READ powerSource NOTIFY infoChanged)
  Q_PROPERTY(
      QString integratedGpuName READ integratedGpuName NOTIFY infoChanged)
  Q_PROPERTY(
      QString integratedGpuMemory READ integratedGpuMemory NOTIFY infoChanged)
  Q_PROPERTY(
      QString diagnosticReportFormat READ diagnosticReportFormat WRITE
          setDiagnosticReportFormat NOTIFY diagnosticReportPreferencesChanged)
  Q_PROPERTY(
      QString diagnosticReportDestination READ diagnosticReportDestination WRITE
          setDiagnosticReportDestination NOTIFY
              diagnosticReportPreferencesChanged)

public:
  explicit SystemInfoProvider(QObject *parent = nullptr);

  QString osName() const { return m_osName; }
  QString desktopEnvironment() const { return m_desktopEnvironment; }
  QString kernelVersion() const { return m_kernelVersion; }
  QString cpuModel() const { return m_cpuModel; }
  QString motherboardModel() const { return m_motherboardModel; }
  QString biosVersion() const { return m_biosVersion; }
  QString cudaVersion() const { return m_cudaVersion; }
  QString graphicsApiSummary() const { return m_graphicsApiSummary; }
  QString deviceType() const { return m_deviceType; }
  bool virtualMachine() const { return !m_virtualizationType.isEmpty(); }
  QString virtualizationType() const { return m_virtualizationType; }
  bool onBattery() const { return m_onBattery; }
  QString powerSource() const { return m_powerSource; }
  QString integratedGpuName() const {
    return localizeGpuName(m_integratedGpuName);
  }
  QString integratedGpuMemory() const { return m_integratedGpuMemory; }
  QString diagnosticReportFormat() const { return m_diagnosticReportFormat; }
  QString diagnosticReportDestination() const {
    return m_diagnosticReportDestination;
  }

  Q_INVOKABLE static QString localizeGpuName(const QString &rawName);

  Q_INVOKABLE void refresh();
  Q_INVOKABLE void rescanHardware();
  Q_INVOKABLE bool requestRestart();
  Q_INVOKABLE bool requestRebootToFirmware();
  Q_INVOKABLE bool copyToClipboard(const QString &text);
  Q_INVOKABLE void setDiagnosticReportFormat(const QString &format);
  Q_INVOKABLE void setDiagnosticReportDestination(const QString &destination);
  Q_INVOKABLE QString generateSystemReport(
      const QString &gpuName = QString(), const QString &driverVer = QString(),
      const QString &vramStr = QString(), const QString &ramStr = QString(),
      const QString &pcieStr = QString(), const QString &secureBoot = QString(),
      const QString &format = QStringLiteral("markdown"));

signals:
  void infoChanged();
  void diagnosticReportPreferencesChanged();

private:
  QString detectOsName() const;
  QString detectKernelVersion() const;
  QString detectCpuModel() const;
  QString detectMotherboardModel() const;
  QString detectBiosVersion() const;
  QString detectCudaVersion() const;
  QString detectGraphicsApiSummary() const;
  QString detectDeviceType() const;
  QString detectDesktopEnvironment() const;
  QString detectVirtualizationType() const;
  QString detectIntegratedGpuName() const;
  QString detectIntegratedGpuMemory() const;
  bool detectOnBattery(QString *sourceLabel = nullptr) const;
  void initializeStaticInfo();
  void loadDiagnosticReportPreferences();
  void saveDiagnosticReportPreferences() const;

  QString m_osName;
  QString m_desktopEnvironment;
  QString m_kernelVersion;
  QString m_cpuModel;
  QString m_motherboardModel;
  QString m_biosVersion;
  QString m_cudaVersion;
  QString m_graphicsApiSummary;
  QString m_deviceType;
  QString m_virtualizationType;
  bool m_onBattery = false;
  bool m_staticHardwareLoaded = false;
  QString m_powerSource;
  QString m_integratedGpuName;
  QString m_integratedGpuMemory;
  QString m_diagnosticReportFormat = QStringLiteral("markdown");
  QString m_diagnosticReportDestination = QStringLiteral("preview");
};
