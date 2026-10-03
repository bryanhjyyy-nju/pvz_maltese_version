#include "audiomanager.h"
#include "gamecatalog.h"
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
#include "bullet.h"
#include <QPainter>
#include <QKeyEvent>

MyGameScene::MyGameScene(int n,QMainWindow *parent)
    : QGraphicsScene(parent)
{
    gameLevelNum = n;
    // qDebug() << n;
    m_isGameOver = false;
    m_zombiesSpawned = 0;
    m_zombiesKilled = 0;

    const auto level = GameCatalog::level(n);
    m_totalZombiesForLevel = level.enemies;
    QFont font;
    font.setBold(true);
    font.setFamily("黑体");
    font.setPointSize(18);
    QGraphicsTextItem * textZombieUp = this->addText("已击败：",font);
    textZombieUp->setPos(QPointF(1330,10));
    textZombieUp->setZValue(31);
    QGraphicsTextItem * textZombieDown = this->addText(QString("%1/%2").arg(m_zombiesKilled).arg(m_totalZombiesForLevel),font);
    textZombieDown->setPos(QPointF(1360,60));
    textZombieDown->setZValue(31);
    connect(this, &MyGameScene::changeRestZombieNumber, this, [=](){
        // qDebug() << "change";
        textZombieDown->setPlainText(QString("%1/%2").arg(m_zombiesKilled).arg(m_totalZombiesForLevel));
    });

    // 连接游戏胜利和失败的信号到对应的槽函数
    connect(this, &MyGameScene::gameWin, this, &MyGameScene::winTheGame);
    connect(this, &MyGameScene::gameLose, this, &MyGameScene::loseTheGame);

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
    // this->addRect(3,3,200,200,QPen(Qt::black,3), QBrush(Qt::yellow));
    // font.setFamily("华文新魏");
    // font.setPointSize(12);
    // QGraphicsTextItem * textIt = this->addText("您选择的小白:",font);
    // textIt->setScale(1.2);

    QPixmap shovelPix(":/others/Image/shovelBar.png");
    QGraphicsPixmapItem * shovelBar;
    shovelBar = new QGraphicsPixmapItem(shovelPix);
    shovelBar->setPos(QPointF(1200, 0));
    shovelBar->setScale(0.8);
    addItem(shovelBar);
    shovelPix.load(":/others/Image/shovel.png");
    shovel = new QGraphicsPixmapItem(shovelPix);
    shovel->setPos(QPointF(1200,0));
    shovel->setScale(0.72);
    shovel->setZValue(30);
    addItem(shovel);
    // font.setBold(true);
    font.setFamily("Arial");
    font.setPointSize(25);
    QGraphicsTextItem * textShovel = this->addText("R",font);
    textShovel->setPos(QPointF(1230,20));
    textShovel->setZValue(31);


    //添加网格
    mapGrid = new Map(9, 5, QSize(121,145), QPointF(380,130));
    addItem(mapGrid);
    initMapOccupied(9, 5);

    //添加游戏计时器
    memGameTimer = new QTimer(this);
    memGameTimer->start(100);
    memLongGameTimer = new QTimer(this);
    memLongGameTimer->start(500);

    //添加天空中随即爱心生成计时器
    memSkyHeartTimer = new QTimer(this);
    connect(memSkyHeartTimer, &QTimer::timeout, this, &MyGameScene::generateSkyHeart);
    memSkyHeartTimer->start(6000 + QRandomGenerator::global()->bounded(3000));
    connect(memSkyHeartTimer, &QTimer::timeout, memSkyHeartTimer, [=](){
        memSkyHeartTimer->setInterval(6000 + QRandomGenerator::global()->bounded(3000));
    });

    //todo
    //插入小金毛
    memYellowDogsTimer = new QTimer(this);

    connect(memYellowDogsTimer, &QTimer::timeout, this, [=](){
        // 计算本波要生成的僵尸数量
        int spawnCount = level.perWave;
        if (QRandomGenerator::global()->generateDouble() < level.extraChance) {
            spawnCount++;
        }
        if (QRandomGenerator::global()->generateDouble() < level.extraTwoChance) {
            spawnCount += 2;
        }

        //todo : 前期削减难度，后期增强难度

        for (int i = 0; i < spawnCount; ++i) {
            // 1. 在允许的范围内随机选择一行
            // QRandomGenerator::bounded(N) 生成 [0, N-1] 的整数
            int row = QRandomGenerator::global()->bounded(level.maxRow - level.minRow + 1) + level.minRow;

            // 2. 根据概率选择僵尸种类
            int type = 0; // 默认为种类0
            if (QRandomGenerator::global()->generateDouble() < level.toughChance) {
                if(QRandomGenerator::global()->generateDouble() < level.quickChance){
                    type = 2; //种类2(高移速)
                }
                else{
                    type = 1; // 种类1(高血量)
                }
            }

            // 3. 调用函数生成僵尸
            setAYellowDog(row, type);
            // // 判断是否应当给游戏加速
            // if (!beFaster && m_zombiesSpawned >= 21 &&  gameLevelNum == 9){
            //     level.minInterval = 9000;
            //     level.maxInterval = 12000;
            //     beFaster = true;
            // }
        }

        // 4. 为下一波僵尸设置一个新的随机时间间隔
        int nextInterval = QRandomGenerator::global()->bounded(level.minInterval, level.maxInterval + 1);
        memYellowDogsTimer->setInterval(nextInterval);
    });

    // 启动计时器，设置一个初始延迟，避免游戏一开始就出僵尸
    if(gameLevelNum == 10){
        memYellowDogsTimer->start(20000);
    }
    else{
        memYellowDogsTimer->start(15000); // 第一波僵尸在15秒后开始生成
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
        delete[] mapOccupied;
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
    hearts.append(heart);

    //链接信号和槽
    connect(heart, &Heart::collected,this,[=](){
        addHeart(heart->value());
        AudioManager::instance().play("collect");
        emit heartCollected();
        hearts.removeOne(heart);
    });
    connect(heart, &Heart::i_have_disappeared, this,[=](){
        hearts.removeOne(heart);
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
    hearts.append(heart);
    connect(heart, &Heart::collected,this,[=](){
        addHeart(heart->value());
        AudioManager::instance().play("collect");
        emit heartCollected();
        hearts.removeOne(heart);
    });
    connect(heart, &Heart::i_have_disappeared, this,[=](){
        hearts.removeOne(heart);
    });
}

void MyGameScene::generateBullet(int r,int c){
    //调试
    // qDebug() << "generated!";
    AudioManager::instance().play("shoot");
    Bullet *blt = new Bullet(r, c, memGameTimer);
    this->addItem(blt);
    bullets.append(blt);
    connect(blt, &Bullet::hasDisappeared, this,[=](){
        bullets.removeOne(blt);
    });
}

void MyGameScene::mousePressEvent(QGraphicsSceneMouseEvent * event){
    if (Card::currentState() == GameState::PrePlace){
        //转换到坐标网格系统
        int col, row;
        //转换成坐标网格系统
        if(mapGrid->turnPosToMap(event->scenePos(),col,row)){
            //检查是否被占用
            if(!mapOccupied[row * 9 + col] && restHeart >= GameCatalog::plants().at(chosenNum).cost){
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
                        myDog = new LineWhite(row, col, this, centerLoc);
                        break;

                    case 4:
                        myDog = new DancingWhite;
                        break;

                    case 5:
                        myDog = new AllHeartWhite(this);
                        break;

                    case 6:
                        myDog = new DblSingWhite(row, col, this);
                        break;

                    case 7:
                        myDog = new MoneyWhite;
                        break;

                    default:
                        myDog = new WallWhite;
                        break;
                }
                emit pleaseRemovePreImage();

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
                AudioManager::instance().play("plant");
                emit plantFinished(); //发送种植完成信号

                if(chosenNum == 3){ myDog->startMoving(); }
            }
            // else{
            //     // qDebug() << "已被占用";

            // }

            QGraphicsScene::mousePressEvent(event);
        }
    }
    else if(Card::currentState() == GameState::Normal){
        // qDebug() << "scene clicked";
        Heart::curMousePos = event->scenePos();
        emit sceneClicked();
        QGraphicsScene::mousePressEvent(event);
    }
    else if(Card::currentState() == GameState::Shoveling){
        int col, row;
        if(mapGrid->turnPosToMap(event->scenePos(),col,row)){
            if(mapOccupied[row * 9 + col]){
                AudioManager::instance().play("shovel");
                dogMap[row * 9 + col]->removeItself();
                emit banTracking();
                Card::setGameState(GameState::Normal);
                shovel->setPos(1200,0);
            }
        }
        QGraphicsScene::mousePressEvent(event);
    }
}

void MyGameScene::mouseMoveEvent(QGraphicsSceneMouseEvent * event){
    if(Card::currentState() == GameState::Shoveling){
        shovel->setPos(event->scenePos() - QPointF(shovel->pixmap().width() / 2.0, shovel->pixmap().height() / 2.0));
    }
    else if(Card::currentState() == GameState::PrePlace){
        emit mouseMovedTo(event->scenePos());
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void MyGameScene::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Escape && (Card::currentState() == GameState::PrePlace
                                          || Card::currentState() == GameState::Shoveling)) {
        Card::setGameState(GameState::Normal);
        emit pleaseRemovePreImage();
        emit banTracking();
        shovel->setPos(1200,0);
        return;
    }
    if(event->key() == Qt::Key_R && Card::currentState() == GameState::Normal){
        emit allowTracking();
        Card::setGameState(GameState::Shoveling);
    }
    else if(event->key() == Qt::Key_R && Card::currentState() == GameState::Shoveling){
        emit banTracking();
        Card::setGameState(GameState::Normal);
        shovel->setPos(1200,0);
    }
    QGraphicsScene::keyPressEvent(event);
}

void MyGameScene::setAYellowDog(int r,int typeNum){
    // 如果已生成的僵尸达到本关总数，则停止生成并返回
    if(m_zombiesSpawned >= m_totalZombiesForLevel){
        memYellowDogsTimer->stop();
        return;
    }
    m_zombiesSpawned++;
    //调试代码
    // qDebug() << m_zombiesSpawned;

    YellowDogs *zombie = new YellowDogs(r,this,typeNum);
    this->zombieMap[r].append(zombie);
    this->addItem(zombie);
    connect(zombie, &YellowDogs::arrivedYourHome, this, &MyGameScene::gameLose);
    connect(zombie, &YellowDogs::pleaseRemoveMe, this,[=](YellowDogs *zb){
        this->removeItem(zb);
        zombieMap[r].removeOne(zb);
        zb->deleteLater();

        // 僵尸被消灭，更新计数并检查胜利条件
        if (!m_isGameOver) {
            m_zombiesKilled++;
            //调试
            // qDebug() << m_zombiesKilled;
            emit changeRestZombieNumber();
            checkWinCondition();
        }
    });
    zombie->startMoving();

    ////debug
    // QTimer *debugTimer = new QTimer(this);
    // debugTimer->start(5000);
    // debugTimer->setSingleShot(true);
    // connect(debugTimer,&QTimer::timeout, this, [=](){
    //     zombie->stopMoving();
    // });
}

void MyGameScene::removeWhite(int r,int c){
    if (!dogMap[9 * r + c]) return;
    removeItem(dogMap[9 * r + c]);
    dogMap[9 * r + c]->deleteLater();
    dogMap[9 * r + c] = nullptr;
    mapOccupied[9 * r + c] = false;
}

void MyGameScene::checkWinCondition()
{
    // 如果游戏已结束，或生成的僵尸还未达到关卡总数，则不进行判断
    if (m_isGameOver || m_zombiesSpawned < m_totalZombiesForLevel) {
        return;
    }

    // 如果所有生成的僵尸都已被消灭，则胜利
    if (m_zombiesKilled >= m_totalZombiesForLevel) {
        emit gameWin();
    }
}

void MyGameScene::stopAllTimers()
{
    memGameTimer->stop();
    memSkyHeartTimer->stop();
    memYellowDogsTimer->stop();
    memLongGameTimer->stop();
}

void MyGameScene::winTheGame()
{
    if (m_isGameOver) return; // 防止重复执行
    m_isGameOver = true;

    stopAllTimers(); // 停止所有游戏活动
    for(int i = 0; i < 45; i++){
        if(dogMap[i] != nullptr){
            dogMap[i]->gamePaused();
        }
    }
    for(QGraphicsPixmapItem *item : bullets){
        ((Bullet *)item)->gamePaused();
    }
    for(QGraphicsPixmapItem *item : hearts){
        ((Heart *)item)->gamePaused();
    }

    // 创建 "WIN" 文本
    QGraphicsSimpleTextItem *winText = new QGraphicsSimpleTextItem("WIN");
    QFont font("Arial", 150, QFont::Bold);
    winText->setFont(font);
    winText->setBrush(QBrush(Qt::green));

    // 将文本居中
    QPointF center = sceneRect().center();
    QRectF textRect = winText->boundingRect();
    winText->setPos(center.x() - textRect.width() / 2, center.y() - textRect.height() / 2);

    // 确保文本在最上层显示
    winText->setZValue(32);

    addItem(winText);
}

void MyGameScene::loseTheGame()
{
    if (m_isGameOver) return; // 防止重复执行
    m_isGameOver = true;

    stopAllTimers(); // 停止所有游戏活动
    for(int i = 0; i < 5;i++){
        for(QGraphicsPixmapItem *item : zombieMap[i]){
            ((YellowDogs *)item)->gamePaused();
        }
    }
    for(int i = 0; i < 45; i++){
        if(dogMap[i] != nullptr){
            dogMap[i]->gamePaused();
        }
    }
    for(QGraphicsPixmapItem *item : bullets){
        ((Bullet *)item)->gamePaused();
    }
    for(QGraphicsPixmapItem *item : hearts){
        ((Heart *)item)->gamePaused();
    }

    // 创建 "LOSE" 文本
    QGraphicsSimpleTextItem *loseText = new QGraphicsSimpleTextItem("LOSE");
    QFont font("Arial", 150, QFont::Bold);
    loseText->setFont(font);
    loseText->setBrush(QBrush(Qt::red));

    // 将文本居中
    QPointF center = sceneRect().center();
    QRectF textRect = loseText->boundingRect();
    loseText->setPos(center.x() - textRect.width() / 2, center.y() - textRect.height() / 2);

    // 确保文本在最上层显示
    loseText->setZValue(32);

    addItem(loseText);
}
