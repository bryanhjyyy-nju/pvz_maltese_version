#include "mainscene.h"
#include "ui_mainscene.h"
#include <QPainter>
#include "mypushbutton.h"
#include <QDebug>
#include <QMovie>
#include <QLabel>
#include <QTimer>

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

    //实例化选择关卡的场景
    chooseScene = new ChooseLevelScene;

    //监听选择界面返回按钮信号
    connect(chooseScene,&ChooseLevelScene::chooseSceneBack,this,[=](){
        chooseScene->hide();
        this->show();
    });

    //开始按钮弹跳特效
    connect(startBtn,&MyPushButton::clicked,this,[=](){
        startBtn->zoom1();
        startBtn->zoom2();

        //延时进入关卡场景
        QTimer::singleShot(300,this,[=]{
            //自身隐藏
            this->hide();
            //显示选择关卡场景
            chooseScene->show();
        });
    });

    //添加退出游戏按钮
    MyPushButton * quitBtn = new MyPushButton(":/Image/QuitBtn.png");
    quitBtn->setParent(this);
    quitBtn->move(this->width() * 0.5 - quitBtn->width() * 0.5,this->height() * 0.55 + startBtn->height() * 1.05);

    //退出按钮按下时弹跳特效
    connect(quitBtn,&MyPushButton::clicked,this,[=](){
        quitBtn->zoom1();
        quitBtn->zoom2();
        QTimer::singleShot(300,this,[=]{this->close();});
    });

    //插入两个动画
    QMovie *movie1 = new QMovie(":/Image/startDogs.gif");
    QLabel *label1 = new QLabel(this);
    label1->setMovie(movie1);
    label1->setFixedSize(QSize(400,400));
    label1->setScaledContents(true);
    label1->move(this->width() * 0.05,this->height() * 0.45);
    movie1->start();

    QMovie *movie2 = new QMovie(":/Image/startDogs.gif");
    QLabel *label2 = new QLabel(this);
    label2->setMovie(movie2);
    label2->setFixedSize(QSize(400,400));
    label2->setScaledContents(true);
    label2->move(this->width() * 0.7,this->height() * 0.45);
    movie2->start();
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
