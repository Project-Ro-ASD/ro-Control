#pragma once

#include <QObject>

// Gerçek zamanlı GPU istatistikleri
class GpuMonitor : public QObject
{
    Q_OBJECT

public:
    explicit GpuMonitor(QObject *parent = nullptr);
};
