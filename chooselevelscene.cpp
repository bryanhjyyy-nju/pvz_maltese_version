#include "audiomanager.h"
#include "chooselevelscene.h"
#include "almanacdialog.h"
#include "gameui.h"
#include <QPainter>
#include <QPushButton>
#include <QDebug>
#include <QLabel>
#include "progressstore.h"
#include "mypushbutton.h"

ChooseLevelScene::ChooseLevelScene(QWidget *parent) : GamePage(parent)
{

    //返回按钮
    buildBackBtn();

    //每一关对应的按钮
    buildLevelBtn();
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
    initializePage(QRect(30,30,230,44));
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
    auto *back=new MyPushButton(":/others/Image/backBtn.png");
    back->setParent(this); back->setObjectName("backToMenu");
    back->setToolTip("返回主菜单"); back->setAccessibleName("返回主菜单");
    back->move(1650-back->width()-35,25);
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
    for(int level=1;level<=10;++level)
        findChild<QPushButton*>(QString("level%1").arg(level))->setVisible(progress.isUnlocked(level));
    progressLabel->setText(progress.error().isEmpty()
        ? QString("已通过：%1 / 10 关 · 通关后解锁下一关。%2")
            .arg(progress.highestCompleted()).arg(progress.highestCompleted() == 10 ? "  已全部通关！" : "")
        : progress.error());
}
