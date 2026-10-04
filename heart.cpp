//heart.cpp
#include "heart.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QEasingCurve>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include "mygamescene.h"
#include "gameartwork.h"

QPointF Heart::curMousePos = QPointF(0, 0);

Heart::Heart(QPointF startPos, QPointF endPos, MyGameScene *gameScene, QEasingCurve::Type type, QObject *parent,bool tutorial,int fallDurationMs):QObject(parent), memEndPos(endPos)
{
    setPixmap(GameArtwork::cuteHeart());
    setOpacity(1.0);
    setPos(startPos); //设置起始位置
    setZValue(10); //确保在最上层

    //设置仅允许使用鼠标左键点击
    setAcceptedMouseButtons(Qt::LeftButton);

    //创建下落动画
    memFallAnim = new QPropertyAnimation(this, "pos", this);
    memFallAnim->setObjectName("heartFallAnimation");
    memFallAnim->setDuration(qMax(1,fallDurationMs));
    memFallAnim->setStartValue(startPos);
    memFallAnim->setEndValue(endPos);
    memFallAnim->setEasingCurve(type);

    //创建收集动画
    memCollectAnim = new QPropertyAnimation(this, "pos", this);
    memCollectAnim->setObjectName("heartCollectAnimation");
    memCollectAnim->setDuration(800);
    memCollectAnim->setEasingCurve(QEasingCurve::InQuad);

    //创建爱心消失计时器
    memDisappearTimer = new QTimer(this);
    memDisappearTimer->setInterval(3500);
    memDisappearTimer->setSingleShot(true);

    //链接信号和曹
    //下落结束链接已经到达地面

    // qDebug() << "1";
    connect(memFallAnim, &QPropertyAnimation::finished, this, &Heart::hasReachedGround);

    //倒计时结束链接已经消失
    memBlinkAnim = new QPropertyAnimation(this,"opacity",this);
    memBlinkAnim->setObjectName("heartBlinkAnimation");
    memBlinkAnim->setDuration(500);
    memBlinkAnim->setStartValue(1.0);
    memBlinkAnim->setKeyValueAt(.5,.2);
    memBlinkAnim->setEndValue(1.0);
    memBlinkAnim->setLoopCount(2);
    connect(memDisappearTimer, &QTimer::timeout, memBlinkAnim, [this] { memBlinkAnim->start(); });
    connect(memBlinkAnim, &QPropertyAnimation::finished, this, &Heart::hasDisappear);

    // qDebug() << "2";
    //收集动作结束链接删去阳光
    connect(memCollectAnim, &QPropertyAnimation::finished, this,[=](){
        emit collected();
        scene()->removeItem(this);
        deleteLater();
    });

    if(!tutorial) startFall();

    connect(gameScene,&MyGameScene::sceneClicked, this,[=](){
        if (this->x() < curMousePos.x()
            && this->y() < curMousePos.y()
            && this->x() + this->boundingRect().width() > curMousePos.x()
            && this->y() + this->boundingRect().height() > curMousePos.y()
            && isCollectable){
            memDisappearTimer->stop();
            memBlinkAnim->stop();
            setOpacity(1.0);
            // Collection owns the position immediately, even during a slow fall.
            memFallAnim->stop();
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
    fadeAnim->setStartValue(opacity());
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
    if(memBlinkAnim->state()==QAbstractAnimation::Running) memBlinkAnim->pause();
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
    if(memBlinkAnim->state()==QAbstractAnimation::Paused) memBlinkAnim->resume();
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



































