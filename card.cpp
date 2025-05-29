#include "card.h"
#include <QDebug>
#include <QPainter>
#include "gamestate.h"

GameState Card::cardGameState = GameState::Normal;
QString Card::cardSelectedWhite = "";

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

    switch(cardNum) {
    case 0:
        whiteType = "singingWhite";
        coolTime = 7500;
        heartCost = 100;
        break;
    case 1:
        whiteType = "heartWhite";
        coolTime = 7500;
        heartCost = 50;
        break;
        //todo : 添加更多卡牌
    }

    connect(this, &Card::clicked, [this]() {
        if (cardGameState == GameState::Normal) {
            emit cardSelected(this);
        }
    });

}

void Card::mousePressEvent(QMouseEvent *e)
{
    // 只在正常状态下处理点击
    if (cardGameState == GameState::Normal) {
        QPushButton::mousePressEvent(e);
    }
}

void Card::setGameState(GameState state) {
    cardGameState = state;
}

GameState Card::currentState() {
    return cardGameState;
}

void Card::setSelectedWhite(const QString& sWhite) {
    cardSelectedWhite = sWhite;
}

QString Card::selectedWhite() {
    return cardSelectedWhite;
}


