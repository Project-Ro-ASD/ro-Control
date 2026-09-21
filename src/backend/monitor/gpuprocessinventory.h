#pragma once

#include <QString>
#include <QVariantList>

class GpuProcessInventory {
public:
  static QVariantList queryGpuProcesses(int selectedGpuIndex = 0);
  static QVariantList queryGpuDevices(int selectedGpuIndex = 0,
                                      const QString &fallbackName = {});
  static bool killProcess(int pid, const QVariantList &knownProcesses = {},
                          QString *errorMessage = nullptr);
};
