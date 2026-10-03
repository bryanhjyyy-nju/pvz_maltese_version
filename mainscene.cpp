#include "audiomanager.h"
#include "mainscene.h"
#include "almanacdialog.h"
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
    setWindowIcon(QIcon(":/white/Image/dogIcon.jpg"));

    //设置窗口标题
    setWindowTitle("PvZ_Demo");

    //添加开始按钮
    buildStartBtn();

    //添加退出游戏按钮
    buildQuitBtn();

    auto *resume = new QPushButton("继续上次关卡", this);
    resume->setObjectName("resumeGame");
    resume->setGeometry(620, 795, 190, 48);
    connect(resume, &QPushButton::clicked, this, [this] {
        hide();
        chooseScene->show();
        chooseScene->continueGame();
    });
    auto *almanac = new QPushButton("植物 / 僵尸图鉴", this);
    almanac->setGeometry(830, 795, 210, 48);
    connect(almanac, &QPushButton::clicked, this, [this] {
        AlmanacDialog dialog(this);
        dialog.exec();
    });
    auto *sound = new QPushButton("声音设置", this);
    sound->setGeometry(1390, 30, 180, 44);
    connect(sound, &QPushButton::clicked, this, [this] { AudioManager::instance().showSettings(this); });
    //插入两个动画
    setGif(400,400,this->width() * 0.05,this->height() * 0.45);
    setGif(400,400,this->width() * 0.7,this->height() * 0.45);

}

void MainScene::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pix;

    //背景图片
    pix.load(":/others/Image/StartPage.jpg");

    painter.drawPixmap(0,0,this->width(),this->height(),pix);

    //绘制游戏标题
    pix.load(":/others/Image/Title.png");
    pix = pix.scaled(pix.width() * 2,pix.height() * 2);
    painter.drawPixmap(this->width() * 0.5 - pix.width() * 0.5,this->height() * 0.2,pix);
}

void MainScene::buildStartBtn(){
    //添加开始按钮
    MyPushButton * startBtn = new MyPushButton(":/others/Image/StartBtn.png");
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
}

void MainScene::buildQuitBtn(){
    //添加退出游戏按钮
    MyPushButton * quitBtn = new MyPushButton(":/others/Image/QuitBtn.png");
    quitBtn->setParent(this);
    quitBtn->move(this->width() * 0.5 - quitBtn->width() * 0.5,this->height() * 0.55 + quitBtn->height() * 1.05);

    //退出按钮按下时弹跳特效
    connect(quitBtn,&MyPushButton::clicked,this,[=](){
        quitBtn->zoom1();
        quitBtn->zoom2();
        QTimer::singleShot(300,this,[=]{this->close();});
    });
}

void MainScene::setGif(int w,int h,int x,int y){
    //设置动画
    QMovie *movie = new QMovie(":/others/Image/startDogs.gif", QByteArray(), this);
    QLabel *label = new QLabel(this);
    label->setMovie(movie);
    label->setFixedSize(QSize(w,h));
    label->setScaledContents(true);
    label->move(x,y);
    movie->start();
}

MainScene::~MainScene()
{
    delete chooseScene;
    delete ui;
}
