#include "systeminfoprovider.h"

#include "commandrunner.h"

#include <QFile>
#include <QStringList>
#include <QRegularExpression>
#include <QSysInfo>
#include <QTextStream>

#if defined(Q_OS_UNIX)
#include <sys/utsname.h>
#endif

namespace {

QString simplifiedDesktopName(const QString &desktop) {
  const QString trimmed = desktop.trimmed();
  if (trimmed.compare(QStringLiteral("KDE"), Qt::CaseInsensitive) == 0 ||
      trimmed.compare(QStringLiteral("KDE Plasma"), Qt::CaseInsensitive) == 0 ||
      trimmed.compare(QStringLiteral("Plasma"), Qt::CaseInsensitive) == 0) {
    return QStringLiteral("KDE Plasma");
  }

  if (trimmed.compare(QStringLiteral("GNOME"), Qt::CaseInsensitive) == 0) {
    return QStringLiteral("GNOME");
  }

  return trimmed;
}

[[maybe_unused]] QString virtualizationLabel(const QString &value) {
  const QString lowered = value.trimmed().toLower();
  if (lowered.isEmpty() || lowered == QStringLiteral("none")) {
    return {};
  }
  if (lowered == QStringLiteral("kvm") || lowered == QStringLiteral("qemu")) {
    return QStringLiteral("KVM/QEMU");
  }
  if (lowered == QStringLiteral("vmware")) {
    return QStringLiteral("VMware");
  }
  if (lowered == QStringLiteral("oracle") ||
      lowered == QStringLiteral("virtualbox")) {
    return QStringLiteral("VirtualBox");
  }
  if (lowered == QStringLiteral("microsoft")) {
    return QStringLiteral("Hyper-V");
  }
  if (lowered == QStringLiteral("parallels")) {
    return QStringLiteral("Parallels");
  }
  return value.trimmed();
}

QString valueFromOsRelease(const QString &key) {
  QFile file(QStringLiteral("/etc/os-release"));
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }

  QTextStream stream(&file);
  while (!stream.atEnd()) {
    const QString line = stream.readLine().trimmed();
    if (!line.startsWith(key + QLatin1Char('='))) {
      continue;
    }

    QString value = line.mid(key.size() + 1).trimmed();
    if (value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"')) &&
        value.size() >= 2) {
      value = value.mid(1, value.size() - 2);
    }
    return value;
  }

  return {};
}

}  // namespace

SystemInfoProvider::SystemInfoProvider(QObject *parent) : QObject(parent) {
  refresh();
}

void SystemInfoProvider::refresh() {
  const QString nextOsName = detectOsName();
  const QString nextDesktopEnvironment = detectDesktopEnvironment();
  const QString nextKernelVersion = detectKernelVersion();
  const QString nextCpuModel = detectCpuModel();

  if (m_osName == nextOsName &&
      m_desktopEnvironment == nextDesktopEnvironment &&
      m_kernelVersion == nextKernelVersion && m_cpuModel == nextCpuModel) {
    return;
  }

  m_osName = nextOsName;
  m_desktopEnvironment = nextDesktopEnvironment;
  m_kernelVersion = nextKernelVersion;
  m_cpuModel = nextCpuModel;
  emit infoChanged();
}

QString SystemInfoProvider::detectOsName() const {
  const QString prettyName = valueFromOsRelease(QStringLiteral("PRETTY_NAME"));
  if (!prettyName.isEmpty()) {
    return prettyName;
  }

  const QString productName = QSysInfo::prettyProductName();
  if (!productName.isEmpty()) {
    return productName;
  }

  return QSysInfo::productType();
}

QString SystemInfoProvider::detectKernelVersion() const {
#if defined(Q_OS_UNIX)
  utsname name {};
  if (uname(&name) == 0) {
    return QString::fromLocal8Bit(name.release);
  }
#endif
  return QSysInfo::kernelVersion();
}

QString SystemInfoProvider::detectCpuModel() const {
  QString virtualizationType;
#if defined(Q_OS_LINUX)
  CommandRunner runner;
  const auto lscpuResult = runner.run(QStringLiteral("lscpu"));
  if (lscpuResult.success()) {
    const QStringList lines =
        lscpuResult.stdout.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
      if (line.startsWith(QStringLiteral("Model name:")) ||
          line.startsWith(QStringLiteral("Hardware:")) ||
          line.startsWith(QStringLiteral("Processor:"))) {
        const int separatorIndex = line.indexOf(QLatin1Char(':'));
        if (separatorIndex >= 0) {
          const QString value = line.mid(separatorIndex + 1).trimmed();
          if (!value.isEmpty() &&
              value.compare(QSysInfo::currentCpuArchitecture(),
                            Qt::CaseInsensitive) != 0) {
            return value;
          }
        }
      }
    }
  }

  QFile cpuInfo(QStringLiteral("/proc/cpuinfo"));
  if (cpuInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream stream(&cpuInfo);
    while (!stream.atEnd()) {
      const QString line = stream.readLine();
      if (line.startsWith(QStringLiteral("model name")) ||
          line.startsWith(QStringLiteral("Hardware")) ||
          line.startsWith(QStringLiteral("Processor"))) {
        const int separatorIndex = line.indexOf(QLatin1Char(':'));
        if (separatorIndex >= 0) {
          const QString value = line.mid(separatorIndex + 1).trimmed();
          if (!value.isEmpty() &&
              value.compare(QSysInfo::currentCpuArchitecture(),
                            Qt::CaseInsensitive) != 0) {
            return value;
          }
        }
      }
    }
  }

  const auto virtResult =
      runner.run(QStringLiteral("systemd-detect-virt"), {QStringLiteral("--quiet"), QStringLiteral("--vm")});
  if (virtResult.success()) {
    const auto virtName = runner.run(QStringLiteral("systemd-detect-virt"));
    if (virtName.success()) {
      virtualizationType = virtualizationLabel(virtName.stdout.trimmed());
    }
  }

  if (virtualizationType.isEmpty()) {
    const QString productName =
        valueFromOsRelease(QStringLiteral("VIRTUALIZATION"));
    virtualizationType = virtualizationLabel(productName);
  }
#elif defined(Q_OS_MACOS)
  CommandRunner runner;
  const auto result =
      runner.run(QStringLiteral("sysctl"),
                 {QStringLiteral("-n"), QStringLiteral("machdep.cpu.brand_string")});
  if (result.success()) {
    const QString value = result.stdout.trimmed();
    if (!value.isEmpty()) {
      return value;
    }
  }
#endif

  const QString architecture = QSysInfo::currentCpuArchitecture();
  if (!virtualizationType.isEmpty()) {
    return architecture.isEmpty()
               ? QStringLiteral("%1 Virtual CPU").arg(virtualizationType)
               : QStringLiteral("%1 Virtual CPU (%2)")
                     .arg(virtualizationType, architecture);
  }
  return architecture.isEmpty() ? QStringLiteral("Unknown CPU")
                                : QStringLiteral("CPU (%1)").arg(architecture);
}

QString SystemInfoProvider::detectDesktopEnvironment() const {
  QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP").trimmed();
  if (desktop.isEmpty()) {
    desktop = qEnvironmentVariable("DESKTOP_SESSION").trimmed();
  }

  if (desktop.isEmpty()) {
    return {};
  }

  desktop.replace(QLatin1Char(':'), QLatin1String(" / "));
  const QStringList parts = desktop.split(QLatin1Char('/'), Qt::SkipEmptyParts);
  QStringList normalizedParts;
  for (const QString &part : parts) {
    const QString trimmed = simplifiedDesktopName(part);
    if (trimmed.isEmpty()) {
      continue;
    }

    if (!normalizedParts.contains(trimmed, Qt::CaseInsensitive)) {
      normalizedParts << trimmed;
    }
  }

  return normalizedParts.join(QStringLiteral(" / "));
}
