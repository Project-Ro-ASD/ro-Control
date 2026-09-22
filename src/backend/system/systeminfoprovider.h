#pragma once

#include <QFuture>
#include <QObject>
#include <QString>
#include <QTimer>

#include <atomic>
#include <memory>

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
  Q_PROPERTY(QString resizableBarStatus READ resizableBarStatus NOTIFY infoChanged)
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
  ~SystemInfoProvider() override;

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
  QString resizableBarStatus() const { return m_resizableBarStatus; }
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
  // Privileged actions (reboot / firmware reboot) run off the GUI thread so
  // the pkexec authorization dialog never freezes the window. Both return
  // true when the request was accepted; the rootActionFinished signal then
  // reports the actual outcome.
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
  // action is "reboot" or "firmware-reboot".
  void rootActionFinished(bool success, const QString &action);

private:
  // Probes which shell out (lspci, vulkaninfo, nvidia-smi) are collected on a
  // worker thread and never touch instance state, so they are safe to run
  // while the rest of the object is being torn down.
  struct HardwareScanData {
    QString integratedGpuName;
    QString integratedGpuMemory;
    QString cudaVersion;
    QString graphicsApiSummary;
  };

  QString detectOsName() const;
  QString detectKernelVersion() const;
  QString detectCpuModel() const;
  QString detectMotherboardModel() const;
  QString detectBiosVersion() const;
  static QString detectCudaVersion();
  static QString detectGraphicsApiSummary(const QString &cudaVersion);
  QString detectDeviceType() const;
  QString detectDesktopEnvironment() const;
  QString detectVirtualizationType() const;
  static QString detectIntegratedGpuName();
  static QString detectIntegratedGpuMemory(const QString &integratedGpuName);
  QString detectResizableBarStatus() const;
  bool detectOnBattery(QString *sourceLabel = nullptr) const;
  void initializeStaticInfo();
  void startHardwareScan();
  bool startRootAction(const QString &action);
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
  QString m_resizableBarStatus;
  QString m_integratedGpuName;
  QString m_integratedGpuMemory;
  QString m_diagnosticReportFormat = QStringLiteral("markdown");
  QString m_diagnosticReportDestination = QStringLiteral("preview");

  QTimer m_powerTimer;
  QFuture<void> m_hardwareScanFuture;
  QFuture<void> m_rootActionFuture;
  quint64 m_hardwareScanGeneration = 0;
  bool m_rootActionInProgress = false;
  // Set from the destructor to abort a pending pkexec wait.
  std::shared_ptr<std::atomic_bool> m_rootActionCancel =
      std::make_shared<std::atomic_bool>(false);
};
