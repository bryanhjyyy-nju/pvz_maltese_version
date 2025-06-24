#include "yellowdogs.h"
#include <QDebug>

YellowDogs::YellowDogs(int row,MyGameScene *myScene,int typeNum): targetWhiteDog(nullptr),memIsMoving(true){
    itRow = row;
    setZValue(5);
    //初始化血量，攻击力，移动速度，贴图
    initArgues(typeNum);

    movingAnim = new QPropertyAnimation(this, "pos", this);
    movingAnim->setDuration(1000);
    // movingAnim->setLoopCount(-1);
    movingAnim->setEasingCurve(QEasingCurve::Linear);
    qreal distance = speed * 1.0;
    connect(movingAnim, &QPropertyAnimation::finished, this,[=](){
        if(memIsMoving){
            QPointF target = pos() + QPointF(-distance, 0);
            movingAnim->setStartValue(pos());
            movingAnim->setEndValue(target);
            movingAnim->start();
        }
    });
    connect(this, &YellowDogs::isAttacked, this, [this](){
        if(hp <= 0){ removeItself(); }
    });

    connect(myScene->memGameTimer,&QTimer::timeout, this,[=](){
        if(checkCollision()){
            //攻击逻辑
            stopMoving();
            targetWhiteDog->cutHp(atkPower);
            if(targetWhiteDog->getHp() <= 0){
                targetWhiteDog->removeItself();
                targetWhiteDog = nullptr;
                // memIsMoving = true;
                // startMoving(MyDirection::Left);
            }
        }
        else{
            if(!memIsMoving){
                //继续走
                memIsMoving = true;
                startMoving(MyDirection::Left);
            }
        }
        if(x() < 10){ emit arrivedYourHome(); }
        // else if(!checkCollision()){
        //     startMoving(MyDirection::Left);
        // }
    });

}

void YellowDogs::initArgues(int typeNum){
    if(typeNum == 1){
        hp = 800;
        speed = 20;
        atkPower = 12;
        setupGifAnimation(":/yellow/Image/guitarYellow.gif",0.4);
    }
    else {
        hp = 300;
        speed = 30;
        atkPower = 10;
        setupGifAnimation(":/yellow/Image/forkYellow.gif",0.55);
    }
}

bool YellowDogs::checkCollision(){
    // 获取场景中所有与当前 YellowDogs 碰撞的 item
    QList<QGraphicsItem*> colliding_items = collidingItems();

    // 遍历所有碰撞的 item
    for (QGraphicsItem *item : colliding_items) {
        // 尝试将 item 转换为 WhiteDogs 类型
        WhiteDogs *whiteDog = dynamic_cast<WhiteDogs*>(item);

        // 如果转换成功，并且它和 YellowDogs 在同一行，说明发生了有效碰撞
        if (whiteDog && whiteDog->getItRow() == this->getItRow()) {
            // 在这里你可以保存目标，以便后续攻击
            targetWhiteDog = whiteDog;
            return true; // 发现碰撞，返回 true
        }
    }

    return false; // 没有找到碰撞的 WhiteDogs
}

void YellowDogs::stopMoving(){
    if(memIsMoving){
        memIsMoving = false;
        movingAnim->stop();
    }
}

void YellowDogs::getAttacked(int atk){
    cutHp(atk);
    emit isAttacked();
}

// GuitarDog::GuitarDog(int row, MyGameScene *myScene) : YellowDogs(row, myScene){}

// void GuitarDog::initArgues(){
//     hp = 800;
//     speed = 20;
//     setupGifAnimation(":/yellow/Image/guitarYellow.gif",0.6);
// }
