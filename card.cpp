#include "card.h"
#include <QDebug>
#include <QPainter>

Card::Card(int cardNum):whiteType("") ,coolTime(0) ,heartCost(0),cardIndex(cardNum){
    //图片加载
    QPixmap pix;
    bool ret = pix.load(":/Image/card.png");
    if(!ret){
        qDebug() << "图片加载失败" ;
        return;
    }

    pix = pix.scaled(pix.width() * 1.21,pix.height() * 1.21);

    //设置图片固定大小
    this->setFixedSize(pix.width(),pix.height());

    //设置不规则图片样式
    this->setStyleSheet("QPushButton{border:0px;}");

    //设置图标
    this->setIcon(pix);

    //设置图标大小
    this->setIconSize(QSize(pix.width(),pix.height()));
}

