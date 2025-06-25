#include "yellowdogs.h"
#include <QDebug>

YellowDogs::YellowDogs(int row,MyGameScene *myScene,int typeNum): targetWhiteDog(nullptr){
    itRow = row;
    setZValue(5);
    //初始化血量，攻击力，移动速度，贴图
    initArgues(typeNum);

    //设置位置
    setPos(QPointF(480 + 9 * 121 - pixmap().width() / 2, 130 + 145 * (row + 0.5) - pixmap().height() / 2));

    //先关联被打和移除自己的信号和槽
    connect(this, &YellowDogs::isAttacked, this, [this](){
        if(hp <= 0){ removeItself(); }
    });

    //设置运动动画
    movingAnim = new QPropertyAnimation(this, "pos", this);
    movingAnim->setDuration(1000);
    // movingAnim->setLoopCount(-1);
    qreal distance = speed * 1.0;
    movingAnim->setEasingCurve(QEasingCurve::Linear);
    movingAnim->setEndValue(pos() + QPointF(-distance, 0));

    connect(movingAnim, &QPropertyAnimation::finished, this,[=](){
        if(memIsMoving){
            movingAnim->setStartValue(pos());
            movingAnim->setEndValue(pos() + QPointF(-distance, 0));
            movingAnim->start();
        }
    });

    connect(myScene->memGameTimer,&QTimer::timeout, this,[=](){
        if (m_isGamePaused){
            this->stopMoving();
            return;
        }
        if (x() < 20) {
            emit arrivedYourHome();
            this->stopMoving();
            return;
        }
        if(checkCollision()){
            //攻击逻辑
            stopMoving();
            targetWhiteDog->cutHp(atkPower);
            if(targetWhiteDog->getAtkType()){ getAttacked(atkPower * 0.8); } //跳舞小狗反弹80%伤害
            if(targetWhiteDog->getHp() <= 0){
                targetWhiteDog->removeItself();
                targetWhiteDog = nullptr;
            }
        }
        else{
            startMoving();
        }
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
    targetWhiteDog = nullptr;
    return false; // 没有找到碰撞的 WhiteDogs
}

void YellowDogs::getAttacked(int atk){
    cutHp(atk);
    emit isAttacked();
}


void YellowDogs::gamePaused(){
    if(movie){
        movie->setPaused(true);
    }
    m_isGamePaused = true;
    stopMoving();
}

void YellowDogs::gameContinued(){
    if(movie){
        movie->setPaused(false);
    }
    m_isGamePaused = false;
    startMoving();
}
