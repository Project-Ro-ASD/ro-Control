#pragma once

#include <QDBusAbstractAdaptor>
#include <QObject>
#include <QString>

class CpuMonitor;
class GpuMonitor;
class RamMonitor;
class FanController;
class PowerController;
class HealthGuard;

class RoControlDBusAdaptor : public QDBusAbstractAdaptor {
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "io.github.ProjectRoASD.rocontrol")

public:
  explicit RoControlDBusAdaptor(QObject *parent, CpuMonitor *cpu,
                                GpuMonitor *gpu, RamMonitor *ram,
                                FanController *fan, PowerController *power,
                                HealthGuard *guard);

public slots:
  QString GetTelemetry();
  QString GetThermalStatus();
  QString GetGpuHealth();
  QString GetGpuProcesses();
  QString GetGpuDevices();
  bool SelectGpu(int index);
  bool SetFanMode(const QString &mode);
  QString CycleFanMode();
  bool ApplyFanCurvePreset(const QString &presetName);
  QStringList GetFanModes();
  bool SetFanSpeed(int percent);
  bool SetPowerLimit(double watts);
  bool SetPersistenceMode(bool enabled);
  bool SetClockOffsets(int coreMhz, int memMhz);
  bool SetFanSmoothing(bool enabled, int rampUp, int rampDown, int hysteresis);

signals:
  void ThermalAlert(const QString &source, int temperatureC, int thresholdC);
  void TelemetryUpdated();

private:
  CpuMonitor *m_cpu;
  GpuMonitor *m_gpu;
  RamMonitor *m_ram;
  FanController *m_fan;
  PowerController *m_power;
  HealthGuard *m_guard;
};

class RoControlDBusService : public QObject {
  Q_OBJECT

public:
  explicit RoControlDBusService(QObject *parent, CpuMonitor *cpu,
                                GpuMonitor *gpu, RamMonitor *ram,
                                FanController *fan, PowerController *power,
                                HealthGuard *guard);
  ~RoControlDBusService() override;

  bool isRegistered() const;

private:
  bool m_registered = false;
  RoControlDBusAdaptor *m_adaptor = nullptr;
};
