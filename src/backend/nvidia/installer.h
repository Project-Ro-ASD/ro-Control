#pragma once

#include <QObject>

// DNF ile sürücü kurulum/kaldırma
class NvidiaInstaller : public QObject
{
    Q_OBJECT

public:
    explicit NvidiaInstaller(QObject *parent = nullptr);
};
