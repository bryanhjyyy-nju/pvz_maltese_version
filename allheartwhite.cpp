#include "gamecatalog.h"
#include "allheartwhite.h"

AllHeartWhite::AllHeartWhite(MyGameScene *myScene) :WhiteDogs(":/white/Image/allHeartWhite.gif")
{
    const auto& stats = GameCatalog::plants().at(5);
    hp = stats.health;
    heartCost = stats.cost;


    whiteDogTimer = new QTimer(this);
    connect(whiteDogTimer,&QTimer::timeout,this,[this](){
        emit heartGenerated(this->pos());
    });

    whiteDogTimer->start(stats.actionIntervalMs);

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
