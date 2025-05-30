#include "heartwhite.h"


HeartWhite::HeartWhite():WhiteDogs(":/white/Image/heartWhite.gif", 1.2)
{
    hp = 300;
    heartCost = 50;

    memHeartProductionTimer = new QTimer(this);
    connect(memHeartProductionTimer,&QTimer::timeout,this,[this](){
        emit heartGenerated(this->pos());
    });

    memHeartProductionTimer->start(12000); //每12s 产一个阳光
}
