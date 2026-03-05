#pragma once

#include <QObject>

// GPU ve sürücü tespiti
class NvidiaDetector : public QObject
{
    Q_OBJECT

public:
    explicit NvidiaDetector(QObject *parent = nullptr);
};
