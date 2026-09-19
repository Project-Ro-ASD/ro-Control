#include "diagnosticreportformatter.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

namespace {
void insertIfPresent(QJsonObject &object, const QString &key,
                     const QString &value) {
  if (!value.isEmpty()) {
    object.insert(key, value);
  }
}
} // namespace

QString DiagnosticReportFormatter::format(const DiagnosticReportData &data,
                                          const QString &format) {
  const QString outputFormat = format.trimmed().toLower();
  if (outputFormat == QStringLiteral("json")) {
    QJsonObject report;
    report.insert(QStringLiteral("report"),
                  QStringLiteral("ro-Control System Diagnostic Report"));
    insertIfPresent(report, QStringLiteral("operatingSystem"), data.osName);
    insertIfPresent(report, QStringLiteral("linuxKernel"), data.kernelVersion);
    insertIfPresent(report, QStringLiteral("desktopEnvironment"),
                    data.desktopEnvironment);
    insertIfPresent(report, QStringLiteral("processor"), data.cpuModel);
    insertIfPresent(report, QStringLiteral("motherboard"),
                    data.motherboardModel);
    insertIfPresent(report, QStringLiteral("uefiBios"), data.biosVersion);
    insertIfPresent(report, QStringLiteral("graphicsCard"), data.gpuName);
    insertIfPresent(report, QStringLiteral("nvidiaDriver"), data.driverVersion);
    insertIfPresent(report, QStringLiteral("videoMemory"), data.vram);
    if (!data.integratedGpuName.isEmpty() &&
        !data.integratedGpuMemory.isEmpty()) {
      report.insert(QStringLiteral("integratedGraphics"),
                    data.integratedGpuName);
      report.insert(QStringLiteral("integratedGraphicsMemory"),
                    data.integratedGpuMemory);
    }
    insertIfPresent(report, QStringLiteral("systemMemory"), data.ram);
    insertIfPresent(report, QStringLiteral("pcieLink"), data.pcie);
    insertIfPresent(report, QStringLiteral("platformSecurity"),
                    data.secureBoot);
    insertIfPresent(report, QStringLiteral("computeGraphics"),
                    data.graphicsApiSummary);
    return QString::fromUtf8(
        QJsonDocument(report).toJson(QJsonDocument::Indented));
  }

  QString report;
  QTextStream out(&report);
  const bool markdown = outputFormat != QStringLiteral("plain");
  const auto field = [markdown](const QString &label, const QString &value) {
    return markdown ? QStringLiteral("- **%1:** %2\n").arg(label, value)
                    : QStringLiteral("%1: %2\n").arg(label, value);
  };
  out << (markdown ? QStringLiteral("# ro-Control System Diagnostic Report\n\n")
                   : QStringLiteral("ro-Control System Diagnostic Report\n\n"));
  const auto write = [&out, &field](const QString &label,
                                    const QString &value) {
    if (!value.isEmpty()) {
      out << field(label, value);
    }
  };
  write(QStringLiteral("Operating System"), data.osName);
  write(QStringLiteral("Linux Kernel"), data.kernelVersion);
  write(QStringLiteral("Desktop Environment"), data.desktopEnvironment);
  write(QStringLiteral("Processor (CPU)"), data.cpuModel);
  write(QStringLiteral("Motherboard"), data.motherboardModel);
  write(QStringLiteral("UEFI / BIOS"), data.biosVersion);
  write(QStringLiteral("Graphics Card (GPU)"), data.gpuName);
  write(QStringLiteral("NVIDIA Driver"), data.driverVersion);
  write(QStringLiteral("Video Memory (VRAM)"), data.vram);
  if (!data.integratedGpuName.isEmpty() &&
      !data.integratedGpuMemory.isEmpty()) {
    write(QStringLiteral("Integrated Graphics"), data.integratedGpuName);
    write(QStringLiteral("Integrated Graphics Memory"),
          data.integratedGpuMemory);
  }
  write(QStringLiteral("System Memory (RAM)"), data.ram);
  write(QStringLiteral("PCIe Link"), data.pcie);
  write(QStringLiteral("Platform Security"), data.secureBoot);
  write(QStringLiteral("Compute & Graphics"), data.graphicsApiSummary);
  return report;
}
