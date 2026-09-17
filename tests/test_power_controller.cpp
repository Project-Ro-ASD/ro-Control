#include <QCoreApplication>
#include <QSettings>
#include <QSignalSpy>
#include <QTest>

#include "power/powercontroller.h"

class TestPowerController : public QObject {
  Q_OBJECT

private slots:
  void init() {
    QCoreApplication::setOrganizationName(
        QStringLiteral("Project-Ro-ASD-TestSuite"));
    QCoreApplication::setApplicationName(
        QStringLiteral("ro-control-power-test"));

    QSettings settings;
    settings.clear();
    settings.sync();
  }

  void testInitialProperties() {
    PowerController controller;
    QVERIFY(controller.availablePresets().contains(QStringLiteral("eco")));
    QVERIFY(controller.availablePresets().contains(QStringLiteral("balanced")));
    QVERIFY(
        controller.availablePresets().contains(QStringLiteral("performance")));
    QVERIFY(controller.availablePresets().contains(QStringLiteral("custom")));
  }

  void testUpdatePowerDraw() {
    PowerController controller;
    QSignalSpy drawSpy(&controller, &PowerController::currentPowerDrawWChanged);

    controller.updatePowerDraw(125.5);
    QCOMPARE(controller.currentPowerDrawW(), 125.5);
    QCOMPARE(drawSpy.count(), 1);

    // Same value should not trigger another signal
    controller.updatePowerDraw(125.5);
    QCOMPARE(drawSpy.count(), 1);
  }

  void testPresetSelection() {
    PowerController controller;
    QSignalSpy presetSpy(&controller, &PowerController::powerPresetChanged);

    QVERIFY(controller.applyPowerPreset(QStringLiteral("custom")));
    QCOMPARE(controller.powerPreset(), QStringLiteral("custom"));
    QCOMPARE(presetSpy.count(), 1);
  }

  void testClockOffsets() {
    qputenv("RO_CONTROL_MOCK_POWER_CONTROL", "1");
    PowerController controller;
    QSignalSpy offsetSpy(&controller, &PowerController::clockOffsetsChanged);

    QCOMPARE(controller.minCoreOffsetMHz(), -500);
    QCOMPARE(controller.maxCoreOffsetMHz(), 500);
    QCOMPARE(controller.minMemoryOffsetMHz(), -1000);
    QCOMPARE(controller.maxMemoryOffsetMHz(), 2000);

    QVERIFY(controller.setClockOffsets(100, 400));
    QCOMPARE(controller.coreClockOffsetMHz(), 100);
    QCOMPARE(controller.memoryClockOffsetMHz(), 400);
    QCOMPARE(offsetSpy.count(), 1);

    // Test reset
    QVERIFY(controller.resetClockOffsets());
    QCOMPARE(controller.coreClockOffsetMHz(), 0);
    QCOMPARE(controller.memoryClockOffsetMHz(), 0);

    qunsetenv("RO_CONTROL_MOCK_POWER_CONTROL");
  }

  void testPersistenceModeToggle() {
    qputenv("RO_CONTROL_MOCK_POWER_CONTROL", "1");
    PowerController controller;
    QSignalSpy persistSpy(&controller,
                          &PowerController::persistenceModeChanged);

    QVERIFY(controller.setPersistenceMode(true));
    QCOMPARE(controller.persistenceModeEnabled(), true);
    QCOMPARE(persistSpy.count(), 1);

    QVERIFY(controller.setPersistenceMode(false));
    QCOMPARE(controller.persistenceModeEnabled(), false);
    QCOMPARE(persistSpy.count(), 2);

    qunsetenv("RO_CONTROL_MOCK_POWER_CONTROL");
  }

  void testPowerLimitSetting() {
    qputenv("RO_CONTROL_MOCK_POWER_CONTROL", "1");
    PowerController controller;
    QSignalSpy limitSpy(&controller, &PowerController::powerLimitApplied);

    QVERIFY(controller.setPowerLimit(150.0));
    QCOMPARE(limitSpy.count(), 1);
    QCOMPARE(controller.powerLimitW(), 150.0);

    QVERIFY(controller.resetToDefault());

    qunsetenv("RO_CONTROL_MOCK_POWER_CONTROL");
  }
};

QTEST_MAIN(TestPowerController)
#include "test_power_controller.moc"
