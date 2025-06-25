  #include "heartwhite.h"


HeartWhite::HeartWhite(MyGameScene *myScene):WhiteDogs(":/white/Image/heartWhite.gif", 1.2)
{
    hp = 300;
    heartCost = 50;

    memHeartProductionTimer = new QTimer(this);
    connect(memHeartProductionTimer,&QTimer::timeout,this,[this](){
        emit heartGenerated(this->pos());
    });

    memHeartProductionTimer->start(12000); //每 20s 产一个阳光

    connect(this, &HeartWhite::heartGenerated, myScene, [=](){
        myScene->generateWhiteHeart(this->pos());
    });
}

void HeartWhite::gamePaused(){
    if(movie){
        movie->stop();
    }
    if(memHeartProductionTimer){
        if(memHeartProductionTimer->isActive()){
            memHeartProductionTimer->stop();
        }
    }
}

void HeartWhite::gameContinued(){
    if(movie){
        movie->start();
    }
    if(memHeartProductionTimer){
        if(!memHeartProductionTimer->isActive()){
            memHeartProductionTimer->start();
        }
    }
}
