#include "singingwhite.h"
#include "whitedogs.h"
#include <QDebug>

SingingWhite::SingingWhite(int r, int c,MyGameScene *myScene):WhiteDogs(":/white/Image/singingWhite.gif")
{
    hp = 300;
    heartCost = 100;
    isZombieOnYourLawn = false;
    setItPos(r,c);
    whiteDogTimer = new QTimer(this);
    connect(this, &SingingWhite::bulletShot,myScene, &MyGameScene::generateBullet);
    connect(whiteDogTimer,&QTimer::timeout,this,[=](){
        emit bulletShot(r,c);
    });
    whiteDogTimer->setInterval(2000);
    connect(myScene->getGameTimer(), &QTimer::timeout, this,[=](){
        if(!isZombieOnYourLawn){
            if(isInFrontOfMe(myScene->getZombieMap(r))){
                // qDebug() << "1";
                isZombieOnYourLawn = true;
                whiteDogTimer->start();
            }
        }
        else{
            if(!isInFrontOfMe(myScene->getZombieMap(r))){
                // qDebug() << "2";
                isZombieOnYourLawn = false;
                whiteDogTimer->stop();
            }
        }
    });
}

bool SingingWhite::isInFrontOfMe(const QVector<MyItem *> &items){
    for(auto item : items){
        if(item->x() > this->x()){
            return true;
        }
    }
    return false;
}

