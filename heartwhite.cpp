  #include "heartwhite.h"


HeartWhite::HeartWhite(MyGameScene *scene):WhiteDogs(":/white/Image/heartWhite.gif", 1.2)
{
    hp = 300;
    heartCost = 50;

    memHeartProductionTimer = new QTimer(this);
    connect(memHeartProductionTimer,&QTimer::timeout,this,[this](){
        emit heartGenerated(this->pos());
    });

    memHeartProductionTimer->start(24000); //每 20s 产一个阳光

    connect(this, &HeartWhite::heartGenerated, scene, [=](){
        scene->generateWhiteHeart(this->pos());
    });
}
