#include "playscene.h"
#include "gamecatalog.h"
#include <QPainter>
#include "mypushbutton.h"
#include <QTimer>
#include <QLabel>
#include "card.h"
#include <QVector>
#include "mygamescene.h"
#include <QMouseEvent>

PlayScene::PlayScene(int levelNum) :
    levelIndex(levelNum), //维护传进来的关卡号, 加载地图
    myGameScene(new MyGameScene(levelNum,this)),
    myGraphicsView(new QGraphicsView(myGameScene, this))
{

    //先初始化Card的静态成员
    Card::setCurRestHeart(50);
    Card::setGameState(GameState::Normal);
    Card::setSelectedWhite("");

    //设置标题
    QString titleStr = QString(" 第 %1 关").arg(levelNum);

    //初始化游戏场景
    //设置固定大小
    setFixedSize(1650,900);

    //设置窗口图标
    setWindowIcon(QIcon(":/white/Image/dogIcon.jpg"));

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

    //初始化预加载图片
    preImageLabel = new QLabel(this);
    preImageLabel->setVisible(false);
    preImageLabel->setAttribute(Qt::WA_TransparentForMouseEvents);


    //加载植物
    // 设置主窗口
    setCentralWidget(myGraphicsView);  // 将视图设置为中心部件

    connect(this, &PlayScene::gameLose, this, &PlayScene::gamePaused);
    connect(this, &PlayScene::gameWin, this, &PlayScene::gamePaused);
    connect(this, &PlayScene::gameLose, this, [=](){
        QTimer::singleShot(3000,this,[=](){
            emit this->playSceneBack();
        });
    });
    connect(this, &PlayScene::gameWin, this, [=](){
        QTimer::singleShot(3000,this,[=](){
            emit this->playSceneBack();
        });
    });
    //接收游戏胜利失败暂停信号
    connect(myGameScene, &MyGameScene::gameLose, this, &PlayScene::gameLose);
    connect(myGameScene, &MyGameScene::gameWin, this, &PlayScene::gameWin);
    // connect(myGameScene, &MyGameScene::gamePause, this, &PlayScene::gamePause);



    // 配置视图
    myGraphicsView->setRenderHint(QPainter::Antialiasing);  // 抗锯齿
    myGraphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);  // 对齐方式
    myGraphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate); // 设置更新模式
    myGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    myGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //设置能够显示剩余爱心的图标
    restHeartLabel = new QLabel(this);
    restHeartLabel->setFixedSize(100,100);
    restHeartLabel->setText(QString::number(myGameScene->getRestHeart()));

    //设置字体颜色和大小
    QFont font;
    font.setFamily("华文新魏");
    font.setPointSize(18);
    restHeartLabel->setFont(font);

    //移动
    restHeartLabel->move(360, 91);
    restHeartLabel->setAlignment(Qt::AlignHCenter);

    //使得鼠标能够穿透restHeartLabel
    restHeartLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    //Card类爱心初始化为50
    Card::setCurRestHeart(myGameScene->getRestHeart());

    //种植以后剩余爱心显示减少
    connect(myGameScene,&MyGameScene::plantFinished, this,[=](){
        restHeartLabel->setText(QString::number(myGameScene->getRestHeart()));
        Card::setCurRestHeart(myGameScene->getRestHeart());
        myCards[myGameScene->getChosenNum()]->startCooldown();
        emit signalToCard();
    });

    //收集爱心以后剩余爱心增多
    connect(myGameScene, &MyGameScene::heartCollected, this,[=](){
        restHeartLabel->setText(QString::number(myGameScene->getRestHeart()));
        Card::setCurRestHeart(myGameScene->getRestHeart());
        emit signalToCard();
    });

    connect(myGameScene, &MyGameScene::allowTracking, this, [=](){
        myGraphicsView->setMouseTracking(true);
    });
    connect(myGameScene, &MyGameScene::banTracking, this, [=](){
        myGraphicsView->setMouseTracking(false);
    });

    connect(myGameScene, &MyGameScene::mouseMovedTo, this, [=](QPointF mousePos){
        preImageLabel->move(mousePos.x() - preImageLabel->width() / 2, mousePos.y() - preImageLabel->height() / 2);
        preImageLabel->setVisible(true);
    });
}


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

    connect(myGameScene, &MyGameScene::pleaseRemovePreImage, this, &PlayScene::stopShow);
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

//设置卡槽
void PlayScene::setCardBar(){
    QLabel * cardBarLbl = new QLabel;
    cardBarLbl->setParent(this);
    QPixmap pix;
    pix.load(":/others/Image/cardBar.png");
    pix = pix.scaled(pix.width() * 1.5,pix.height() * 1.5);
    cardBarLbl->setFixedSize(pix.width(),pix.height());
    cardBarLbl->setPixmap(pix);
    cardBarLbl->move(350,0);
}

//设置卡牌在槽中
void PlayScene::setCardsInBar(){
    for(int i = 0; i < (levelIndex < 8 ? levelIndex : 8); i++){
        const auto& plant = GameCatalog::plants().at(i);
        //设置卡牌
        Card *card = new Card(i);
        card->setParent(this);
        card->move(470 + i * (card->width() + 5.5), 10);
        card->whiteType = plant.id;
        card->coolTime = plant.cooldownMs;
        card->heartCost = plant.cost;
        connect(this, &PlayScene::gameLose, card, &Card::gamePaused);
        connect(this, &PlayScene::gameWin, card, &Card::gamePaused);

        //将卡牌指针添加到容器中
        myCards.append(card);

        //设置卡牌图标
        QLabel *whiteIcon = new QLabel;
        whiteIcon->setParent(this);

        QPixmap pix;
        pix.load(plant.image);
        pix = pix.scaled(plant.iconSize, plant.iconSize);
        whiteIcon->setFixedSize(card->width(),card->height());
        whiteIcon->setPixmap(pix);
        whiteIcon->move(470 + i * (card->width() + 5.5) + plant.iconX, plant.iconY);
        //鼠标能够穿透
        whiteIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

        //设置阳光消耗数字显示
        QLabel *label = new QLabel;
        label->setParent(this);
        label->setFixedSize(card->width(),card->height());
        label->setText(QString::number(plant.cost));

        //设置字体颜色和大小
        QFont font;
        font.setFamily("Arial");
        font.setPointSize(10);
        label->setFont(font);

        //移动
        label->move(470 + i * (card->width() + 5.5) - 10, 91);
        label->setAlignment(Qt::AlignHCenter);

        //使得鼠标能够穿透label
        label->setAttribute(Qt::WA_TransparentForMouseEvents);

        //链接卡牌被选择事件和处理卡牌选择事件
        connect(card, &Card::cardSelected, this, &PlayScene::handleCardSelected);
        connect(card, &Card::cardSelected, myGameScene, [=](){
            myGameScene->setChosenNum(i);
            myGraphicsView->setMouseTracking(true);
            startShow(i);
        });

        //除爱心小狗外，其他小狗一开始就进入冷却
        if(i != 1) {
            card->startCooldown();
        }

        //初次检查爱心是否足够
        if(card->heartCost <= myGameScene->getRestHeart()) {
            card->setHeartIsEnough(true);
        }

        //链接将信号传给卡牌
        connect(this, &PlayScene::signalToCard, card, &Card::checkHeartEnough);
    }

}

void PlayScene::handleCardSelected(Card *card){
    // 进入预放置状态
    Card::setGameState(GameState::PrePlace);
    Card::setSelectedWhite(card->whiteType);
}

void PlayScene::startShow(int num){
    QPixmap pix;
    const auto& plant = GameCatalog::plants().at(num);
    pix.load(plant.image);
    pix = pix.scaled(plant.iconSize * 2, plant.iconSize * 2);
    preImageLabel->setFixedSize(plant.iconSize * 2, plant.iconSize * 2);
    preImageLabel->setPixmap(pix);
    // preImageLabel->move(100 - preImageLabel->width() / 2, 100 - preImageLabel->height() / 2);
    // preImageLabel->setVisible(true);
}

void PlayScene::stopShow(){
    preImageLabel->setVisible(false);
    myGraphicsView->setMouseTracking(false);
}

// void PlayScene::mouseMoveEvent(QMouseEvent *event){
//     if(imageFollowing){
//         preImageLabel->move(event->pos().x(), event->pos().y());
//     }
//     QMainWindow::mouseMoveEvent(event);
// }

void PlayScene::gamePaused(){
    Card::setGameState(GameState::Paused);
}

void PlayScene::gameContinued(){
    Card::setGameState(GameState::Normal);
}

void PlayScene::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);
    emit playSceneBack();
}
