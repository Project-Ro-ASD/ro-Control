#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>

class HealthGuard : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool notificationsEnabled READ notificationsEnabled WRITE
                 setNotificationsEnabled NOTIFY notificationsEnabledChanged)
  Q_PROPERTY(int gpuWarningThresholdC READ gpuWarningThresholdC WRITE
                 setGpuWarningThresholdC NOTIFY thresholdsChanged)
  Q_PROPERTY(int gpuCriticalThresholdC READ gpuCriticalThresholdC WRITE
                 setGpuCriticalThresholdC NOTIFY thresholdsChanged)
  Q_PROPERTY(int cpuWarningThresholdC READ cpuWarningThresholdC WRITE
                 setCpuWarningThresholdC NOTIFY thresholdsChanged)
  Q_PROPERTY(int cpuCriticalThresholdC READ cpuCriticalThresholdC WRITE
                 setCpuCriticalThresholdC NOTIFY thresholdsChanged)
  Q_PROPERTY(bool alertActive READ alertActive NOTIFY alertActiveChanged)
  Q_PROPERTY(QString lastAlertTitle READ lastAlertTitle NOTIFY alertChanged)
  Q_PROPERTY(QString lastAlertMessage READ lastAlertMessage NOTIFY alertChanged)
  Q_PROPERTY(
      QString lastAlertSeverity READ lastAlertSeverity NOTIFY alertChanged)

public:
  explicit HealthGuard(QObject *parent = nullptr);
  ~HealthGuard() override;

  bool notificationsEnabled() const;
  int gpuWarningThresholdC() const;
  int gpuCriticalThresholdC() const;
  int cpuWarningThresholdC() const;
  int cpuCriticalThresholdC() const;
  bool alertActive() const;
  QString lastAlertTitle() const;
  QString lastAlertMessage() const;
  QString lastAlertSeverity() const;

  Q_INVOKABLE void setNotificationsEnabled(bool enabled);
  Q_INVOKABLE void setGpuWarningThresholdC(int tempC);
  Q_INVOKABLE void setGpuCriticalThresholdC(int tempC);
  Q_INVOKABLE void setCpuWarningThresholdC(int tempC);
  Q_INVOKABLE void setCpuCriticalThresholdC(int tempC);
  Q_INVOKABLE void clearAlert();

  Q_INVOKABLE void updateGpuTemperature(int tempC);
  Q_INVOKABLE void updateCpuTemperature(int tempC);

signals:
  void notificationsEnabledChanged();
  void thresholdsChanged();
  void alertActiveChanged();
  void alertChanged();
  void thermalAlertTriggered(const QString &title, const QString &message,
                             const QString &severity);

private:
  void loadSettings();
  void saveSettings();
  void evaluateAlerts();

  bool m_notificationsEnabled = true;
  int m_gpuWarningThresholdC = 82;
  int m_gpuCriticalThresholdC = 88;
  int m_cpuWarningThresholdC = 85;
  int m_cpuCriticalThresholdC = 95;

  int m_currentGpuTempC = 0;
  int m_currentCpuTempC = 0;
  bool m_alertActive = false;
  QString m_lastAlertTitle;
  QString m_lastAlertMessage;
  QString m_lastAlertSeverity = QStringLiteral("normal");

  qint64 m_lastGpuAlertTime = 0;
  qint64 m_lastCpuAlertTime = 0;
};
