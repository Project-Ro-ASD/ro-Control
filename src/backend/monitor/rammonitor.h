#pragma once

#include <QObject>

// Gerçek zamanlı RAM istatistikleri
class RamMonitor : public QObject
{
    Q_OBJECT

public:
    explicit RamMonitor(QObject *parent = nullptr);
};
