#pragma once

#include <QObject>

// DNF paket yöneticisi wrapper
class DnfManager : public QObject
{
    Q_OBJECT

public:
    explicit DnfManager(QObject *parent = nullptr);
};
