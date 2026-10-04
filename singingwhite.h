#ifndef SINGINGWHITE_H
#define SINGINGWHITE_H

#include <QObject>
#include "whitedogs.h"
#include "mygamescene.h"

class SingingWhite : public WhiteDogs
{
    Q_OBJECT
    friend class BattleSnapshot;
public:
    explicit SingingWhite(int r, int c,MyGameScene *myScene);
    void shootBullet(int r, int c);
    bool isInFrontOfMe(const QVector<MyItem *> &items);

    void gamePaused() override;
    void gameContinued() override;


protected:
    bool isZombieOnYourLawn;

signals:
    void bulletShot(int r,int c);
};

#endif // SINGINGWHITE_H
