#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

class PowerController : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool supported READ supported NOTIFY supportedChanged)
  Q_PROPERTY(bool controlSupported READ controlSupported NOTIFY
                 controlSupportedChanged)
  Q_PROPERTY(double currentPowerDrawW READ currentPowerDrawW NOTIFY
                 currentPowerDrawWChanged)
  Q_PROPERTY(double powerLimitW READ powerLimitW NOTIFY powerLimitWChanged)
  Q_PROPERTY(double minPowerLimitW READ minPowerLimitW NOTIFY
                 powerLimitConstraintsChanged)
  Q_PROPERTY(double maxPowerLimitW READ maxPowerLimitW NOTIFY
                 powerLimitConstraintsChanged)
  Q_PROPERTY(double defaultPowerLimitW READ defaultPowerLimitW NOTIFY
                 powerLimitConstraintsChanged)
  Q_PROPERTY(bool persistenceModeEnabled READ persistenceModeEnabled NOTIFY
                 persistenceModeChanged)
  Q_PROPERTY(QString powerPreset READ powerPreset WRITE applyPowerPreset NOTIFY
                 powerPresetChanged)
  Q_PROPERTY(QString systemPowerProfile READ systemPowerProfile NOTIFY
                 systemPowerProfileChanged)
  Q_PROPERTY(bool systemPowerProfileSupported READ systemPowerProfileSupported
                 NOTIFY systemPowerProfileSupportedChanged)
  Q_PROPERTY(QString activePreset READ activePreset NOTIFY activePresetChanged)
  Q_PROPERTY(QStringList availablePresets READ availablePresets CONSTANT)
  Q_PROPERTY(
      int coreClockOffsetMHz READ coreClockOffsetMHz NOTIFY clockOffsetsChanged)
  Q_PROPERTY(int memoryClockOffsetMHz READ memoryClockOffsetMHz NOTIFY
                 clockOffsetsChanged)
  Q_PROPERTY(bool clockOffsetSupported READ clockOffsetSupported NOTIFY
                 clockOffsetSupportedChanged)
  Q_PROPERTY(int minCoreOffsetMHz READ minCoreOffsetMHz CONSTANT)
  Q_PROPERTY(int maxCoreOffsetMHz READ maxCoreOffsetMHz CONSTANT)
  Q_PROPERTY(int minMemoryOffsetMHz READ minMemoryOffsetMHz CONSTANT)
  Q_PROPERTY(int maxMemoryOffsetMHz READ maxMemoryOffsetMHz CONSTANT)
  Q_PROPERTY(
      QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
  explicit PowerController(QObject *parent = nullptr);
  ~PowerController() override;

  bool supported() const;
  bool controlSupported() const;
  double currentPowerDrawW() const;
  double powerLimitW() const;
  double minPowerLimitW() const;
  double maxPowerLimitW() const;
  double defaultPowerLimitW() const;
  bool persistenceModeEnabled() const;
  QString powerPreset() const;
  QString systemPowerProfile() const;
  bool systemPowerProfileSupported() const;
  QString activePreset() const;
  QStringList availablePresets() const;
  int coreClockOffsetMHz() const;
  int memoryClockOffsetMHz() const;
  bool clockOffsetSupported() const;
  int minCoreOffsetMHz() const { return -500; }
  int maxCoreOffsetMHz() const { return 500; }
  int minMemoryOffsetMHz() const { return -1000; }
  int maxMemoryOffsetMHz() const { return 2000; }
  QString statusMessage() const;

  Q_INVOKABLE void refresh();
  Q_INVOKABLE bool setPowerLimit(double watts);
  Q_INVOKABLE bool setPersistenceMode(bool enabled);
  Q_INVOKABLE bool applyPowerPreset(const QString &preset);
  Q_INVOKABLE bool resetToDefault();
  Q_INVOKABLE void updatePowerDraw(double watts);
  Q_INVOKABLE bool setClockOffsets(int coreOffsetMHz, int memoryOffsetMHz);
  Q_INVOKABLE bool resetClockOffsets();

signals:
  void supportedChanged();
  void controlSupportedChanged();
  void currentPowerDrawWChanged();
  void powerLimitWChanged();
  void powerLimitConstraintsChanged();
  void persistenceModeChanged();
  void powerPresetChanged();
  void systemPowerProfileChanged();
  void systemPowerProfileSupportedChanged();
  void activePresetChanged();
  void clockOffsetsChanged();
  void clockOffsetSupportedChanged();
  void statusMessageChanged();
  void powerLimitApplied(double targetWatts, bool success);
  void clockOffsetsApplied(int coreOffsetMHz, int memoryOffsetMHz,
                           bool success);

private:
  void detectCapabilities();
  void queryPowerMetrics();
  void querySystemPowerProfile();
  bool applySystemPowerProfile(const QString &preset);
  void loadSettings();
  void saveSettings();
  void setStatusMessage(const QString &msg);

  QTimer m_timer;
  bool m_supported = false;
  bool m_controlSupported = false;
  double m_currentPowerDrawW = 0.0;
  double m_powerLimitW = 0.0;
  double m_minPowerLimitW = 0.0;
  double m_maxPowerLimitW = 0.0;
  double m_defaultPowerLimitW = 0.0;
  bool m_persistenceModeEnabled = false;
  QString m_powerPreset = QStringLiteral("balanced");
  QString m_systemPowerProfile;
  bool m_systemPowerProfileSupported = false;
  QString m_systemPowerBackend;
  int m_coreClockOffsetMHz = 0;
  int m_memoryClockOffsetMHz = 0;
  bool m_clockOffsetSupported = false;
  QString m_statusMessage;
  quint64 m_refreshTickCount = 0;
};
