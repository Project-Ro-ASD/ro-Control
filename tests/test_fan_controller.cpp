#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

#include "fan/fancontroller.h"

class TestFanController : public QObject {
  Q_OBJECT

private slots:
  void initTestCase() {
    QCoreApplication::setOrganizationName("Project-Ro-ASD-Test");
    QCoreApplication::setApplicationName("ro-control-test");
    QSettings settings;
    settings.clear();
  }

  void init() {
    QSettings settings;
    settings.clear();
    qunsetenv("RO_CONTROL_MOCK_FAN_CAPABILITY");
    qunsetenv("RO_CONTROL_FAN_SYSFS_ROOT");
    qunsetenv("RO_CONTROL_COMMAND_NVIDIA_SETTINGS");
  }

  void testConstructionAndDefaults() {
    FanController fan;
    QVERIFY(fan.running());
    QCOMPARE(fan.fanMode(), QStringLiteral("auto"));
    QCOMPARE(fan.modeEnum(), FanController::FanMode::Auto);
    QCOMPARE(fan.availableModes().size(), 6);
    QVERIFY(fan.customCurvePoints().size() >= 4);
    QVERIFY(!fan.safetyOverrideActive());
    QCOMPARE(fan.thermalThresholdC(), 85);
  }

  void testModeTransitions() {
    FanController fan;
    fan.stop();

    fan.setFanMode(QStringLiteral("silent"));
    QCOMPARE(fan.fanMode(), QStringLiteral("silent"));
    QCOMPARE(fan.modeEnum(), FanController::FanMode::Silent);

    fan.setFanMode(QStringLiteral("balanced"));
    QCOMPARE(fan.fanMode(), QStringLiteral("balanced"));
    QCOMPARE(fan.modeEnum(), FanController::FanMode::Balanced);

    fan.setFanMode(QStringLiteral("performance"));
    QCOMPARE(fan.fanMode(), QStringLiteral("performance"));
    QCOMPARE(fan.modeEnum(), FanController::FanMode::Performance);

    fan.setFanMode(QStringLiteral("manual"));
    QCOMPARE(fan.fanMode(), QStringLiteral("manual"));
    QCOMPARE(fan.modeEnum(), FanController::FanMode::Manual);

    fan.setFanMode(QStringLiteral("custom"));
    QCOMPARE(fan.fanMode(), QStringLiteral("custom"));
    QCOMPARE(fan.modeEnum(), FanController::FanMode::Custom);

    fan.resetToAuto();
    QCOMPARE(fan.fanMode(), QStringLiteral("auto"));
    QCOMPARE(fan.modeEnum(), FanController::FanMode::Auto);
  }

  void testManualSpeedClamping() {
    FanController fan;
    fan.stop();

    fan.setManualFanSpeedPercent(65);
    QCOMPARE(fan.manualFanSpeedPercent(), 65);

    fan.setManualFanSpeedPercent(150);
    QCOMPARE(fan.manualFanSpeedPercent(), 100);

    fan.setManualFanSpeedPercent(-20);
    QCOMPARE(fan.manualFanSpeedPercent(), 0);
  }

  void testCurveInterpolationCompleteMath() {
    // 1. Balanced curve standard points
    const auto balanced = FanController::defaultBalancedCurve();
    QVERIFY(!balanced.isEmpty());

    // Below min temperature (40°C -> 30%)
    QCOMPARE(FanController::calculateCurveFanSpeed(balanced, 20), 30);
    QCOMPARE(FanController::calculateCurveFanSpeed(balanced, -10), 30);

    // Exact min point
    QCOMPARE(FanController::calculateCurveFanSpeed(balanced, 40), 30);

    // Midpoint between 40°C (30%) and 55°C (45%)
    // delta temp = 15, delta speed = 15. At 47°C -> 30 + 7 = 37%
    const int mid1 = FanController::calculateCurveFanSpeed(balanced, 47);
    QCOMPARE(mid1, 37);

    // Exact intermediate point (68°C -> 65%)
    QCOMPARE(FanController::calculateCurveFanSpeed(balanced, 68), 65);

    // Exact max point (85°C -> 100%)
    QCOMPARE(FanController::calculateCurveFanSpeed(balanced, 85), 100);

    // Above max point
    QCOMPARE(FanController::calculateCurveFanSpeed(balanced, 95), 100);
    QCOMPARE(FanController::calculateCurveFanSpeed(balanced, 150), 100);

    // 2. Empty curve fallback
    QCOMPARE(FanController::calculateCurveFanSpeed({}, 50), 50);

    // 3. Single point curve
    QCOMPARE(FanController::calculateCurveFanSpeed({{60, 70}}, 40), 70);
    QCOMPARE(FanController::calculateCurveFanSpeed({{60, 70}}, 80), 70);

    // 4. Duplicate temperature points (resolves to max speed)
    const QVector<FanCurvePoint> duplicateCurve = {
        {50, 30}, {50, 60}, {80, 100}};
    QCOMPARE(FanController::calculateCurveFanSpeed(duplicateCurve, 50), 60);

    // 5. Out-of-order curve points (auto-sorted)
    const QVector<FanCurvePoint> disorderedCurve = {
        {80, 100}, {40, 20}, {60, 50}};
    QCOMPARE(FanController::calculateCurveFanSpeed(disorderedCurve, 50), 35);
  }

  void testThermalSafetyDynamicWatchdogAndHysteresisMargin() {
    FanController fan;
    fan.stop();
    fan.setFanMode(QStringLiteral("silent"));

    // Safe normal temp
    fan.updateTemperature(60);
    QVERIFY(!fan.safetyOverrideActive());
    QVERIFY(fan.targetFanSpeedPercent() < 100);

    // Critical temp reached (>= 85°C) -> triggers safety override
    fan.updateTemperature(86);
    QVERIFY(fan.safetyOverrideActive());
    QCOMPARE(fan.targetFanSpeedPercent(), 100);

    // Slightly cooled to 82°C -> still in safety margin (< 85°C but > 80°C)
    fan.updateTemperature(82);
    QVERIFY(fan.safetyOverrideActive());
    QCOMPARE(fan.targetFanSpeedPercent(), 100);

    // Cooled down below recovery margin (85 - 5 = 80°C) -> safety deactivated
    fan.updateTemperature(78);
    QVERIFY(!fan.safetyOverrideActive());
    QVERIFY(fan.targetFanSpeedPercent() < 100);
  }

  void testDirectionalHysteresisAndAntiHunting() {
    FanController fan;
    fan.stop();
    fan.setSmoothingEnabled(false);
    fan.setFanMode(QStringLiteral("balanced"));

    // Set initial temperature at 68°C (speed is 65%)
    fan.updateTemperature(68);
    const int initialSpeed = fan.targetFanSpeedPercent();
    QCOMPARE(initialSpeed, 65);

    // Temperature drops slightly from 68°C to 67°C (delta = 1°C < 2°C
    // hysteresis) Fan speed should NOT hunt down immediately
    fan.updateTemperature(67);
    QCOMPARE(fan.targetFanSpeedPercent(), 65);

    // Temperature drops further to 64°C (delta = 4°C >= 2°C hysteresis)
    // Fan speed steps down smoothly
    fan.updateTemperature(64);
    QVERIFY(fan.targetFanSpeedPercent() < 65);

    // Temperature rises to 70°C -> immediate cooling reaction
    fan.updateTemperature(70);
    QVERIFY(fan.targetFanSpeedPercent() > 65);
  }

  void testCustomCurveEditingAndMonotonicSorting() {
    FanController fan;
    fan.stop();

    fan.resetCustomCurve();
    const auto initialPoints = fan.customCurvePoints();
    QVERIFY(initialPoints.size() >= 4);

    QVERIFY(fan.setCustomCurvePoint(0, 35, 25));
    QVERIFY(!fan.setCustomCurvePoint(-1, 50, 50)); // Out of bounds
    QVERIFY(!fan.setCustomCurvePoint(99, 50, 50)); // Out of bounds

    const auto updated = fan.customCurvePoints();
    QCOMPARE(updated.at(0).temperatureC, 35);
    QCOMPARE(updated.at(0).fanSpeedPercent, 25);

    // Variant representation test
    const auto variantList = fan.customCurvePointsVariant();
    QCOMPARE(variantList.size(), updated.size());
  }

  void testPersistenceAndCorruptedDataRecovery() {
    {
      FanController fan;
      fan.stop();
      fan.setFanMode(QStringLiteral("performance"));
      fan.setManualFanSpeedPercent(85);
      fan.setCustomCurvePoint(0, 30, 20);
    }

    // New instance loads persisted configuration
    {
      FanController fan;
      fan.stop();
      QCOMPARE(fan.fanMode(), QStringLiteral("performance"));
      QCOMPARE(fan.manualFanSpeedPercent(), 85);
      QCOMPARE(fan.customCurvePoints().at(0).temperatureC, 30);
    }

    // Corrupted curveCount in QSettings
    {
      QSettings settings;
      settings.beginGroup(QStringLiteral("FanControl"));
      settings.setValue(QStringLiteral("curveCount"), 999);
      settings.endGroup();
    }

    // Should recover safely to default curve without crashing
    {
      FanController fan;
      fan.stop();
      QVERIFY(fan.customCurvePoints().size() >= 4);
    }
  }

  void testCapabilityDetectionSeparation() {
    // 1. Mock Controllable
    qputenv("RO_CONTROL_MOCK_FAN_CAPABILITY", "controllable");
    {
      FanController fan;
      fan.stop();
      QVERIFY(fan.supported());
      QVERIFY(fan.controlSupported());
      QCOMPARE(fan.capability(),
               FanController::ControlCapability::Controllable);
      QCOMPARE(fan.capabilityString(), QStringLiteral("controllable"));
    }

    // 2. Mock Telemetry Only (Read-Only)
    qputenv("RO_CONTROL_MOCK_FAN_CAPABILITY", "telemetry_only");
    {
      FanController fan;
      fan.stop();
      QVERIFY(fan.supported());
      QVERIFY(!fan.controlSupported());
      QCOMPARE(fan.capability(),
               FanController::ControlCapability::TelemetryOnly);
      QCOMPARE(fan.capabilityString(), QStringLiteral("telemetry_only"));
    }

    // 3. Mock Permission Denied
    qputenv("RO_CONTROL_MOCK_FAN_CAPABILITY", "permission_denied");
    {
      FanController fan;
      fan.stop();
      QVERIFY(fan.supported());
      QVERIFY(!fan.controlSupported());
      QCOMPARE(fan.capability(),
               FanController::ControlCapability::PermissionDenied);
      QCOMPARE(fan.capabilityString(), QStringLiteral("permission_denied"));
    }

    // 4. Mock Unsupported
    qputenv("RO_CONTROL_MOCK_FAN_CAPABILITY", "unsupported");
    {
      FanController fan;
      fan.stop();
      QVERIFY(!fan.supported());
      QVERIFY(!fan.controlSupported());
      QCOMPARE(fan.capability(), FanController::ControlCapability::Unsupported);
      QCOMPARE(fan.capabilityString(), QStringLiteral("unsupported"));
    }
  }

  void testSysfsHwmonCapabilityAndWritableValidation() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hwmonDir = tempDir.filePath(QStringLiteral("hwmon0"));
    QVERIFY(QDir().mkpath(hwmonDir));

    QFile nameFile(hwmonDir + QStringLiteral("/name"));
    QVERIFY(nameFile.open(QIODevice::WriteOnly | QIODevice::Text));
    nameFile.write("nouveau\n");
    nameFile.close();

    QFile fanInput(hwmonDir + QStringLiteral("/fan1_input"));
    QVERIFY(fanInput.open(QIODevice::WriteOnly | QIODevice::Text));
    fanInput.write("1850\n");
    fanInput.close();

    QFile pwmFile(hwmonDir + QStringLiteral("/pwm1"));
    QVERIFY(pwmFile.open(QIODevice::WriteOnly | QIODevice::Text));
    pwmFile.write("128\n");
    pwmFile.close();

    QFile pwmEnable(hwmonDir + QStringLiteral("/pwm1_enable"));
    QVERIFY(pwmEnable.open(QIODevice::WriteOnly | QIODevice::Text));
    pwmEnable.write("2\n");
    pwmEnable.close();

    // Make pwm writable
    QVERIFY(QFile::setPermissions(
        pwmFile.fileName(), QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                QFileDevice::ReadGroup |
                                QFileDevice::WriteGroup));

    qputenv("RO_CONTROL_FAN_SYSFS_ROOT", tempDir.path().toUtf8());

    FanController fan;
    fan.stop();
    fan.refresh();

    QVERIFY(fan.supported());
    QVERIFY(fan.controlSupported());
    QCOMPARE(fan.currentRpm(), 1850);
  }

  void testFullScanKeepsStoppedFanChannelsVisible() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hwmonDir = tempDir.filePath(QStringLiteral("hwmon0"));
    QVERIFY(QDir().mkpath(hwmonDir));

    QFile nameFile(hwmonDir + QStringLiteral("/name"));
    QVERIFY(nameFile.open(QIODevice::WriteOnly | QIODevice::Text));
    nameFile.write("nct6798\n");
    nameFile.close();

    QFile labelFile(hwmonDir + QStringLiteral("/fan1_label"));
    QVERIFY(labelFile.open(QIODevice::WriteOnly | QIODevice::Text));
    labelFile.write("CPU_FAN\n");
    labelFile.close();

    QFile fanInput(hwmonDir + QStringLiteral("/fan1_input"));
    QVERIFY(fanInput.open(QIODevice::WriteOnly | QIODevice::Text));
    fanInput.write("0\n");
    fanInput.close();

    qputenv("RO_CONTROL_FAN_SYSFS_ROOT", tempDir.path().toUtf8());
    FanController fan;
    fan.stop();
    const QVariantMap scanResult = fan.runHardwareSetup();

    QCOMPARE(fan.systemFanCount(), 1);
    QVERIFY(scanResult.value(QStringLiteral("completed")).toBool());
    QCOMPARE(scanResult.value(QStringLiteral("channelCount")).toInt(), 1);
    QVERIFY(!scanResult.value(QStringLiteral("controlSupported")).toBool());
    const QVariantMap detected = fan.systemFans().first().toMap();
    QCOMPARE(detected.value(QStringLiteral("type")).toString(),
             QStringLiteral("CPU"));
    QCOMPARE(detected.value(QStringLiteral("rpm")).toInt(), 0);
    QVERIFY(detected.value(QStringLiteral("telemetryAvailable")).toBool());

    qunsetenv("RO_CONTROL_FAN_SYSFS_ROOT");
  }

  void testMockNvidiaSettingsCommand() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString scriptPath =
        tempDir.filePath(QStringLiteral("fake-nvidia-settings.sh"));
    QFile script(scriptPath);
    QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
    script.write("#!/bin/sh\nexit 0\n");
    script.close();
    QVERIFY(QFile::setPermissions(scriptPath, QFileDevice::ReadOwner |
                                                  QFileDevice::WriteOwner |
                                                  QFileDevice::ExeOwner));

    qputenv("RO_CONTROL_COMMAND_NVIDIA_SETTINGS", scriptPath.toUtf8());

    FanController fan;
    fan.stop();
    fan.setFanMode(QStringLiteral("manual"));
    fan.setManualFanSpeedPercent(70);

    QVERIFY(fan.supported());
    QVERIFY(fan.controlSupported());

    qunsetenv("RO_CONTROL_COMMAND_NVIDIA_SETTINGS");
  }

  void testHardwareScanDoesNotWriteNvidiaFanControlState() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString scriptPath =
        tempDir.filePath(QStringLiteral("fake-nvidia-settings.sh"));
    const QString argsLogPath = tempDir.filePath(QStringLiteral("args.log"));
    QFile script(scriptPath);
    QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
    script.write("#!/bin/sh\nprintf '%s\\n' \"$@\" > \"$RO_CONTROL_NVIDIA_SETTINGS_ARGS_LOG\"\necho 0\nexit 0\n");
    script.close();
    QVERIFY(QFile::setPermissions(scriptPath, QFileDevice::ReadOwner |
                                                  QFileDevice::WriteOwner |
                                                  QFileDevice::ExeOwner));

    qputenv("RO_CONTROL_COMMAND_NVIDIA_SETTINGS", scriptPath.toUtf8());
    qputenv("RO_CONTROL_NVIDIA_SETTINGS_ARGS_LOG", argsLogPath.toUtf8());
    FanController fan;
    fan.stop();
    const QVariantMap result = fan.runHardwareSetup();

    QVERIFY(result.value(QStringLiteral("completed")).toBool());
    QFile argsLog(argsLogPath);
    QVERIFY(argsLog.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString arguments = QString::fromUtf8(argsLog.readAll());
    QVERIFY(arguments.contains(QStringLiteral("-q")));
    QVERIFY(arguments.contains(QStringLiteral("[gpu:0]/GPUFanControlState")));
    QVERIFY(!arguments.contains(QStringLiteral("-a")));
    QVERIFY(!arguments.contains(QStringLiteral("GPUFanControlState=0")));

    qunsetenv("RO_CONTROL_COMMAND_NVIDIA_SETTINGS");
    qunsetenv("RO_CONTROL_NVIDIA_SETTINGS_ARGS_LOG");
  }

  void testPerFanConfigurationAndCustomization() {
    FanController fan;
    fan.stop();

    // The UI topology reflects only live, detected channels; profiles remain
    // configurable even when this test machine exposes no physical fan RPM.
    QCOMPARE(fan.systemFanCount(), fan.systemFans().size());

    // 1. GPU Fan adjustments
    QVERIFY(fan.setFanModeForFan(QStringLiteral("gpu_0"),
                                 QStringLiteral("silent")));
    QCOMPARE(fan.fanMode(), QStringLiteral("silent"));
    QVERIFY(fan.setManualSpeedForFan(QStringLiteral("gpu_0"), 65));
    QCOMPARE(fan.manualFanSpeedPercent(), 65);
    QVERIFY(fan.setThermalThresholdForFan(QStringLiteral("gpu_0"), 80));
    QCOMPARE(fan.thermalThresholdC(), 80);

    // 2. CPU Fan adjustments
    QVERIFY(fan.setFanModeForFan(QStringLiteral("cpu_fan_0"),
                                 QStringLiteral("performance")));
    QVERIFY(fan.setManualSpeedForFan(QStringLiteral("cpu_fan_0"), 75));
    QVERIFY(fan.setThermalThresholdForFan(QStringLiteral("cpu_fan_0"), 92));
    QVERIFY(
        fan.setCustomCurvePointForFan(QStringLiteral("cpu_fan_0"), 0, 30, 25));

    QVariantMap cpuCfg = fan.getFanConfig(QStringLiteral("cpu_fan_0"));
    if (!cpuCfg.isEmpty()) {
      QCOMPARE(cpuCfg.value(QStringLiteral("mode")).toString(),
               QStringLiteral("performance"));
      QCOMPARE(cpuCfg.value(QStringLiteral("manualSpeedPercent")).toInt(), 75);
    }
    if (!cpuCfg.isEmpty())
      QCOMPARE(cpuCfg.value(QStringLiteral("thermalThresholdC")).toInt(), 92);

    // 3. Chassis Fan adjustments
    QVERIFY(fan.setFanModeForFan(QStringLiteral("sys_fan_0"),
                                 QStringLiteral("manual")));
    QVERIFY(fan.setManualSpeedForFan(QStringLiteral("sys_fan_0"), 40));
    QVariantMap sysCfg = fan.getFanConfig(QStringLiteral("sys_fan_0"));
    if (!sysCfg.isEmpty()) {
      QCOMPARE(sysCfg.value(QStringLiteral("mode")).toString(),
               QStringLiteral("manual"));
      QCOMPARE(sysCfg.value(QStringLiteral("manualSpeedPercent")).toInt(), 40);
    }

    // 4. Reset fan to auto
    QVERIFY(fan.resetFanToAuto(QStringLiteral("cpu_fan_0")));
    cpuCfg = fan.getFanConfig(QStringLiteral("cpu_fan_0"));
    if (!cpuCfg.isEmpty())
      QCOMPARE(cpuCfg.value(QStringLiteral("mode")).toString(),
               QStringLiteral("auto"));
  }

  void testFanDisplayNamesPersistAndGpuTestDoesNotChangeProfile() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString scriptPath =
        tempDir.filePath(QStringLiteral("fake-nvidia-settings.sh"));
    QFile script(scriptPath);
    QVERIFY(script.open(QIODevice::WriteOnly | QIODevice::Text));
    script.write("#!/bin/sh\nexit 0\n");
    script.close();
    QVERIFY(QFile::setPermissions(scriptPath, QFileDevice::ReadOwner |
                                                  QFileDevice::WriteOwner |
                                                  QFileDevice::ExeOwner));

    qputenv("RO_CONTROL_MOCK_FAN_CAPABILITY", "controllable");
    qputenv("RO_CONTROL_COMMAND_NVIDIA_SETTINGS", scriptPath.toUtf8());
    FanController fan;
    fan.stop();
    fan.setFanMode(QStringLiteral("balanced"));

    QVERIFY(fan.setFanDisplayName(QStringLiteral("gpu_0"),
                                  QStringLiteral("Front GPU Fan")));
    QCOMPARE(fan.getFanConfig(QStringLiteral("gpu_0"))
                 .value(QStringLiteral("name"))
                 .toString(),
             QStringLiteral("Front GPU Fan"));
    QVERIFY(!fan.setFanDisplayName(QStringLiteral("gpu_0"), QString()));
    QVERIFY(!fan.setFanDisplayName(QStringLiteral("unknown"),
                                   QStringLiteral("Invalid")));

    QVERIFY(fan.testFanSpeedForFan(QStringLiteral("gpu_0"), 100));
    QCOMPARE(fan.fanMode(), QStringLiteral("balanced"));
    QVERIFY(fan.restoreFanControlForFan(QStringLiteral("gpu_0")));
    QCOMPARE(fan.fanMode(), QStringLiteral("balanced"));
    qunsetenv("RO_CONTROL_MOCK_FAN_CAPABILITY");
    qunsetenv("RO_CONTROL_COMMAND_NVIDIA_SETTINGS");
  }

  void testGpuLiveRpmAndAutoIdleTelemetry() {
    // Test 0 RPM Auto Idle state
    qputenv("RO_CONTROL_MOCK_FAN_RPM", "0");
    {
      FanController fan;
      fan.stop();
      fan.refresh();
      QCOMPARE(fan.currentRpm(), 0);
      QVariantMap gpuCfg = fan.getFanConfig(QStringLiteral("gpu_0"));
      QCOMPARE(gpuCfg.value(QStringLiteral("rpm")).toInt(), 0);
      QCOMPARE(gpuCfg.value(QStringLiteral("speedPercent")).toInt(), 0);
      QVERIFY(gpuCfg.value(QStringLiteral("isZeroRpm")).toBool());
    }

    // Test live spinning RPM state
    qputenv("RO_CONTROL_MOCK_FAN_RPM", "1450");
    {
      FanController fan;
      fan.stop();
      fan.refresh();
      QCOMPARE(fan.currentRpm(), 1450);
      QVariantMap gpuCfg = fan.getFanConfig(QStringLiteral("gpu_0"));
      QCOMPARE(gpuCfg.value(QStringLiteral("rpm")).toInt(), 1450);
      QVERIFY(!gpuCfg.value(QStringLiteral("isZeroRpm")).toBool());
    }
    qunsetenv("RO_CONTROL_MOCK_FAN_RPM");
  }

  void testUnavailableAuxiliaryFansDoNotFabricateTelemetry() {
    FanController fan;
    fan.stop();
    const QVariantMap cpu = fan.getFanConfig(QStringLiteral("cpu_fan_0"));
    const QVariantMap sys = fan.getFanConfig(QStringLiteral("sys_fan_0"));

    QVERIFY(!cpu.value(QStringLiteral("speedAvailable")).toBool());
    QVERIFY(!sys.value(QStringLiteral("speedAvailable")).toBool());
    QCOMPARE(sys.value(QStringLiteral("rpm")).toInt(), 0);
  }

  void testProfileExportAndImport() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString filePath =
        tempDir.filePath(QStringLiteral("my_gaming_profile.json"));

    FanController fan;
    fan.stop();
    fan.setFanMode(QStringLiteral("performance"));
    fan.setManualFanSpeedPercent(82);
    fan.setCustomCurvePoint(0, 35, 25);

    QVERIFY(fan.exportProfile(QStringLiteral("GamingProfile"), filePath));
    QVERIFY(QFile::exists(filePath));

    // Change fan settings
    fan.setFanMode(QStringLiteral("silent"));
    fan.setManualFanSpeedPercent(40);
    fan.setCustomCurvePoint(0, 50, 45);

    // Import saved profile
    QVERIFY(fan.importProfile(filePath));
    QCOMPARE(fan.fanMode(), QStringLiteral("performance"));
    QCOMPARE(fan.manualFanSpeedPercent(), 82);
    QCOMPARE(fan.customCurvePoints().at(0).temperatureC, 35);
    QCOMPARE(fan.customCurvePoints().at(0).fanSpeedPercent, 25);
  }

  void testBatteryProfileSync() {
    FanController fan;
    fan.stop();
    fan.setBatteryProfileSyncEnabled(true);
    QVERIFY(fan.batteryProfileSyncEnabled());

    fan.setFanMode(QStringLiteral("performance"));
    QCOMPARE(fan.fanMode(), QStringLiteral("performance"));

    // On Battery -> should switch to silent
    fan.syncPowerSource(true);
    QCOMPARE(fan.fanMode(), QStringLiteral("silent"));

    // Back to AC Power -> should restore performance
    fan.syncPowerSource(false);
    QCOMPARE(fan.fanMode(), QStringLiteral("performance"));
  }

  void testSmoothingAndRampRates() {
    FanController fan;
    fan.stop();

    // Smoothing is opt-in so a first install never silently changes cooling.
    QVERIFY(!fan.smoothingEnabled());
    QCOMPARE(fan.hysteresisTempC(), 2);
    QCOMPARE(fan.rampUpRatePercent(), 20);
    QCOMPARE(fan.rampDownRatePercent(), 5);

    fan.setSmoothingEnabled(false);
    QVERIFY(!fan.smoothingEnabled());

    fan.setHysteresisTempC(4);
    QCOMPARE(fan.hysteresisTempC(), 4);

    fan.setRampUpRatePercent(30);
    QCOMPARE(fan.rampUpRatePercent(), 30);

    fan.setRampDownRatePercent(10);
    QCOMPARE(fan.rampDownRatePercent(), 10);
  }

  void testCurvePresetsAndCycleMode() {
    FanController fan;
    fan.stop();

    // Test Cycle Fan Mode
    fan.setFanMode(QStringLiteral("auto"));
    QCOMPARE(fan.cycleFanMode(), QStringLiteral("silent"));
    QCOMPARE(fan.cycleFanMode(), QStringLiteral("balanced"));
    QCOMPARE(fan.cycleFanMode(), QStringLiteral("performance"));
    QCOMPARE(fan.cycleFanMode(), QStringLiteral("manual"));
    QCOMPARE(fan.cycleFanMode(), QStringLiteral("custom"));
    QCOMPARE(fan.cycleFanMode(), QStringLiteral("auto"));

    // Test Curve Presets
    QVERIFY(fan.applyCurvePreset(QStringLiteral("stealth")));
    QVERIFY(fan.customCurvePoints().size() >= 4);
    QCOMPARE(fan.customCurvePoints().at(0).fanSpeedPercent,
             0); // zero-rpm under 45°C

    QVERIFY(fan.applyCurvePreset(QStringLiteral("aggressive")));
    QVERIFY(fan.customCurvePoints().size() >= 4);
    QCOMPARE(fan.customCurvePoints().at(0).fanSpeedPercent, 50);

    QVERIFY(fan.applyCurvePreset(QStringLiteral("stepped")));
    QVERIFY(fan.customCurvePoints().size() >= 4);

    // Per fan preset
    QVERIFY(fan.applyCurvePresetForFan(QStringLiteral("cpu_fan_0"),
                                       QStringLiteral("aggressive")));
    QVERIFY(fan.applyCurvePresetForFan(QStringLiteral("sys_fan_0"),
                                       QStringLiteral("stealth")));
  }
};

QTEST_MAIN(TestFanController)
#include "test_fan_controller.moc"
