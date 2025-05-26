#include "mypushbutton.h"
#include <QDebug>
#include <QPropertyAnimation>

MyPushButton::MyPushButton(QString firstImg, QString secondImg, float times){
    this->firstImgPath = firstImg;
    this->secondImgPath = secondImg;

    //图片加载
    QPixmap pix;
    bool ret = pix.load(firstImg);
    if(!ret){
        qDebug() << "图片加载失败" ;
        return;
    }

    pix = pix.scaled(pix.width() * times,pix.height() * times);

    //设置图片固定大小
    this->setFixedSize(pix.width(),pix.height());

    //设置不规则图片样式
    this->setStyleSheet("QPushButton{border:0px;}");

    //设置图标
    this->setIcon(pix);

    //设置图标大小
    this->setIconSize(QSize(pix.width(),pix.height()));
}

void MyPushButton::zoom1(){
    //创建动态对象
    QPropertyAnimation *animation = new QPropertyAnimation(this,"geometry");

    //设置动画时间间隔
    animation->setDuration(200);

    //起始位置
    animation->setStartValue(QRect(this->x(),this->y(),this->width(),this->height()));

    //结束位置
    animation->setEndValue(QRect(this->x(),this->y()+10,this->width(),this->height()));

    //设置弹跳曲线
    animation->setEasingCurve(QEasingCurve::OutBounce);

    //执行动画
    animation->start();
}

void MyPushButton::zoom2(){
    //创建动态对象
    QPropertyAnimation *animation = new QPropertyAnimation(this,"geometry");

    //设置动画时间间隔
    animation->setDuration(200);

    //起始位置
    animation->setStartValue(QRect(this->x(),this->y()+10,this->width(),this->height()));

    //结束位置
    animation->setEndValue(QRect(this->x(),this->y(),this->width(),this->height()));

    //设置弹跳曲线
    animation->setEasingCurve(QEasingCurve::OutBounce);

    //执行动画
    animation->start();
}
