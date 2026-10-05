#include "leveltutorial.h"
#include "mygamescene.h"
#include "gameui.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

LevelTutorial::LevelTutorial(int number,MyGameScene *board,QWidget *parent)
    : QFrame(parent),level(number),scene(board) {
    setObjectName("levelTutorial"); setProperty("manualScale",true);
    auto *layout=new QVBoxLayout(this);
    title=new QLabel(this); instructions=new QLabel(this); instructions->setWordWrap(true);
    next=new QPushButton("完成教程，开始防守！",this); next->setObjectName("finishTutorial");
    GameUi::styleButton(next); layout->addWidget(title); layout->addWidget(instructions); layout->addWidget(next);
    connect(next,&QPushButton::clicked,this,[this] { if(usedPause && viewedAlmanac) enter(Step::Done); });
    connect(scene,&MyGameScene::plantFinished,this,[this] {
        if(current==Step::Plant) { enter(Step::Heart); scene->generateTutorialHeart(); }
    });
    connect(scene,&MyGameScene::heartCollected,this,[this] { if(current==Step::Heart) enter(Step::Controls); });
    connect(scene,&MyGameScene::plantRemoved,this,[this](int,int) {
        if(current!=Step::Shovel) return;
        ++removed;
        if(removed==3) enter(Step::Done);
        else instructions->setText(QString("点击上方铲子或按 R 拿起，左键点击示范小白即可铲除；右键取消。\n已铲除 %1 / 3 株。腾出的格子可以重新种植；铲除不会返还爱心。").arg(removed));
    });
    hide();
}
void LevelTutorial::start() {
    if(level==1) { scene->addHeart(qMax(0,100-scene->getRestHeart())); enter(Step::Plant); }
    else {
        for(int row=1;row<=3;++row) scene->addTutorialPlant(row,2);
        enter(Step::Shovel);
    }
}
void LevelTutorial::enter(Step step) {
    current=step;
    if(step==Step::Done) { hide(); emit finished(); return; }
    next->setVisible(step==Step::Controls); next->setEnabled(false);
    if(step==Step::Plant) {
        title->setText("教程 1 / 3 · 种下你的第一株歌唱小白");
        instructions->setText("左键选择上方的歌唱小白卡片，再点中间草坪的空格子。已为练习准备 100 爱心。\n歌唱小白花费 100 爱心，同排前方有金毛时，每 1.6 秒发射音符，造成 30 点伤害。");
    } else if(step==Step::Heart) {
        title->setText("教程 2 / 3 · 点击草坪上的爱心");
        instructions->setText("爱心是种植小白的资源。左键点击这颗示范爱心，即可增加 25 爱心。\n开战后天空会落下爱心；第二关解锁的爱心小白也会定时产出爱心。卡片下方显示种植花费。");
    } else if(step==Step::Controls) {
        title->setText("教程 3 / 3 · 试试暂停与图鉴"); updateControlsLesson();
    } else {
        title->setText("铲子练习 · 清理这三株歌唱小白");
        instructions->setText("点击上方铲子或按 R 拿起，左键点击示范小白即可铲除；右键取消。\n已铲除 0 / 3 株。腾出的格子可以重新种植；铲除不会返还爱心。");
    }
    show(); raise(); emit stepChanged(step);
}
void LevelTutorial::updateControlsLesson() {
    instructions->setText(QString("按空格或点击左侧暂停按钮，暂停所有战斗活动；按空格或点“继续游戏”恢复。\n点击左侧或暂停菜单中的图鉴，查看小白与金毛的生命、花费、攻击和技能，帮助安排防守。\n暂停：%1    图鉴：%2    两项都体验后，点击下方按钮开始。")
        .arg(usedPause ? "已体验 ✓" : "待体验").arg(viewedAlmanac ? "已查看 ✓" : "待查看"));
    next->setEnabled(usedPause && viewedAlmanac);
}
void LevelTutorial::notePauseUsed() { if(current==Step::Controls) { usedPause=true; updateControlsLesson(); } }
void LevelTutorial::noteAlmanacViewed() { if(current==Step::Controls) { viewedAlmanac=true; updateControlsLesson(); } }
void LevelTutorial::fitCanvas(qreal scale,const QPointF& offset) {
    const int height=current==Step::Controls ? 255 : 150;
    setGeometry(QRect((offset+QPointF(390,150)*scale).toPoint(),QSize(qRound(1120*scale),qRound(height*scale))));
    layout()->setContentsMargins(qRound(18*scale),qRound(12*scale),qRound(18*scale),qRound(12*scale));
    layout()->setSpacing(qRound(8*scale));
    setStyleSheet(QString("QFrame#levelTutorial{background:#fff0c8; border:%1px solid #65452d; border-radius:%2px;}").arg(qMax(1,qRound(4*scale))).arg(qRound(20*scale)));
    title->setStyleSheet(QString("color:#805024; font:bold %1px '%2';").arg(qRound(25*scale)).arg(GameUi::fontFamily()));
    instructions->setStyleSheet(QString("color:#563a26; font:%1px '%2';").arg(qRound(20*scale)).arg(GameUi::fontFamily()));
    next->setStyleSheet(GameUi::styleSheet()+QString("QPushButton{font-size:%1px;}").arg(qRound(18*scale)));
}
