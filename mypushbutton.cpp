#include "mypushbutton.h"
#include <QDebug>

MyPushButton::MyPushButton(QWidget *parent)
    : QPushButton(parent)
{}

MyPushButton::MyPushButton(QString normalImg, QString pressImg){
    this->normalImgPath = normalImg;
    this->pressImgPath = pressImg;

    //图片加载
    QPixmap pix;
    bool ret = pix.load(normalImg);
    if(!ret){
        qDebug() << "图片加载失败" ;
        return;
    }

    //设置图片固定大小
    this->setFixedSize(pix.width(),pix.height());

    //设置不规则图片样式
    this->setStyleSheet("QPushButton{border:0px;}");

    //设置图标
    this->setIcon(pix);

    //设置图标大小
    this->setIconSize(QSize(pix.width(),pix.height()));


}
