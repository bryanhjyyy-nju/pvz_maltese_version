#include "audiomanager.h"
#include "mainscene.h"
#include "almanacdialog.h"
#include "gameui.h"
#include <QPainter>
#include "gameartwork.h"
#include <QPushButton>
#include <QDebug>
#include <QMovie>
#include <QLabel>
#include <QTimer>

MainScene::MainScene(QWidget *parent) : GamePage(parent)
{
    //设置窗口图标
    setWindowIcon(QIcon(":/white/Image/dogIcon.jpg"));

    //设置窗口标题
    setWindowTitle("小白大战小金毛");

    //添加开始按钮
    buildStartBtn();

    //添加退出游戏按钮
    buildQuitBtn();

    auto *resume = new QPushButton("继续上次关卡", this);
    GameUi::styleButton(resume);
    resume->setObjectName("resumeGame");
    resume->setGeometry(620, 795, 190, 48);
    connect(resume, &QPushButton::clicked, this, [this] {
        emit continueRequested();
    });
    auto *almanac = new QPushButton("植物 / 僵尸图鉴", this);
    GameUi::styleButton(almanac,"gold");
    almanac->setGeometry(830, 795, 210, 48);
    connect(almanac, &QPushButton::clicked, this, [this] {
        AlmanacDialog dialog(this);
        dialog.exec();
    });
    auto *sound = new QPushButton("声音设置", this);
    GameUi::styleButton(sound,"gold");
    sound->setGeometry(1390, 30, 180, 44);
    connect(sound, &QPushButton::clicked, this, [this] { AudioManager::instance().showSettings(this); });
    //插入两个动画
    setGif(400,400,this->width() * 0.05,this->height() * 0.45);
    setGif(400,400,this->width() * 0.7,this->height() * 0.45);
    initializePage(QRect(1170,30,200,44));

}

void MainScene::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    prepareCanvasPaint(painter);
    QPixmap pix;

    //背景图片
    pix.load(":/others/Image/StartPage.jpg");

    painter.drawPixmap(0,0,1650,900,pix);

    GameArtwork::drawTitle(painter,QRectF(325,155,1000,235),"小白大战小金毛");

}

void MainScene::buildStartBtn() {
    auto *startBtn=new QPushButton("开始游戏",this);
    startBtn->setObjectName("startGame");
    GameUi::styleButton(startBtn,"sunshine");
    startBtn->setGeometry(610,495,430,84);
    connect(startBtn,&QPushButton::clicked,this,[this] {
        AudioManager::instance().play("click");
        emit startRequested();
    });
}

void MainScene::buildQuitBtn() {
    auto *quitBtn = new QPushButton("退出游戏",this);
    quitBtn->setObjectName("quitGame");
    GameUi::styleButton(quitBtn,"sunshine");
    quitBtn->setGeometry(610,595,430,84);
    connect(quitBtn,&QPushButton::clicked,this,[this] {
        AudioManager::instance().play("click");
        emit quitRequested();
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
