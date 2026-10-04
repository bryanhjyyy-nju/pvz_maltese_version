#include "gamecatalog.h"
#include "allheartwhite.h"

AllHeartWhite::AllHeartWhite(MyGameScene *myScene) :WhiteDogs(":/white/Image/allHeartWhite.gif")
{
    setGameSpeed(myScene->gameSpeed());
    const auto& stats = GameCatalog::plants().at(5);
    hp = stats.health;
    heartCost = stats.cost;


    whiteDogTimer = new GameTimer(this,myScene->gameSpeed());
    whiteDogTimer->setObjectName("doubleHeartTimer");
    connect(whiteDogTimer,&QTimer::timeout,this,[this](){
        emit heartGenerated(pos()-QPointF(65,0));
        emit heartGenerated(pos()+QPointF(65,0));
    });

    whiteDogTimer->start(stats.actionIntervalMs);

    connect(this, &AllHeartWhite::heartGenerated, myScene, &MyGameScene::generateWhiteHeart);
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
