#pragma once

#include <QObject>

// Sürücü güncelleme kontrolü
class NvidiaUpdater : public QObject
{
    Q_OBJECT

public:
    explicit NvidiaUpdater(QObject *parent = nullptr);
};
