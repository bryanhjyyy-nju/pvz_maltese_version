#ifndef ALLHEARTWHITE_H
#define ALLHEARTWHITE_H

#include <QObject>
#include "whitedogs.h"
#include <QTimer>
#include "mygamescene.h"

class AllHeartWhite : public WhiteDogs
{
    Q_OBJECT
public:
    explicit AllHeartWhite(MyGameScene *myScene);

    void gamePaused() override;
    void gameContinued() override;

protected:
    // QTimer* memHeartProductionTimer;

signals:
    void heartGenerated(QPointF pos);
};

#endif // ALLHEARTWHITE_H
