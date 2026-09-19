#include "systemplatformprobes.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace {
QString readTextFile(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }
  return QString::fromUtf8(file.readAll()).trimmed();
}
} // namespace

bool SystemPlatformProbes::onBattery(QString *sourceLabel) {
  const QString overrideOnline =
      qEnvironmentVariable("RO_CONTROL_POWER_SUPPLY_ONLINE").trimmed();
  if (!overrideOnline.isEmpty()) {
    const bool onBattery = overrideOnline == QStringLiteral("0");
    if (sourceLabel) {
      *sourceLabel =
          onBattery ? QStringLiteral("Battery") : QStringLiteral("AC Power");
    }
    return onBattery;
  }
#if defined(Q_OS_LINUX)
  QDir powerDir(QStringLiteral("/sys/class/power_supply"));
  bool hasBattery = false;
  bool discharging = false;
  bool acOnline = false;
  for (const QFileInfo &entry :
       powerDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
    const QString type = readTextFile(entry.absoluteFilePath() + "/type");
    if (type.compare(QStringLiteral("Battery"), Qt::CaseInsensitive) == 0) {
      hasBattery = true;
      discharging =
          discharging || readTextFile(entry.absoluteFilePath() + "/status")
                                 .compare(QStringLiteral("Discharging"),
                                          Qt::CaseInsensitive) == 0;
    } else if (type.compare(QStringLiteral("Mains"), Qt::CaseInsensitive) ==
               0) {
      acOnline =
          acOnline || readTextFile(entry.absoluteFilePath() + "/online") == "1";
    }
  }
  const bool onBattery = hasBattery && (discharging || !acOnline);
  if (sourceLabel) {
    *sourceLabel = hasBattery ? (onBattery ? QStringLiteral("Battery")
                                           : QStringLiteral("AC Power"))
                              : QStringLiteral("AC / Desktop");
  }
  return onBattery;
#else
  if (sourceLabel) {
    *sourceLabel = QStringLiteral("AC Power");
  }
  return false;
#endif
}
