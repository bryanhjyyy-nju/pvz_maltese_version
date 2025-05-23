#include "mainscene.h"
#include "ui_mainscene.h"
#include <QPainter>
#include "mypushbutton.h"
#include <QDebug>

MainScene::MainScene(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainScene)
{
    ui->setupUi(this);

    //配置主场景

    //设置固定大小
    setFixedSize(1650,900);

    //设置窗口图标
    setWindowIcon(QIcon(":/Image/dogIcon.jpg"));

    //设置窗口标题
    setWindowTitle("PvZ_Demo");

    //添加开始按钮
    MyPushButton * startBtn = new MyPushButton(":/Image/StartBtn.png");
    startBtn->setParent(this);
    startBtn->move(this->width() * 0.5 - startBtn->width() * 0.5,this->height() * 0.55);

    //开始按钮弹跳特效
    connect(startBtn,&MyPushButton::clicked,[=](){
        startBtn->zoom1();
        startBtn->zoom2();
    });

    //添加退出游戏按钮
    MyPushButton * quitBtn = new MyPushButton(":/Image/QuitBtn.png");
    quitBtn->setParent(this);
    quitBtn->move(this->width() * 0.5 - quitBtn->width() * 0.5,this->height() * 0.55 + startBtn->height() * 1.05);

    //退出按钮按下时弹跳特效
    connect(quitBtn,&MyPushButton::pressed,[=](){
        quitBtn->zoom1();
        quitBtn->zoom2();
    });

    //实现退出功能
    connect(quitBtn,&MyPushButton::clicked,this,&MainScene::close);
}

void MainScene::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pix;

    //背景图片
    pix.load(":/Image/StartPage.jpg");

    painter.drawPixmap(0,0,this->width(),this->height(),pix);

    //绘制游戏标题
    pix.load(":/Image/Title.png");
    pix = pix.scaled(pix.width() * 2,pix.height() * 2);
    painter.drawPixmap(this->width() * 0.5 - pix.width() * 0.5,this->height() * 0.2,pix);

}

MainScene::~MainScene()
{
    delete ui;
}
