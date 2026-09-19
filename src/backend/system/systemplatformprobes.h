#pragma once

#include <QString>

class SystemPlatformProbes {
public:
  static bool onBattery(QString *sourceLabel = nullptr);
};
