#include "audiomanager.h"
#include "chooselevelscene.h"
#include "almanacdialog.h"
#include "gameui.h"
#include <QPainter>
#include <QPushButton>
#include <QDebug>
#include <QLabel>
#include "progressstore.h"

ChooseLevelScene::ChooseLevelScene(QWidget *parent) : GamePage(parent)
{

    //返回按钮
    buildBackBtn();

    //每一关对应的按钮
    buildLevelBtn();
    continueButton = new QPushButton(this);
    GameUi::styleButton(continueButton);
    continueButton->setObjectName("continueGame");
    continueButton->setGeometry(620, 785, 400, 48);
    connect(continueButton, &QPushButton::clicked, this, &ChooseLevelScene::continueRequested);
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
    initializePage(QRect(1170,30,200,44));
}

void ChooseLevelScene::paintEvent(QPaintEvent *){
    QPainter painter(this);
    prepareCanvasPaint(painter);
    QPixmap pix;

    //背景图片
    bool ret = pix.load(":/others/Image/StartPage.jpg");
    if(!ret){
        qDebug() << "图片加载失败" ;
        return;
    }

    painter.drawPixmap(0,0,1650,900,pix);

    //绘制关卡选择四个字
    ret = pix.load(":others/Image/chooseTitle.png");
    if(!ret){
        qDebug() << "图片加载失败" ;
        return;
    }
    pix = pix.scaled(pix.width() * 2,pix.height() * 2);
    painter.drawPixmap(1650 * 0.2 - pix.width() * 0.5,900 * 0.1,pix);

}

void ChooseLevelScene::buildBackBtn() {
    auto *back=new QPushButton("返回首页",this);
    GameUi::styleButton(back,"gold");
    back->setObjectName("backToMenu");
    back->setGeometry(1440,25,180,50);
    connect(back,&QPushButton::clicked,this,&ChooseLevelScene::backRequested);
}

void ChooseLevelScene::buildLevelBtn() {
    for(int level=1;level<=10;++level) {
        auto *button=new QPushButton(QString("%1\n关卡").arg(level),this);
        button->setObjectName(QString("level%1").arg(level));
        GameUi::styleButton(button,"level");
        const int index=level-1;
        button->setGeometry(120+300*(index%5),200+300*(index/5),180,180);
        connect(button,&QPushButton::clicked,this,[this,level] {
            AudioManager::instance().play("click");
            emit levelRequested(level);
        });
    }
}

void ChooseLevelScene::refreshProgress(const ProgressStore& progress) {
    continueButton->setEnabled(progress.hasProgress());
    continueButton->setText(QString("继续游戏 · 第 %1 关").arg(progress.resumeLevel()));
    progressLabel->setText(progress.error().isEmpty()
        ? QString("最高通过：%1 / 10 关 · 自动记录关卡进度；继续游戏会从该关开局。%2")
            .arg(progress.highestCompleted()).arg(progress.highestCompleted() == 10 ? "  已全部通关！" : "")
        : progress.error());
}
