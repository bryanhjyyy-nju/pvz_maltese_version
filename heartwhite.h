#ifndef HEARTWHITE_H
#define HEARTWHITE_H

#include <QObject>
#include "whitedogs.h"
#include <QTimer>
#include "mygamescene.h"

class HeartWhite : public WhiteDogs
{
    Q_OBJECT
public:
    explicit HeartWhite(MyGameScene *myScene);

    void gamePaused() override;
    void gameContinued() override;



protected:
    QTimer* memHeartProductionTimer;

signals:
    void heartGenerated(QPointF pos);
};

#endif // HEARTWHITE_H
