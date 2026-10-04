#include "gamecatalog.h"
#include "dblsingwhite.h"

DblSingWhite::DblSingWhite(int r, int c,MyGameScene *myScene) : WhiteDogs(":/white/Image/dblSingWhite.gif",0.35)
{
    const auto& stats = GameCatalog::plants().at(6);
    hp = stats.health;
    heartCost = stats.cost;
    isZombieOnYourLawn = false;
    setItPos(r,c);
    whiteDogTimer = new QTimer(this);
    whiteDogTimer->setObjectName("doubleBurstTimer");
    secondShotTimer = new QTimer(this);
    secondShotTimer->setObjectName("doubleSecondShotTimer");
    secondShotTimer->setSingleShot(true);
    secondShotTimer->setInterval(GameCatalog::DoubleShotGapMs);
    connect(this, &DblSingWhite::bulletShot,myScene, &MyGameScene::generateBullet);
    connect(whiteDogTimer,&QTimer::timeout,this,[=](){
        if(hp<=0 || !isInFrontOfMe(myScene->getZombieMap(r))) return;
        emit bulletShot(r,c);
        secondShotTimer->start();
    });
    connect(secondShotTimer,&QTimer::timeout,this,[=] {
        if(hp>0 && isInFrontOfMe(myScene->getZombieMap(r))) emit bulletShot(r,c);
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
                secondShotTimer->stop();
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
