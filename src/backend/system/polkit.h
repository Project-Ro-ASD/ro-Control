#pragma once

#include <QObject>

// PolicyKit yetki yükseltme
class PolkitHelper : public QObject
{
    Q_OBJECT

public:
    explicit PolkitHelper(QObject *parent = nullptr);
};
