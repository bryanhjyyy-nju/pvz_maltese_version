#include "allheartwhite.h"

AllHeartWhite::AllHeartWhite(MyGameScene *myScene) :WhiteDogs(":/white/Image/allHeartWhite.gif")
{
    hp = 300;
    heartCost = 125;


    whiteDogTimer = new QTimer(this);
    connect(whiteDogTimer,&QTimer::timeout,this,[this](){
        emit heartGenerated(this->pos());
    });

    whiteDogTimer->start(6000); //每 6s 产一个阳光

    connect(this, &AllHeartWhite::heartGenerated, myScene, [=](){
        myScene->generateWhiteHeart(this->pos());
    });
}

void AllHeartWhite::gamePaused(){
    if(movie){
        movie->stop();
    }
    if(whiteDogTimer){
        if(whiteDogTimer->isActive()){
            whiteDogTimer->stop();
        }
    }
}

void AllHeartWhite::gameContinued(){
    if(movie){
        movie->start();
    }
    if(whiteDogTimer){
        if(!whiteDogTimer->isActive()){
            whiteDogTimer->start();
        }
    }
}
