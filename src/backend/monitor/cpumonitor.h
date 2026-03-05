#pragma once

#include <QObject>

// Gerçek zamanlı CPU istatistikleri
class CpuMonitor : public QObject
{
    Q_OBJECT

public:
    explicit CpuMonitor(QObject *parent = nullptr);
};
