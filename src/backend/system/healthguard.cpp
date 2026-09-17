#include "healthguard.h"

#include <QDateTime>
#include <QSettings>

namespace {

constexpr auto kSettingsGroup = "HealthGuard";
constexpr auto kKeyNotifications = "NotificationsEnabled";
constexpr auto kKeyGpuWarning = "GpuWarningThreshold";
constexpr auto kKeyGpuCritical = "GpuCriticalThreshold";
constexpr auto kKeyCpuWarning = "CpuWarningThreshold";
constexpr auto kKeyCpuCritical = "CpuCriticalThreshold";
constexpr qint64 kAlertCooldownSecs = 60;

} // namespace

HealthGuard::HealthGuard(QObject *parent) : QObject(parent) { loadSettings(); }

HealthGuard::~HealthGuard() { saveSettings(); }

bool HealthGuard::notificationsEnabled() const {
  return m_notificationsEnabled;
}

int HealthGuard::gpuWarningThresholdC() const { return m_gpuWarningThresholdC; }

int HealthGuard::gpuCriticalThresholdC() const {
  return m_gpuCriticalThresholdC;
}

int HealthGuard::cpuWarningThresholdC() const { return m_cpuWarningThresholdC; }

int HealthGuard::cpuCriticalThresholdC() const {
  return m_cpuCriticalThresholdC;
}

bool HealthGuard::alertActive() const { return m_alertActive; }

QString HealthGuard::lastAlertTitle() const { return m_lastAlertTitle; }

QString HealthGuard::lastAlertMessage() const { return m_lastAlertMessage; }

QString HealthGuard::lastAlertSeverity() const { return m_lastAlertSeverity; }

void HealthGuard::setNotificationsEnabled(bool enabled) {
  if (m_notificationsEnabled != enabled) {
    m_notificationsEnabled = enabled;
    emit notificationsEnabledChanged();
    saveSettings();
  }
}

void HealthGuard::setGpuWarningThresholdC(int tempC) {
  if (tempC > 40 && tempC < 110 && m_gpuWarningThresholdC != tempC) {
    m_gpuWarningThresholdC = tempC;
    emit thresholdsChanged();
    saveSettings();
  }
}

void HealthGuard::setGpuCriticalThresholdC(int tempC) {
  if (tempC > 45 && tempC < 115 && m_gpuCriticalThresholdC != tempC) {
    m_gpuCriticalThresholdC = tempC;
    emit thresholdsChanged();
    saveSettings();
  }
}

void HealthGuard::setCpuWarningThresholdC(int tempC) {
  if (tempC > 40 && tempC < 115 && m_cpuWarningThresholdC != tempC) {
    m_cpuWarningThresholdC = tempC;
    emit thresholdsChanged();
    saveSettings();
  }
}

void HealthGuard::setCpuCriticalThresholdC(int tempC) {
  if (tempC > 50 && tempC < 125 && m_cpuCriticalThresholdC != tempC) {
    m_cpuCriticalThresholdC = tempC;
    emit thresholdsChanged();
    saveSettings();
  }
}

void HealthGuard::clearAlert() {
  if (m_alertActive) {
    m_alertActive = false;
    m_lastAlertSeverity = QStringLiteral("normal");
    m_lastGpuAlertTime = 0;
    m_lastCpuAlertTime = 0;
    emit alertActiveChanged();
    emit alertChanged();
  }
}

void HealthGuard::loadSettings() {
  QSettings settings;
  settings.beginGroup(QString::fromLatin1(kSettingsGroup));
  m_notificationsEnabled =
      settings.value(QString::fromLatin1(kKeyNotifications), true).toBool();
  m_gpuWarningThresholdC =
      settings.value(QString::fromLatin1(kKeyGpuWarning), 82).toInt();
  m_gpuCriticalThresholdC =
      settings.value(QString::fromLatin1(kKeyGpuCritical), 88).toInt();
  m_cpuWarningThresholdC =
      settings.value(QString::fromLatin1(kKeyCpuWarning), 85).toInt();
  m_cpuCriticalThresholdC =
      settings.value(QString::fromLatin1(kKeyCpuCritical), 95).toInt();
  settings.endGroup();
}

void HealthGuard::saveSettings() {
  QSettings settings;
  settings.beginGroup(QString::fromLatin1(kSettingsGroup));
  settings.setValue(QString::fromLatin1(kKeyNotifications),
                    m_notificationsEnabled);
  settings.setValue(QString::fromLatin1(kKeyGpuWarning),
                    m_gpuWarningThresholdC);
  settings.setValue(QString::fromLatin1(kKeyGpuCritical),
                    m_gpuCriticalThresholdC);
  settings.setValue(QString::fromLatin1(kKeyCpuWarning),
                    m_cpuWarningThresholdC);
  settings.setValue(QString::fromLatin1(kKeyCpuCritical),
                    m_cpuCriticalThresholdC);
  settings.endGroup();
}

void HealthGuard::updateGpuTemperature(int tempC) {
  if (tempC <= 0) {
    return;
  }
  m_currentGpuTempC = tempC;
  evaluateAlerts();
}

void HealthGuard::updateCpuTemperature(int tempC) {
  if (tempC <= 0) {
    return;
  }
  m_currentCpuTempC = tempC;
  evaluateAlerts();
}

void HealthGuard::evaluateAlerts() {
  const qint64 now = QDateTime::currentSecsSinceEpoch();

  // GPU check
  if (m_currentGpuTempC >= m_gpuCriticalThresholdC) {
    if (now - m_lastGpuAlertTime > kAlertCooldownSecs) {
      m_lastGpuAlertTime = now;
      m_alertActive = true;
      m_lastAlertTitle = tr("GPU Critical Temperature Alert");
      m_lastAlertMessage =
          tr("GPU temperature reached %1°C (critical limit: %2°C). "
             "Cooling is throttled.")
              .arg(m_currentGpuTempC)
              .arg(m_gpuCriticalThresholdC);
      m_lastAlertSeverity = QStringLiteral("critical");
      emit alertActiveChanged();
      emit alertChanged();
      if (m_notificationsEnabled) {
        emit thermalAlertTriggered(m_lastAlertTitle, m_lastAlertMessage,
                                   m_lastAlertSeverity);
      }
    }
  } else if (m_currentGpuTempC >= m_gpuWarningThresholdC) {
    if (now - m_lastGpuAlertTime > kAlertCooldownSecs) {
      m_lastGpuAlertTime = now;
      m_alertActive = true;
      m_lastAlertTitle = tr("GPU High Temperature Warning");
      m_lastAlertMessage = tr("GPU temperature is %1°C (warning limit: %2°C).")
                               .arg(m_currentGpuTempC)
                               .arg(m_gpuWarningThresholdC);
      m_lastAlertSeverity = QStringLiteral("warning");
      emit alertActiveChanged();
      emit alertChanged();
      if (m_notificationsEnabled) {
        emit thermalAlertTriggered(m_lastAlertTitle, m_lastAlertMessage,
                                   m_lastAlertSeverity);
      }
    }
  } else if (m_currentCpuTempC >= m_cpuCriticalThresholdC) {
    if (now - m_lastCpuAlertTime > kAlertCooldownSecs) {
      m_lastCpuAlertTime = now;
      m_alertActive = true;
      m_lastAlertTitle = tr("CPU Critical Temperature Alert");
      m_lastAlertMessage =
          tr("CPU temperature reached %1°C (critical limit: %2°C).")
              .arg(m_currentCpuTempC)
              .arg(m_cpuCriticalThresholdC);
      m_lastAlertSeverity = QStringLiteral("critical");
      emit alertActiveChanged();
      emit alertChanged();
      if (m_notificationsEnabled) {
        emit thermalAlertTriggered(m_lastAlertTitle, m_lastAlertMessage,
                                   m_lastAlertSeverity);
      }
    }
  } else if (m_currentCpuTempC >= m_cpuWarningThresholdC) {
    if (now - m_lastCpuAlertTime > kAlertCooldownSecs) {
      m_lastCpuAlertTime = now;
      m_alertActive = true;
      m_lastAlertTitle = tr("CPU High Temperature Warning");
      m_lastAlertMessage = tr("CPU temperature is %1°C (warning limit: %2°C).")
                               .arg(m_currentCpuTempC)
                               .arg(m_cpuWarningThresholdC);
      m_lastAlertSeverity = QStringLiteral("warning");
      emit alertActiveChanged();
      emit alertChanged();
      if (m_notificationsEnabled) {
        emit thermalAlertTriggered(m_lastAlertTitle, m_lastAlertMessage,
                                   m_lastAlertSeverity);
      }
    }
  } else if (m_alertActive &&
             m_currentGpuTempC < (m_gpuWarningThresholdC - 4) &&
             m_currentCpuTempC < (m_cpuWarningThresholdC - 4)) {
    clearAlert();
  }
}
