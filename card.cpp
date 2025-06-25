#include "card.h"
#include <QDebug>
#include <QPainter>
#include "gamestate.h"
#include <QTimer>
#include <QGraphicsEffect>
#include "cardstate.h"

GameState Card::cardGameState = GameState::Normal;
QString Card::cardSelectedWhite = "";
int Card::curRestHeart = 50;

Card::Card(int cardNum):whiteType("") ,coolTime(0) ,heartCost(0),cardIndex(cardNum) {
    //图片加载
    QPixmap pix;
    bool ret = pix.load(":/others/Image/card.png");
    if(!ret){
        qDebug() << "图片加载失败" ;
        return;
    }
    //图片缩放
    pix = pix.scaled(pix.width() * 1.21,pix.height() * 1.21);

    //设置图片固定大小
    this->setFixedSize(pix.width(),pix.height());

    //设置不规则图片样式
    this->setStyleSheet("QPushButton{border:0px;}");

    //设置图标
    this->setIcon(pix);

    //设置图标大小
    this->setIconSize(QSize(pix.width(),pix.height()));

    // 初始化冷却计时器
    memCoolTimer = new QTimer(this);
    memCoolTimer->setInterval(100); // 每100ms更新一次
    connect(memCoolTimer, &QTimer::timeout, this, [this]() {
        memCoolProgress += 100.0f / coolTime; // 计算进度
        if (memCoolProgress >= 1.0f) {
            memCoolProgress = 1.0f;
            memCoolTimer->stop();
            coolingState = false;
            update();
            // setCardState(CardState::Normal); //发送信号以后自然会设置，此处没必要
            emit cooldownFinished();
        }
        update(); // 触发重绘
    });

    connect(this, &Card::checkHeartEnough, this, [=](){
        if(getCardState() == CardState::Normal){
            if(curRestHeart < heartCost) {
                setEnabled(false);
                setCardState(CardState::Unable);
                memHeartIsEnough = false;
                updateLackingEffect(); //更新爱心不足效果
            }
        }
        else if(getCardState() == CardState::Unable){
            if(curRestHeart >= heartCost) {
                setEnabled(true);
                setCardState(CardState::Normal);
                memHeartIsEnough = true;
                updateNormalEffect(); //更新正常效果
            }
        }
    }); //每次收集爱心以后检查爱心是否足够

    connect(this, &Card::cooldownFinished, this, [=](){
        coolingState = false;
        if(curRestHeart < heartCost) {
            memHeartIsEnough = false;
        }
        else {
            memHeartIsEnough = true;
        }
        if(memHeartIsEnough){
            setCardState(CardState::Normal);
            setEnabled(true);
            updateNormalEffect(); //更新正常效果
        }
        else{
            // setEnabled(false);
            setCardState(CardState::Unable);
            updateLackingEffect(); //更新未激活效果
        }
    });


    connect(this, &Card::clicked, [this]() {
        emit cardSelected(this);
    });

}

void Card::mousePressEvent(QMouseEvent *e)
{
    // 只在正常状态下处理点击
    if (cardGameState == GameState::Normal) {
        QPushButton::mousePressEvent(e);
    }
}

void Card::updateCoolingEffect(){
    // 创建半透明效果
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.5); // 半透明
    setGraphicsEffect(effect);
}

void Card::updateNormalEffect(){
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(1.0); // 不透明
    setGraphicsEffect(effect);
}

void Card::updateLackingEffect(){
    // 创建半透明效果
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.93); // 几乎不透明
    setGraphicsEffect(effect);
}

void Card::setCoolProgress(float progress){
    memCoolProgress = progress;
}

void Card::startCooldown(){
    if (coolingState) return;

    coolingState = true;
    memCoolProgress = 0.0f; //冷却进度从0开始

    // 启动冷却计时器
    memCoolTimer->start();

    // 禁用按钮
    setEnabled(false);

    //设置冷却状态
    setCardState(CardState::Cooling);

    // 更新冷却效果
    updateCoolingEffect();
}

void Card::gamePaused(){
    if(memCoolTimer){
        if(memCoolTimer->isActive()){
            memCoolTimer->stop();
        }
    }
}

void Card::gameContinued(){
    if(memCoolTimer){
        if(!memCoolTimer->isActive() && isCooling()){
            memCoolTimer->start();
        }
    }
}
