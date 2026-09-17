#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include "fan/fancontroller.h"
#include "monitor/cpumonitor.h"
#include "monitor/gpumonitor.h"
#include "monitor/rammonitor.h"
#include "power/powercontroller.h"
#include "system/dbusservice.h"
#include "system/healthguard.h"

class TestDBusService : public QObject {
  Q_OBJECT

private slots:
  void testTelemetryGeneration() {
    CpuMonitor cpu;
    GpuMonitor gpu;
    RamMonitor ram;
    FanController fan;
    PowerController power;
    HealthGuard guard;

    RoControlDBusAdaptor adaptor(this, &cpu, &gpu, &ram, &fan, &power, &guard);

    const QString telemetryJson = adaptor.GetTelemetry();
    QVERIFY(!telemetryJson.isEmpty());

    const QJsonDocument doc = QJsonDocument::fromJson(telemetryJson.toUtf8());
    QVERIFY(doc.isObject());
    const QJsonObject obj = doc.object();
    QVERIFY(obj.contains(QStringLiteral("cpu")));
    QVERIFY(obj.contains(QStringLiteral("gpu")));
    QVERIFY(obj.contains(QStringLiteral("ram")));
    QVERIFY(obj.contains(QStringLiteral("fan")));
    QVERIFY(obj.contains(QStringLiteral("power")));
  }

  void testThermalStatus() {
    CpuMonitor cpu;
    GpuMonitor gpu;
    RamMonitor ram;
    FanController fan;
    PowerController power;
    HealthGuard guard;

    RoControlDBusAdaptor adaptor(this, &cpu, &gpu, &ram, &fan, &power, &guard);

    const QString thermalJson = adaptor.GetThermalStatus();
    QVERIFY(!thermalJson.isEmpty());

    const QJsonDocument doc = QJsonDocument::fromJson(thermalJson.toUtf8());
    QVERIFY(doc.isObject());
    const QJsonObject obj = doc.object();
    QVERIFY(obj.contains(QStringLiteral("gpuTemperatureC")));
    QVERIFY(obj.contains(QStringLiteral("cpuTemperatureC")));
    QVERIFY(obj.contains(QStringLiteral("fanRpm")));
    QVERIFY(obj.contains(QStringLiteral("alertActive")));
  }

  void testGpuHealth() {
    CpuMonitor cpu;
    GpuMonitor gpu;
    RamMonitor ram;
    FanController fan;
    PowerController power;
    HealthGuard guard;

    RoControlDBusAdaptor adaptor(this, &cpu, &gpu, &ram, &fan, &power, &guard);

    const QString healthJson = adaptor.GetGpuHealth();
    QVERIFY(!healthJson.isEmpty());

    const QJsonDocument doc = QJsonDocument::fromJson(healthJson.toUtf8());
    QVERIFY(doc.isObject());
  }

  void testGpuProcessesAndDevices() {
    CpuMonitor cpu;
    GpuMonitor gpu;
    RamMonitor ram;
    FanController fan;
    PowerController power;
    HealthGuard guard;

    RoControlDBusAdaptor adaptor(this, &cpu, &gpu, &ram, &fan, &power, &guard);

    const QString procJson = adaptor.GetGpuProcesses();
    QVERIFY(!procJson.isEmpty());
    const QJsonDocument procDoc = QJsonDocument::fromJson(procJson.toUtf8());
    QVERIFY(procDoc.isObject());
    QVERIFY(procDoc.object().contains(QStringLiteral("processes")));

    const QString devJson = adaptor.GetGpuDevices();
    QVERIFY(!devJson.isEmpty());
    const QJsonDocument devDoc = QJsonDocument::fromJson(devJson.toUtf8());
    QVERIFY(devDoc.isObject());
    QVERIFY(devDoc.object().contains(QStringLiteral("devices")));

    QVERIFY(adaptor.SelectGpu(0));
    QVERIFY(adaptor.SetFanSmoothing(true, 25, 10, 3));
    QVERIFY(adaptor.SetFanMode(QStringLiteral("silent")));
    QCOMPARE(adaptor.CycleFanMode(), QStringLiteral("balanced"));
    QVERIFY(adaptor.ApplyFanCurvePreset(QStringLiteral("stealth")));
    QCOMPARE(adaptor.GetFanModes().size(), 6);
  }
};

QTEST_MAIN(TestDBusService)
#include "test_dbus_service.moc"
