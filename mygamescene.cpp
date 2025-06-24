//mygamescene.cpp
#include "mygamescene.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include "singingwhite.h"
#include "heartwhite.h"
#include "wallwhite.h"
#include "linewhite.h"
#include "dancingwhite.h"
#include "allheartwhite.h"
#include "dblsingwhite.h"
#include "moneywhite.h"
#include "card.h"
#include <QRandomGenerator>
#include "heart.h"
#include <QApplication>
#include "yellowdogs.h"
#include "myDirection.h"
#include "bullet.h"

MyGameScene::MyGameScene(int n,QMainWindow *parent)
    : QGraphicsScene(parent)
{
    gameLevelNum = n;

    //设置有效操作范围
    setSceneRect(0, 0, 1650, 900);

    //初始化存储僵尸的容器
    zombieMap.resize(5);

    //加载背景
    QPixmap backgroundPixmap(":/others/Image/grass.jpg");
    // qDebug() << backgroundPixmap.height() << backgroundPixmap.width();
    backgroundPixmap = backgroundPixmap.scaled((900.0 / backgroundPixmap.height()) * backgroundPixmap.width(),900);
    //采用backGroundItem管理场景界面
    QGraphicsPixmapItem *backGroundItem = addPixmap(backgroundPixmap);
    backGroundItem->setPos(0, 0);
    backGroundItem->setScale(1);

    //添加网格
    mapGrid = new Map(9, 5, QSize(121,145), QPointF(380,130));
    addItem(mapGrid);
    initMapOccupied(9, 5);

    //添加游戏计时器
    memGameTimer = new QTimer(this);
    memGameTimer->start(100);

    //添加天空中随即爱心生成计时器
    memSkyHeartTimer = new QTimer(this);
    connect(memSkyHeartTimer, &QTimer::timeout, this, &MyGameScene::generateSkyHeart);
    memSkyHeartTimer->start(10000 + QRandomGenerator::global()->bounded(3000));

    switch(gameLevelNum){
        case 1:
            //添加小金毛生成计时器
            memYellowDogsTimer = new QTimer(this);
            connect(memYellowDogsTimer, &QTimer::timeout, this, [=](){setAYellowDog(2);});
            memYellowDogsTimer->start(20000);
            break;
        default:
            //添加小金毛生成计时器
            memYellowDogsTimer = new QTimer(this);
            connect(memYellowDogsTimer, &QTimer::timeout, this, [=](){setAYellowDog(2);});
            memYellowDogsTimer->start(20000);
            break;
    }


}

//初始化地图占用表
bool MyGameScene::initMapOccupied(int cols, int rows){
    if(!mapOccupied){
        mapOccupied = new bool[rows * cols];
        for (int i = 0; i < rows * cols; i++){
            mapOccupied[i] = false;
        }
        return true;
    }
    else return false;
}

MyGameScene::~MyGameScene(){
    if(mapOccupied) {
        delete mapOccupied;
        mapOccupied = NULL;
    }
}

//天空随机生成爱心
void MyGameScene::generateSkyHeart(){
    //随机生成起始位置
    int startX = QRandomGenerator::global()->bounded(800) + 300;
    QPointF startPos(startX, -100);

    //随机生成结束位置
    QPointF endPos(startX, 400 + QRandomGenerator::global()->bounded(300));

    //创建爱心添加到场景
    Heart *heart = new Heart(startPos, endPos, this);
    this->addItem(heart);

    //链接信号和槽
    connect(heart, &Heart::collected,this,[=](){
        addHeart(heart->value());
        emit heartCollected();
    });
}

void MyGameScene::setChosenNum(int cardNum){
    chosenNum = cardNum;
}

void MyGameScene::generateWhiteHeart(QPointF whitePos){
    QPointF startPos = whitePos + QPointF(0, 0);
    QPointF endPos = whitePos + QPointF(QRandomGenerator::global()->bounded(100) - 50, QRandomGenerator::global()->bounded(80));
    Heart *heart = new Heart(startPos,endPos,this,QEasingCurve::OutBounce);
    this->addItem(heart);
    connect(heart, &Heart::collected,this,[=](){
        addHeart(heart->value());
        emit heartCollected();
    });
}

//todo:
void MyGameScene::generateBullet(int r,int c){
    //调试
    // qDebug() << "generated!";
    Bullet *blt = new Bullet(r, c, memGameTimer);
    this->addItem(blt);
}

void MyGameScene::mousePressEvent(QGraphicsSceneMouseEvent * event){


    if (Card::currentState() == GameState::PrePlace){

        //转换到坐标网格系统
        int col, row;
        //转换成坐标网格系统
        if(mapGrid->turnPosToMap(event->scenePos(),col,row)){
            //检查是否被占用
            if(!mapOccupied[row * 9 + col]){
                //计算中心的坐标
                QPointF centerLoc = mapGrid->cellCenter(col,row);
                //创建植物并定位
                WhiteDogs * myDog;
                switch (chosenNum){
                        case 0:
                        myDog = new SingingWhite(row, col, this);
                            break;

                        case 1:
                            myDog = new HeartWhite(this);
                            break;

                        case 2:
                            myDog = new WallWhite;
                            break;

                        case 3:
                            myDog = new LineWhite;
                            break;

                        case 4:
                            myDog = new DancingWhite;
                            break;

                        case 5:
                            myDog = new AllHeartWhite;
                            break;

                        case 6:
                            myDog = new DblSingWhite;
                            break;

                        case 7:
                            myDog = new MoneyWhite;
                            break;
                        }
                myDog->setPos(centerLoc - QPointF(myDog->pixmap().width() / 2.0, myDog->pixmap().height() / 2.0));
                addItem(myDog);

                dogMap[row * 9 + col] = myDog;

                myDog->setItPos(row, col);//设置当前植物的所在行和列

                //信号链接删除小狗
                connect(myDog,&WhiteDogs::pleaseRemoveMe, this, &MyGameScene::removeWhite);

                //爱心减少
                cutHeart(myDog->HeartCost());

                //调试
                // qDebug() << restHeart;

                //标记已经占用
                mapOccupied[row * 9 + col] = true;

                //恢复正常状态
                Card::setGameState(GameState::Normal);

                //用于调试
                // qDebug() << col << " " << row;
                emit plantFinished(); //发送种植完成信号
            }
            else{
                // qDebug() << "已被占用";

            }

            QGraphicsScene::mousePressEvent(event);
        }
    }
    else{
        // qDebug() << "scene clicked";
        Heart::curMousePos = event->scenePos();
        emit sceneClicked();
    }
}

void MyGameScene::setAYellowDog(int r){
    YellowDogs *zombie = new YellowDogs(r, this);
    this->zombieMap[r].append(zombie);
    this->addItem(zombie);
    connect(zombie, &YellowDogs::arrivedYourHome, this, &MyGameScene::gameLose);
    zombie->setPos(QPointF(480 + 9 * 121 - zombie->pixmap().width() / 2, 130 + 145 * (r + 0.5) - zombie->pixmap().height() / 2));
    connect(zombie, &YellowDogs::pleaseRemoveMe, this,[=](YellowDogs *zb){
        this->removeItem(zb);
        zombieMap[r].removeOne(zb);
        zb->deleteLater();
    });
    zombie->startMoving(MyDirection::Left);

    ////debug
    // QTimer *debugTimer = new QTimer(this);
    // debugTimer->start(5000);
    // debugTimer->setSingleShot(true);
    // connect(debugTimer,&QTimer::timeout, this, [=](){
    //     zombie->stopMoving();
    // });
}

void MyGameScene::removeWhite(int r,int c){
    removeItem(dogMap[9 * r + c]);
    delete dogMap[9 * r + c];
    dogMap[9 * r + c] = nullptr;
    mapOccupied[9 * r + c] = false;
}
