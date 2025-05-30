#ifndef HEARTWHITE_H
#define HEARTWHITE_H

#include <QObject>
#include "whitedogs.h"
#include <QTimer>

class HeartWhite : public WhiteDogs
{
    Q_OBJECT
public:
    explicit HeartWhite();



protected:
    QTimer* memHeartProductionTimer;

signals:
    void heartGenerated(QPointF pos);
};

#endif // HEARTWHITE_H
