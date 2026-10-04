#ifndef DBLSINGWHITE_H
#define DBLSINGWHITE_H

#include <QObject>
#include "whitedogs.h"
#include "mygamescene.h"

class DblSingWhite : public WhiteDogs
{
    Q_OBJECT
public:
    explicit DblSingWhite(int r, int c,MyGameScene *myScene);
    void shootBullet(int r, int c);
    bool isInFrontOfMe(const QVector<MyItem *> &items);

    void gamePaused() override;
    void gameContinued() override;


protected:
    bool isZombieOnYourLawn;
    GameTimer *secondShotTimer=nullptr;

signals:
    void bulletShot(int r,int c);
};

#endif // DBLSINGWHITE_H
