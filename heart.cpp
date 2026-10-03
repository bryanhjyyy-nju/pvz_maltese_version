//heart.cpp
#include "heart.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QEasingCurve>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include "mygamescene.h"

QPointF Heart::curMousePos = QPointF(0, 0);

Heart::Heart(QPointF startPos, QPointF endPos, MyGameScene *gameScene, QEasingCurve::Type type, QObject *parent):QObject(parent), memEndPos(endPos)
{
    //设置阳光图片
    QPixmap pix;
    pix.load(":/others/Image/heart.png");
    pix = pix.scaled(pix.width() * 0.6, pix.height() * 0.6);
    setPixmap(pix);
    setPos(startPos); //设置起始位置
    setZValue(10); //确保在最上层

    //设置仅允许使用鼠标左键点击
    setAcceptedMouseButtons(Qt::LeftButton);

    //创建下落动画
    memFallAnim = new QPropertyAnimation(this, "pos", this);
    memFallAnim->setDuration(3000);
    memFallAnim->setStartValue(startPos);
    memFallAnim->setEndValue(endPos);
    memFallAnim->setEasingCurve(type);

    //创建收集动画
    memCollectAnim = new QPropertyAnimation(this, "pos", this);
    memCollectAnim->setDuration(800);
    memCollectAnim->setEasingCurve(QEasingCurve::InQuad);

    //创建爱心消失计时器
    memDisappearTimer = new QTimer(this);
    memDisappearTimer->setInterval(4500);
    memDisappearTimer->setSingleShot(true);

    //链接信号和曹
    //下落结束链接已经到达地面

    // qDebug() << "1";
    connect(memFallAnim, &QPropertyAnimation::finished, this, &Heart::hasReachedGround);

    //倒计时结束链接已经消失
    connect(memDisappearTimer, &QTimer::timeout, this, &Heart::hasDisappear);

    // qDebug() << "2";
    //收集动作结束链接删去阳光
    connect(memCollectAnim, &QPropertyAnimation::finished, this,[=](){
        emit collected();
        scene()->removeItem(this);
        deleteLater();
    });

    startFall();

    connect(gameScene,&MyGameScene::sceneClicked, this,[=](){
        if (this->x() < curMousePos.x()
            && this->y() < curMousePos.y()
            && this->x() + this->boundingRect().width() > curMousePos.x()
            && this->y() + this->boundingRect().height() > curMousePos.y()
            && isCollectable){
            memDisappearTimer->stop();
            isCollectable = false;
            //收集动画
            memCollectAnim->setStartValue(pos());
            memCollectAnim->setEndValue(QPointF(380,100));

            memCollectAnim->start();
        }
    });
}

void Heart::startFall(){
    memFallAnim->start();
}

QRectF Heart::boundingRect() const {
    return pixmap().rect();
}

void Heart::hasReachedGround(){
    memDisappearTimer->start();
}

void Heart::hasDisappear(){
    isCollectable = false;
    isDisappearing = true;
    QPropertyAnimation *fadeAnim = new QPropertyAnimation(this, "opacity", this);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);
    fadeAnim->setDuration(1000);
    fadeAnim->start(QPropertyAnimation::DeleteWhenStopped);

    //链接信号和槽
    connect(fadeAnim, &QPropertyAnimation::finished, this,[=](){
        scene()->removeItem(this);
        emit i_have_disappeared();
        deleteLater();
    });
}

void Heart::gamePaused(){
    if(memCollectAnim){
        if(memCollectAnim->state() == memCollectAnim->Running){
            memCollectAnim->setPaused(true);
        }
    }
    if(memFallAnim){
        if(memFallAnim->state() == memFallAnim->Running){
            memFallAnim->setPaused(true);
            isCollectable = false;
        }
    }
    if(memDisappearTimer){
        if(memDisappearTimer->isActive()){
            memDisappearTimer->stop();
            isCollectable = false;
        }
    }
}

void Heart::gameContinued(){
    if(memCollectAnim){
        if(memCollectAnim->state() == memCollectAnim->Paused){
            memCollectAnim->setPaused(false);
        }
    }
    if(memFallAnim){
        if(memFallAnim->state() == memFallAnim->Paused){
            memFallAnim->setPaused(false);
            isCollectable = true;
        }
        else if(memFallAnim->state() == memFallAnim->Stopped && !isDisappearing){
            if(memDisappearTimer){
                if(!memDisappearTimer->isActive()){
                    memDisappearTimer->start();
                    isCollectable = true;
                }
            }
        }
    }
}



































