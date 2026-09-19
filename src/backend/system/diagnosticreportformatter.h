#pragma once

#include <QString>

// Value object deliberately independent from QObject and platform probes so
// report rendering can be tested without querying the host machine.
struct DiagnosticReportData {
  QString osName;
  QString kernelVersion;
  QString desktopEnvironment;
  QString cpuModel;
  QString motherboardModel;
  QString biosVersion;
  QString graphicsApiSummary;
  QString integratedGpuName;
  QString integratedGpuMemory;
  QString gpuName;
  QString driverVersion;
  QString vram;
  QString ram;
  QString pcie;
  QString secureBoot;
};

class DiagnosticReportFormatter {
public:
  static QString format(const DiagnosticReportData &data,
                        const QString &format);
};
