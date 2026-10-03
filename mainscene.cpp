#include "mainscene.h"
#include "audiomanager.h"
#include "almanacdialog.h"
#include "gameui.h"
#include "gameartwork.h"
#include <QPainter>
#include <QPushButton>
#include <QMovie>
#include <QLabel>

MainScene::MainScene(QWidget *parent) : GamePage(parent) {
    auto button=[this](const QString& text,const QString& name,int y) {
        auto *result=new QPushButton(text,this);
        result->setObjectName(name); GameUi::styleButton(result,"sunshine");
        result->setGeometry(610,y,430,84);
        connect(result,&QPushButton::clicked,this,[] { AudioManager::instance().play("click"); });
        return result;
    };
    auto *select=button("选择关卡","startGame",430);
    connect(select,&QPushButton::clicked,this,&MainScene::startRequested);
    resumeButton=button("继续游戏","resumeGame",530);
    connect(resumeButton,&QPushButton::clicked,this,&MainScene::continueRequested);
    auto *quit=button("退出游戏","quitGame",630);
    connect(quit,&QPushButton::clicked,this,&MainScene::quitRequested);
    auto *almanac=button("植物 / 僵尸图鉴","menuAlmanac",730);
    connect(almanac,&QPushButton::clicked,this,[this] { AlmanacDialog dialog(this); dialog.exec(); });
    endlessButton=button("开始无尽模式","endlessGame",330);
    connect(endlessButton,&QPushButton::clicked,this,&MainScene::endlessRequested);
    auto *sound=new QPushButton(this); sound->setObjectName("audioSettings");
    sound->setToolTip("声音设置"); sound->setAccessibleName("声音设置");
    GameUi::styleButton(sound,"gold"); sound->setGeometry(1510,25,90,70);
    sound->setIcon(GameArtwork::speakerIcon()); sound->setIconSize(QSize(46,46));
    connect(sound,&QPushButton::clicked,this,[this] { AudioManager::instance().showSettings(this); });
    auto *save=new QPushButton("存档管理",this); save->setObjectName("saveSettings");
    GameUi::styleButton(save,"gold"); save->setGeometry(30,830,220,44);
    connect(save,&QPushButton::clicked,this,&MainScene::saveSettingsRequested);
    recordLabel=new QLabel(this); recordLabel->setObjectName("endlessRecord");
    recordLabel->setGeometry(510,840,640,40); recordLabel->setAlignment(Qt::AlignCenter);
    recordLabel->setStyleSheet("background:#fff0c8;color:#65452d;border-radius:12px;font:bold 20px 'Microsoft YaHei';");
    setGif(400,400,82,405); setGif(400,400,1155,405);
    initializePage(QRect(30,30,230,44)); refreshState(false,false,false,0);
}
void MainScene::refreshState(bool campaign,bool unlocked,bool endless,int best) {
    resumeButton->setEnabled(campaign);
    endlessButton->setVisible(unlocked);
    endlessButton->setText(endless ? "继续无尽模式" : "开始无尽模式");
    recordLabel->setVisible(unlocked);
    recordLabel->setText(QString("无尽模式最高纪录 · 第 %1 波").arg(best));
}
void MainScene::paintEvent(QPaintEvent*) {
    QPainter painter(this); prepareCanvasPaint(painter);
    painter.drawPixmap(0,0,1650,900,QPixmap(":/others/Image/StartPage.jpg"));
    GameArtwork::drawTitle(painter,QRectF(325,90,1000,215),"小白大战小金毛");
}
void MainScene::setGif(int w,int h,int x,int y) {
    auto *movie=new QMovie(":/others/Image/startDogs.gif",QByteArray(),this);
    auto *label=new QLabel(this); label->setMovie(movie);
    label->setFixedSize(w,h); label->setScaledContents(true); label->move(x,y); movie->start();
}
