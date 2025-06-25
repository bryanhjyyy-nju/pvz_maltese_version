  #include "heartwhite.h"


HeartWhite::HeartWhite(MyGameScene *myScene):WhiteDogs(":/white/Image/heartWhite.gif", 1.2)
{
    hp = 500;
    heartCost = 50;

    whiteDogTimer = new QTimer(this);
    connect(whiteDogTimer,&QTimer::timeout,this,[this](){
        emit heartGenerated(this->pos());
    });

    whiteDogTimer->start(12000); //每 4s 产一个阳光

    connect(this, &HeartWhite::heartGenerated, myScene, [=](){
        myScene->generateWhiteHeart(this->pos());
    });
}

void HeartWhite::gamePaused(){
    if(movie){
        movie->stop();
    }
    if(whiteDogTimer){
        if(whiteDogTimer->isActive()){
            whiteDogTimer->stop();
        }
    }
}

void HeartWhite::gameContinued(){
    if(movie){
        movie->start();
    }
    if(whiteDogTimer){
        if(!whiteDogTimer->isActive()){
            whiteDogTimer->start();
        }
    }
}
