#include "detector.h"
#include "system/commandrunner.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

NvidiaDetector::NvidiaDetector(QObject *parent) : QObject(parent) {}

NvidiaDetector::GpuInfo NvidiaDetector::detect() const {
  GpuInfo info;

  info.name = detectGpuName();
  info.found = !info.name.isEmpty();
  info.driverVersion = detectDriverVersion();
  info.driverLoaded = isModuleLoaded(QStringLiteral("nvidia"));
  info.nouveauActive = isModuleLoaded(QStringLiteral("nouveau"));
  info.secureBootEnabled = detectSecureBoot();

  return info;
}

bool NvidiaDetector::hasNvidiaGpu() const { return !detectGpuName().isEmpty(); }

bool NvidiaDetector::isDriverInstalled() const {
  return !detectDriverVersion().isEmpty();
}

QString NvidiaDetector::installedDriverVersion() const {
  return detectDriverVersion();
}

QString NvidiaDetector::verificationReport() const {
  const QString gpuText = m_info.found ? m_info.name : QStringLiteral("Yok");
  const QString driverText =
      m_info.driverVersion.isEmpty() ? QStringLiteral("Yok") : m_info.driverVersion;
  const QString secureBootText = m_info.secureBootEnabled ? QStringLiteral("Acik")
                                                           : QStringLiteral("Kapali/Bilinmiyor");

  return QStringLiteral("GPU: %1\nSurucu Versiyonu: %2\nSecure Boot: %3\nNVIDIA Modulu: %4\nNouveau: %5")
      .arg(gpuText, driverText, secureBootText,
           m_info.driverLoaded ? QStringLiteral("Yuklu") : QStringLiteral("Yuklu degil"),
           m_info.nouveauActive ? QStringLiteral("Aktif") : QStringLiteral("Aktif degil"));
}

QString NvidiaDetector::detectGpuName() const {
  CommandRunner runner;

  // lspci ile PCI cihazlarını listele, NVIDIA olanı filtrele
  const auto result =
      runner.run(QStringLiteral("lspci"), {QStringLiteral("-mm")});

  if (!result.success())
    return {};

  // lspci -mm çıktısında NVIDIA GPU satırını ara
  const QStringList lines = result.stdout.split(QLatin1Char('\n'));
  for (const QString &line : lines) {
    if (line.contains(QStringLiteral("NVIDIA"), Qt::CaseInsensitive) &&
        line.contains(QStringLiteral("VGA"), Qt::CaseInsensitive)) {
      // Tırnak işaretleri arasındaki model adını çıkar
      static const QRegularExpression re(QStringLiteral("\"([^\"]+)\""));
      auto it = re.globalMatch(line);
      QStringList parts;
      while (it.hasNext())
        parts << it.next().captured(1);

      if (parts.size() >= 3)
        return parts[2]; // Model adı 3. tırnak grubunda
    }
  }

  return {};
}

QString NvidiaDetector::detectDriverVersion() const {
  CommandRunner runner;

  // nvidia-smi varsa sürücü versiyonunu döner
  const auto result = runner.run(QStringLiteral("nvidia-smi"),
                                 {QStringLiteral("--query-gpu=driver_version"),
                                  QStringLiteral("--format=csv,noheader")});

  if (result.success())
    return result.stdout.trimmed();

  // nvidia-smi yoksa modinfo'dan dene
  const auto modinfo =
      runner.run(QStringLiteral("modinfo"), {QStringLiteral("nvidia")});

  if (modinfo.success()) {
    static const QRegularExpression re(QStringLiteral("^version:\\s+(.+)$"),
                                       QRegularExpression::MultilineOption);
    const auto match = re.match(modinfo.stdout);
    if (match.hasMatch())
      return match.captured(1).trimmed();
  }

  return {};
}

bool NvidiaDetector::isModuleLoaded(const QString &moduleName) const {
  // /proc/modules dosyasını oku — yüklü kernel modüllerini listeler
  QFile modules(QStringLiteral("/proc/modules"));
  if (!modules.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  QTextStream stream(&modules);
  while (!stream.atEnd()) {
    const QString line = stream.readLine();
    if (line.startsWith(moduleName + QLatin1Char(' ')))
      return true;
  }

  return false;
}

bool NvidiaDetector::detectSecureBoot() const {
  CommandRunner runner;
  const auto result =
      runner.run(QStringLiteral("mokutil"), {QStringLiteral("--sb-state")});

  if (result.success() || result.exitCode == 1) {
    return result.stdout.contains(QStringLiteral("enabled"), Qt::CaseInsensitive);
  }

  return false;
}

void NvidiaDetector::refresh() {
  m_info = detect();
  emit infoChanged();
}
