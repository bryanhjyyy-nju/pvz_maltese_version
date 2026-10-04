#include "gamecatalog.h"
#include "singingwhite.h"
#include "whitedogs.h"
#include <QDebug>

SingingWhite::SingingWhite(int r, int c,MyGameScene *myScene):WhiteDogs(":/white/Image/singingWhite.gif")
{
    setGameSpeed(myScene->gameSpeed());
    const auto& stats = GameCatalog::plants().at(0);
    hp = stats.health;
    heartCost = stats.cost;
    isZombieOnYourLawn = false;
    setItPos(r,c);
    whiteDogTimer = new GameTimer(this,myScene->gameSpeed());
    connect(this, &SingingWhite::bulletShot,myScene, &MyGameScene::generateBullet);
    connect(whiteDogTimer,&QTimer::timeout,this,[=](){
        emit bulletShot(r,c);
    });
    whiteDogTimer->setInterval(stats.actionIntervalMs);
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

void SingingWhite::gamePaused(){
    if(movie){
        movie->stop();
    }
    if(whiteDogTimer){
        if(whiteDogTimer->isActive()){
            whiteDogTimer->stop();
        }
    }
}

void SingingWhite::gameContinued(){
    if(movie){
        movie->start();
    }
}
