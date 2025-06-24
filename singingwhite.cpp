#include "singingwhite.h"
#include "whitedogs.h"
#include <QDebug>

SingingWhite::SingingWhite(int r, int c,MyGameScene *scene):WhiteDogs(":/white/Image/singingWhite.gif")
{
    hp = 300;
    heartCost = 100;
    setItPos(r,c);
    whiteDogTimer = new QTimer(this);
    connect(this, &SingingWhite::bulletShot,scene, &MyGameScene::generateBullet);
    connect(whiteDogTimer,&QTimer::timeout,this,[=](){
        emit bulletShot(r,c);
    });
    whiteDogTimer->start(2000);
}


