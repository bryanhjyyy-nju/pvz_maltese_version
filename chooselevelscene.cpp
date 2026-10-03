#include "audiomanager.h"
#include "chooselevelscene.h"
#include "almanacdialog.h"
#include "gameui.h"
#include <QMessageBox>
#include <QPainter>
#include "mypushbutton.h"
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include "playscene.h"

ChooseLevelScene::ChooseLevelScene(QWidget *parent, const QString& progressPath)
    : QMainWindow{parent}, progress(progressPath)
{
    //设置固定大小
    setFixedSize(1650,900);

    //设置窗口图标
    setWindowIcon(QIcon(":/white/Image/dogIcon.jpg"));

    //设置窗口标题
    setWindowTitle("PvZ_Demo");

    //返回按钮
    buildBackBtn();

    //每一关对应的按钮
    buildLevelBtn();
    continueButton = new QPushButton(this);
    GameUi::styleButton(continueButton);
    continueButton->setObjectName("continueGame");
    continueButton->setGeometry(620, 785, 400, 48);
    connect(continueButton, &QPushButton::clicked, this, &ChooseLevelScene::continueGame);
    progressLabel = new QLabel(this);
    progressLabel->setGeometry(280, 840, 1100, 45);
    progressLabel->setAlignment(Qt::AlignCenter);
    progressLabel->setStyleSheet("background:#fff0c8; color:#65452d; border:2px solid #997341; border-radius:14px; font:16px 'Microsoft YaHei'; padding:5px;");
    auto *almanac = new QPushButton("植物 / 僵尸图鉴", this);
    GameUi::styleButton(almanac,"gold");
    almanac->setGeometry(1090, 785, 210, 48);
    connect(almanac, &QPushButton::clicked, this, [this] {
        AlmanacDialog dialog(this);
        dialog.exec();
    });
    refreshProgress();
}

void ChooseLevelScene::paintEvent(QPaintEvent *){
    QPainter painter(this);
    QPixmap pix;

    //背景图片
    bool ret = pix.load(":/others/Image/StartPage.jpg");
    if(!ret){
        qDebug() << "图片加载失败" ;
        return;
    }

    painter.drawPixmap(0,0,this->width(),this->height(),pix);

    //绘制关卡选择四个字
    ret = pix.load(":others/Image/chooseTitle.png");
    if(!ret){
        qDebug() << "图片加载失败" ;
        return;
    }
    pix = pix.scaled(pix.width() * 2,pix.height() * 2);
    painter.drawPixmap(this->width() * 0.2 - pix.width() * 0.5,this->height() * 0.1,pix);

}

void ChooseLevelScene::buildBackBtn(){
    //返回按钮
    MyPushButton *backBtn = new MyPushButton(":others/Image/backBtn.png");
    backBtn->setParent(this);
    backBtn->move(this->width() - backBtn->width() * 1.2,backBtn->width() * 0.2);

    connect(backBtn,&MyPushButton::clicked,this,[=](){
        //点击动画
        backBtn->zoom1();
        backBtn->zoom2();

        //延时返回
        QTimer::singleShot(300,this,[=](){
            emit this->chooseSceneBack();
        });
    });
}

void ChooseLevelScene::buildLevelBtn(){
    //创建关卡按钮十个
    for(int i = 0; i < 10; i++){
        MyPushButton *levelBtn = new MyPushButton(":others/Image/levelIcon.png");
        levelBtn->setParent(this);
        levelBtn->move(300 * (i % 5) + 120, 200 + i / 5 * 300);


        //监听每个按钮的点击事件
        connect(levelBtn,&MyPushButton::clicked,this,[=](){
            // qDebug() << i + 1;

            startLevel(i + 1);
        });

        //显示文字：第 i 关
        QLabel *label = new QLabel;
        label->setParent(this);
        label->setFixedSize(levelBtn->width(),levelBtn->height());
        label->setText(QString::number(i + 1));

        //设置字体颜色和大小
        QFont font;
        font.setFamily("Arial");
        font.setPointSize(45);
        font.setBold(true);
        label->setFont(font);
        label->setStyleSheet("color: pink;");

        label->move(300 * (i % 5) + 120, 200 + i / 5 * 300);
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        //使得鼠标能够穿透label
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
}

void ChooseLevelScene::refreshProgress() {
    progress.load();
    continueButton->setEnabled(progress.hasProgress());
    continueButton->setText(QString("继续游戏 · 第 %1 关").arg(progress.resumeLevel()));
    progressLabel->setText(progress.error().isEmpty()
        ? QString("最高通过：%1 / 10 关 · 自动记录关卡进度；继续游戏会从该关开局。%2")
            .arg(progress.highestCompleted()).arg(progress.highestCompleted() == 10 ? "  已全部通关！" : "")
        : progress.error());
}

void ChooseLevelScene::continueGame() {
    refreshProgress();
    if(progress.hasProgress()) startLevel(progress.resumeLevel());
}

void ChooseLevelScene::startLevel(int level) {
    if(play) return;
    if(!progress.startLevel(level))
        QMessageBox::warning(this, "存档未写入", progress.error());
    play = new PlayScene(level);
    connect(play, &PlayScene::gameWin, this, [this, level] {
        if(!progress.completeLevel(level))
            QMessageBox::warning(play, "存档未写入", progress.error());
    });
    connect(play, &PlayScene::playSceneBack, this, [this] {
        if(!play) return;
        AudioManager::instance().setBattle(false);
        play->hide();
        play->deleteLater();
        play = nullptr;
        refreshProgress();
        show();
    });
    connect(play, &PlayScene::mainMenuRequested, this, [this] {
        if(!play) return;
        AudioManager::instance().setBattle(false);
        play->hide();
        play->deleteLater();
        play = nullptr;
        refreshProgress();
        emit chooseSceneBack();
    });
    hide();
    play->show();
}

ChooseLevelScene::~ChooseLevelScene() {
    delete play;
}
