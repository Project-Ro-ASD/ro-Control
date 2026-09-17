#include "dbusservice.h"

#include <QDBusConnection>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "fan/fancontroller.h"
#include "healthguard.h"
#include "monitor/cpumonitor.h"
#include "monitor/gpumonitor.h"
#include "monitor/rammonitor.h"
#include "power/powercontroller.h"

namespace {

constexpr auto kServiceName = "io.github.ProjectRoASD.rocontrol";
constexpr auto kObjectPath = "/io/github/ProjectRoASD/rocontrol";

} // namespace

RoControlDBusAdaptor::RoControlDBusAdaptor(QObject *parent, CpuMonitor *cpu,
                                           GpuMonitor *gpu, RamMonitor *ram,
                                           FanController *fan,
                                           PowerController *power,
                                           HealthGuard *guard)
    : QDBusAbstractAdaptor(parent), m_cpu(cpu), m_gpu(gpu), m_ram(ram),
      m_fan(fan), m_power(power), m_guard(guard) {
  setAutoRelaySignals(true);

  if (m_guard) {
    connect(m_guard, &HealthGuard::thermalAlertTriggered, this,
            [this](const QString &title, const QString &message,
                   const QString &severity) {
              Q_UNUSED(message);
              const int temp =
                  (title.contains(QStringLiteral("GPU"), Qt::CaseInsensitive))
                      ? (m_gpu ? m_gpu->temperatureC() : 0)
                      : (m_cpu ? m_cpu->temperatureC() : 0);
              const int threshold =
                  (title.contains(QStringLiteral("GPU"), Qt::CaseInsensitive))
                      ? (m_guard ? (severity == QStringLiteral("critical")
                                        ? m_guard->gpuCriticalThresholdC()
                                        : m_guard->gpuWarningThresholdC())
                                 : 85)
                      : (m_guard ? (severity == QStringLiteral("critical")
                                        ? m_guard->cpuCriticalThresholdC()
                                        : m_guard->cpuWarningThresholdC())
                                 : 85);
              emit ThermalAlert(title, temp, threshold);
            });
  }

  if (m_gpu) {
    connect(m_gpu, &GpuMonitor::temperatureCChanged, this,
            &RoControlDBusAdaptor::TelemetryUpdated);
  }
}

QString RoControlDBusAdaptor::GetTelemetry() {
  QJsonObject root;
  root.insert(QStringLiteral("timestamp"),
              QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

  // CPU
  QJsonObject cpuObj;
  if (m_cpu) {
    cpuObj.insert(QStringLiteral("available"), m_cpu->available());
    cpuObj.insert(QStringLiteral("usagePercent"), m_cpu->usagePercent());
    cpuObj.insert(QStringLiteral("temperatureC"), m_cpu->temperatureC());
  }
  root.insert(QStringLiteral("cpu"), cpuObj);

  // GPU
  QJsonObject gpuObj;
  if (m_gpu) {
    gpuObj.insert(QStringLiteral("available"), m_gpu->available());
    gpuObj.insert(QStringLiteral("name"), m_gpu->gpuName());
    gpuObj.insert(QStringLiteral("temperatureC"), m_gpu->temperatureC());
    gpuObj.insert(QStringLiteral("hotspotTemperatureC"),
                  m_gpu->hotspotTemperatureC());
    gpuObj.insert(QStringLiteral("memoryTemperatureC"),
                  m_gpu->memoryTemperatureC());
    gpuObj.insert(QStringLiteral("utilizationPercent"),
                  m_gpu->utilizationPercent());
    gpuObj.insert(QStringLiteral("memoryUsedMiB"), m_gpu->memoryUsedMiB());
    gpuObj.insert(QStringLiteral("memoryTotalMiB"), m_gpu->memoryTotalMiB());
    gpuObj.insert(QStringLiteral("memoryUsagePercent"),
                  m_gpu->memoryUsagePercent());
    gpuObj.insert(QStringLiteral("fanSpeedPercent"), m_gpu->fanSpeedPercent());
    gpuObj.insert(QStringLiteral("powerDrawW"), m_gpu->powerDrawW());
    gpuObj.insert(QStringLiteral("powerLimitW"), m_gpu->powerLimitW());
    gpuObj.insert(QStringLiteral("graphicsClockMHz"),
                  m_gpu->graphicsClockMHz());
    gpuObj.insert(QStringLiteral("memoryClockMHz"), m_gpu->memoryClockMHz());
    gpuObj.insert(QStringLiteral("pcieLinkStatus"), m_gpu->pcieLinkStatus());
    gpuObj.insert(QStringLiteral("processCount"), m_gpu->gpuProcessCount());
    gpuObj.insert(QStringLiteral("gpuCount"), m_gpu->gpuCount());
    gpuObj.insert(QStringLiteral("selectedGpuIndex"),
                  m_gpu->selectedGpuIndex());
  }
  root.insert(QStringLiteral("gpu"), gpuObj);

  // RAM
  QJsonObject ramObj;
  if (m_ram) {
    ramObj.insert(QStringLiteral("available"), m_ram->available());
    ramObj.insert(QStringLiteral("totalMiB"), m_ram->totalMiB());
    ramObj.insert(QStringLiteral("usedMiB"), m_ram->usedMiB());
    ramObj.insert(QStringLiteral("usagePercent"), m_ram->usagePercent());
  }
  root.insert(QStringLiteral("ram"), ramObj);

  // Fan
  QJsonObject fanObj;
  if (m_fan) {
    fanObj.insert(QStringLiteral("supported"), m_fan->supported());
    fanObj.insert(QStringLiteral("controlSupported"),
                  m_fan->controlSupported());
    fanObj.insert(QStringLiteral("mode"), m_fan->fanMode());
    fanObj.insert(QStringLiteral("targetSpeedPercent"),
                  m_fan->targetFanSpeedPercent());
    fanObj.insert(QStringLiteral("rpm"), m_fan->currentRpm());
    fanObj.insert(QStringLiteral("safetyOverride"),
                  m_fan->safetyOverrideActive());
    fanObj.insert(QStringLiteral("smoothingEnabled"),
                  m_fan->smoothingEnabled());
    fanObj.insert(QStringLiteral("hysteresisTempC"), m_fan->hysteresisTempC());
  }
  root.insert(QStringLiteral("fan"), fanObj);

  // Power
  QJsonObject powerObj;
  if (m_power) {
    powerObj.insert(QStringLiteral("supported"), m_power->supported());
    powerObj.insert(QStringLiteral("controlSupported"),
                    m_power->controlSupported());
    powerObj.insert(QStringLiteral("powerDrawW"), m_power->currentPowerDrawW());
    powerObj.insert(QStringLiteral("powerLimitW"), m_power->powerLimitW());
    powerObj.insert(QStringLiteral("minLimitW"), m_power->minPowerLimitW());
    powerObj.insert(QStringLiteral("maxLimitW"), m_power->maxPowerLimitW());
    powerObj.insert(QStringLiteral("preset"), m_power->powerPreset());
    powerObj.insert(QStringLiteral("persistenceMode"),
                    m_power->persistenceModeEnabled());
    powerObj.insert(QStringLiteral("coreClockOffsetMHz"),
                    m_power->coreClockOffsetMHz());
    powerObj.insert(QStringLiteral("memoryClockOffsetMHz"),
                    m_power->memoryClockOffsetMHz());
  }
  root.insert(QStringLiteral("power"), powerObj);

  return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

QString RoControlDBusAdaptor::GetThermalStatus() {
  QJsonObject root;
  root.insert(QStringLiteral("gpuTemperatureC"),
              m_gpu ? m_gpu->temperatureC() : 0);
  root.insert(QStringLiteral("gpuHotspotTemperatureC"),
              m_gpu ? m_gpu->hotspotTemperatureC() : 0);
  root.insert(QStringLiteral("gpuMemoryTemperatureC"),
              m_gpu ? m_gpu->memoryTemperatureC() : 0);
  root.insert(QStringLiteral("cpuTemperatureC"),
              m_cpu ? m_cpu->temperatureC() : 0);
  root.insert(QStringLiteral("fanRpm"), m_fan ? m_fan->currentRpm() : 0);
  root.insert(QStringLiteral("fanSpeedPercent"),
              m_fan ? m_fan->currentFanSpeedPercent() : 0);
  root.insert(QStringLiteral("alertActive"),
              m_guard ? m_guard->alertActive() : false);
  root.insert(QStringLiteral("lastAlertTitle"),
              m_guard ? m_guard->lastAlertTitle() : QString{});
  root.insert(QStringLiteral("lastAlertSeverity"),
              m_guard ? m_guard->lastAlertSeverity()
                      : QStringLiteral("normal"));
  return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

QString RoControlDBusAdaptor::GetGpuHealth() {
  QJsonObject root;
  if (m_gpu) {
    root.insert(QStringLiteral("gpuName"), m_gpu->gpuName());
    root.insert(QStringLiteral("temperatureC"), m_gpu->temperatureC());
    root.insert(QStringLiteral("hotspotTemperatureC"),
                m_gpu->hotspotTemperatureC());
    root.insert(QStringLiteral("memoryTemperatureC"),
                m_gpu->memoryTemperatureC());
    root.insert(QStringLiteral("utilizationPercent"),
                m_gpu->utilizationPercent());
    root.insert(QStringLiteral("powerDrawW"), m_gpu->powerDrawW());
    root.insert(QStringLiteral("powerLimitW"), m_gpu->powerLimitW());
    root.insert(QStringLiteral("pcieLinkStatus"), m_gpu->pcieLinkStatus());
    root.insert(QStringLiteral("processCount"), m_gpu->gpuProcessCount());
    root.insert(QStringLiteral("gpuCount"), m_gpu->gpuCount());
  }
  if (m_power) {
    root.insert(QStringLiteral("persistenceMode"),
                m_power->persistenceModeEnabled());
    root.insert(QStringLiteral("powerPreset"), m_power->powerPreset());
    root.insert(QStringLiteral("coreClockOffsetMHz"),
                m_power->coreClockOffsetMHz());
    root.insert(QStringLiteral("memoryClockOffsetMHz"),
                m_power->memoryClockOffsetMHz());
  }
  return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

QString RoControlDBusAdaptor::GetGpuProcesses() {
  QJsonObject root;
  QJsonArray arr;
  if (m_gpu) {
    const auto procs = m_gpu->gpuProcesses();
    for (const auto &p : procs) {
      arr.append(QJsonObject::fromVariantMap(p.toMap()));
    }
  }
  root.insert(QStringLiteral("processes"), arr);
  root.insert(QStringLiteral("count"), arr.size());
  return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

QString RoControlDBusAdaptor::GetGpuDevices() {
  QJsonObject root;
  QJsonArray arr;
  if (m_gpu) {
    const auto devs = m_gpu->gpuDevices();
    for (const auto &d : devs) {
      arr.append(QJsonObject::fromVariantMap(d.toMap()));
    }
  }
  root.insert(QStringLiteral("devices"), arr);
  root.insert(QStringLiteral("count"), arr.size());
  root.insert(QStringLiteral("selectedIndex"),
              m_gpu ? m_gpu->selectedGpuIndex() : 0);
  return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

bool RoControlDBusAdaptor::SelectGpu(int index) {
  if (!m_gpu) {
    return false;
  }
  m_gpu->setSelectedGpuIndex(index);
  return true;
}

bool RoControlDBusAdaptor::SetFanMode(const QString &mode) {
  if (!m_fan) {
    return false;
  }
  m_fan->setFanMode(mode);
  return true;
}

QString RoControlDBusAdaptor::CycleFanMode() {
  if (!m_fan) {
    return QStringLiteral("auto");
  }
  return m_fan->cycleFanMode();
}

bool RoControlDBusAdaptor::ApplyFanCurvePreset(const QString &presetName) {
  if (!m_fan) {
    return false;
  }
  return m_fan->applyCurvePreset(presetName);
}

QStringList RoControlDBusAdaptor::GetFanModes() {
  if (!m_fan) {
    return {};
  }
  return m_fan->availableModes();
}

bool RoControlDBusAdaptor::SetFanSpeed(int percent) {
  if (!m_fan || !m_fan->controlSupported()) {
    return false;
  }
  m_fan->setFanMode(QStringLiteral("manual"));
  m_fan->setManualFanSpeedPercent(percent);
  return true;
}

bool RoControlDBusAdaptor::SetPowerLimit(double watts) {
  if (!m_power || !m_power->controlSupported()) {
    return false;
  }
  return m_power->setPowerLimit(watts);
}

bool RoControlDBusAdaptor::SetPersistenceMode(bool enabled) {
  if (!m_power) {
    return false;
  }
  return m_power->setPersistenceMode(enabled);
}

bool RoControlDBusAdaptor::SetClockOffsets(int coreMhz, int memMhz) {
  if (!m_power || !m_power->controlSupported()) {
    return false;
  }
  return m_power->setClockOffsets(coreMhz, memMhz);
}

bool RoControlDBusAdaptor::SetFanSmoothing(bool enabled, int rampUp,
                                           int rampDown, int hysteresis) {
  if (!m_fan) {
    return false;
  }
  m_fan->setSmoothingEnabled(enabled);
  if (rampUp > 0) {
    m_fan->setRampUpRatePercent(rampUp);
  }
  if (rampDown > 0) {
    m_fan->setRampDownRatePercent(rampDown);
  }
  if (hysteresis >= 0) {
    m_fan->setHysteresisTempC(hysteresis);
  }
  return true;
}

RoControlDBusService::RoControlDBusService(QObject *parent, CpuMonitor *cpu,
                                           GpuMonitor *gpu, RamMonitor *ram,
                                           FanController *fan,
                                           PowerController *power,
                                           HealthGuard *guard)
    : QObject(parent) {
  m_adaptor = new RoControlDBusAdaptor(this, cpu, gpu, ram, fan, power, guard);

  QDBusConnection connection = QDBusConnection::sessionBus();
  if (connection.isConnected()) {
    if (connection.registerObject(QString::fromLatin1(kObjectPath), this)) {
      if (connection.registerService(QString::fromLatin1(kServiceName))) {
        m_registered = true;
      }
    }
  }
}

RoControlDBusService::~RoControlDBusService() {
  if (m_registered) {
    QDBusConnection::sessionBus().unregisterService(
        QString::fromLatin1(kServiceName));
    QDBusConnection::sessionBus().unregisterObject(
        QString::fromLatin1(kObjectPath));
  }
}

bool RoControlDBusService::isRegistered() const { return m_registered; }
