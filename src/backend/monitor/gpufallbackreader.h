#pragma once

#include <QString>

class CommandRunner;

class GpuFallbackReader {
public:
  static bool readGenericLinuxGpuMetrics(int *temperatureC,
                                         int *utilizationPercent,
                                         int *memoryUsedMiB,
                                         int *memoryTotalMiB);
  static bool readNvidiaTemperatureFallback(CommandRunner &runner, int *value);
  static bool readNvidiaHotspotAndMemoryTemps(CommandRunner &runner,
                                              int *hotspotC, int *memoryC);
  static bool hasNvidiaPciDevice();
  static bool readTemperatureFromSensorsOutput(const QString &text, int *value);
};
