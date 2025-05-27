#include "playscene.h"
#include <QPainter>
#include "mypushbutton.h"
#include <QTimer>
#include <QLabel>
#include "card.h"
#include "whitedogs.h"
#include "singingwhite.h"

PlayScene::PlayScene(int levelNum) :
    levelIndex(levelNum), //维护传进来的关卡号, 加载地图
    myGraphicsScene(new QGraphicsScene(this)),
    myGraphicsView(new QGraphicsView(myGraphicsScene, this))
{

    //设置标题
    QString titleStr = QString(" 第 %1 关").arg(levelNum);

    //初始化游戏场景
    //设置固定大小
    setFixedSize(1650,900);

    //设置窗口图标
    setWindowIcon(QIcon(":/Image/dogIcon.jpg"));

    //设置窗口标题
    setWindowTitle("PvZ_Demo" + titleStr);

    //返回按钮
    //之后会替换成暂停按钮
    buildBackBtn();

    //设置关卡数文字
    setLevelText();

    //设置卡槽
    setCardBar();

    //设置卡牌
    setCardsInBar();

    //加载植物
    // 设置主窗口
    setCentralWidget(myGraphicsView);  // 将视图设置为中心部件

    // 配置场景
    myGraphicsScene->setSceneRect(0, 0, 100, 100);  // 设置场景逻辑坐标范围
    QPixmap backgroundPixmap(":/others/Image/grass.jpg");
    //采用backGroundItem
    QGraphicsPixmapItem *backGroundItem = myGraphicsScene->addPixmap(backgroundPixmap);

    backGroundItem->setPos(0, 0);
    backGroundItem->setScale(1.5);


    // 配置视图
    myGraphicsView->setRenderHint(QPainter::Antialiasing);  // 抗锯齿
    myGraphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);  // 对齐方式
    myGraphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate); // 设置更新模式

    // 添加测试植物
    WhiteDogs* singingWhite = new SingingWhite;
    singingWhite->setPos(0 , 0);  // 设置位置
    myGraphicsScene->addItem(singingWhite);  // 将singingWhite添加到场景


}

// void PlayScene::paintEvent(QPaintEvent *)
// {
//     QPainter painter(this);
//     QPixmap pix;

//     //背景图片
//     pix.load(":/others/Image/grass.jpg");

//     painter.drawPixmap(0,0,pix.width() * this->height() / pix.height(),this->height(),pix);

// }

void PlayScene::buildBackBtn(){
    //返回按钮
    MyPushButton *backBtn = new MyPushButton(":/others/Image/backBtn.png");
    backBtn->setParent(this);
    backBtn->move(this->width() - backBtn->width() * 1.2,backBtn->width() * 0.2);

    connect(backBtn,&MyPushButton::clicked,this,[=](){
        //点击动画
        backBtn->zoom1();
        backBtn->zoom2();

        //延时返回
        QTimer::singleShot(300,this,[=](){
            emit this->playSceneBack();
        });
    });
}

void PlayScene::setLevelText(){
    //定义字体
    QFont font;
    font.setFamily("华文新魏");
    font.setBold(true);
    font.setPointSize(20);
    QString levStr = QString("Level: %1").arg(this->levelIndex);

    //显示当前关卡数并设置字体
    QLabel * levNumLbl = new QLabel;
    levNumLbl->setParent(this);
    levNumLbl->setFont(font);
    levNumLbl->setText(levStr);
    levNumLbl->setGeometry(50,this->height() - 80,180,100);
}

//todo: 将卡槽改成Item控件
void PlayScene::setCardBar(){
    //设置卡槽
    QLabel * cardBarLbl = new QLabel;
    cardBarLbl->setParent(this);

    QPixmap pix;
    pix.load(":/others/Image/cardBar.png");
    pix = pix.scaled(pix.width() * 1.5,pix.height() * 1.5);
    cardBarLbl->setFixedSize(pix.width(),pix.height());
    cardBarLbl->setPixmap(pix);
    cardBarLbl->move(350,0);

}

void PlayScene::setCardsInBar(){
    //设置卡牌
    for(int i = 0; i < (levelIndex < 8 ? levelIndex : 8); i++){
        //设置卡牌
        Card *card = new Card(i);
        card->setParent(this);
        card->move(470 + i * (card->width() + 5.5), 10);
        card->whiteType = this->whiteTypes[i];
        card->coolTime = this->coolTimes[i];
        card->heartCost = this->heartCosts[i];

        //设置卡牌图标
        QLabel *whiteIcon = new QLabel;
        whiteIcon->setParent(this);

        QPixmap pix;
        pix.load(whiteImages[i]);
        pix = pix.scaled(sizes[i], sizes[i]);
        whiteIcon->setFixedSize(card->width(),card->height());
        whiteIcon->setPixmap(pix);
        whiteIcon->move(470 + i * (card->width() + 5.5) + Xs[i], Ys[i]);
        //鼠标能够穿透
        whiteIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

    }
}
