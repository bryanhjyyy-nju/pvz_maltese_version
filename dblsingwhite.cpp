#include "dblsingwhite.h"

DblSingWhite::DblSingWhite(int r, int c,MyGameScene *myScene) : WhiteDogs(":/white/Image/dblSingWhite.gif",0.35)
{
    hp = 300;
    heartCost = 200;
    isZombieOnYourLawn = false;
    setItPos(r,c);
    whiteDogTimer = new QTimer(this);
    connect(this, &DblSingWhite::bulletShot,myScene, &MyGameScene::generateBullet);
    connect(whiteDogTimer,&QTimer::timeout,this,[=](){
        emit bulletShot(r,c);
    });
    whiteDogTimer->setInterval(1000);
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

bool DblSingWhite::isInFrontOfMe(const QVector<MyItem *> &items){
    for(auto item : items){
        if(item->x() > this->x()){
            return true;
        }
    }
    return false;
}

void DblSingWhite::gamePaused(){
    if(movie){
        movie->stop();
    }
    if(whiteDogTimer){
        if(whiteDogTimer->isActive()){
            whiteDogTimer->stop();
        }
    }
}

void DblSingWhite::gameContinued(){
    if(movie){
        movie->start();
    }
}
