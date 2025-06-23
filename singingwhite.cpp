#include "singingwhite.h"
#include "whitedogs.h"
#include <QDebug>

SingingWhite::SingingWhite(int r, int c,MyGameScene *scene):WhiteDogs(":/white/Image/singingWhite.gif")
{
    hp = 300;
    heartCost = 100;
    setItPos(r,c);
    whiteDogTimer = new QTimer(this);
    whiteDogTimer->start(2000);
    connect(this, &SingingWhite::bulletShot,scene, &MyGameScene::generateBullet);
    connect(whiteDogTimer,&QTimer::timeout,this,[=](){
        emit bulletShot(r,c);
    });



}


