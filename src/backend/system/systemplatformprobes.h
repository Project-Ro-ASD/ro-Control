#pragma once

#include <QString>

struct PlatformStaticInfo {
  QString osName;
  QString desktopEnvironment;
  QString kernelVersion;
  QString cpuModel;
  QString motherboardModel;
  QString biosVersion;
  QString cudaVersion;
  QString graphicsApiSummary;
  QString deviceType;
  QString virtualizationType;
  QString integratedGpuName;
  QString integratedGpuMemory;
};

class SystemPlatformProbes {
public:
  static PlatformStaticInfo probeStaticInfo();
  static bool onBattery(QString *sourceLabel = nullptr);

  static QString detectOsName();
  static QString detectDesktopEnvironment();
  static QString detectKernelVersion();
  static QString detectCpuModel();
  static QString detectMotherboardModel(const QString &virtualizationType = {});
  static QString detectBiosVersion();
  static QString detectCudaVersion();
  static QString detectGraphicsApiSummary();
  static QString detectVirtualizationType();
  static QString detectDeviceType(const QString &virtualizationType = {});
  static QString detectIntegratedGpuName();
  static QString
  detectIntegratedGpuMemory(const QString &integratedGpuName = {});
};
