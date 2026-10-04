#ifndef LINEWHITE_H
#define LINEWHITE_H

#include <QObject>
#include "whitedogs.h"
#include <QPropertyAnimation>
#include "mygamescene.h"
#include "yellowdogs.h"


class LineWhite : public WhiteDogs
{
    Q_OBJECT
public:
    explicit LineWhite(int row, int col, MyGameScene *myScene, QPointF cPos);

    bool checkCollision();

    void gamePaused() override;
    void gameContinued() override;

private:
    MyGameScene *battleScene;
    bool m_isGamePaused = false;
    bool plantingCellVacated = false;
    // QPropertyAnimation *memRunningAnim;
    YellowDogs *targetYellowDog = nullptr;

signals:
    void vacatedPlantingCell();
};

#endif // LINEWHITE_H
