#include "audiomanager.h"
#include "gamecatalog.h"
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
#include "combateffect.h"
#include "bullet.h"
#include <QPainter>
#include <QKeyEvent>

MyGameScene::MyGameScene(int n,QMainWindow *parent)
    : QGraphicsScene(parent)
{
    gameLevelNum = qBound(1,n,GameCatalog::LevelCount);
    m_isGameOver = false;
    m_zombiesSpawned = 0;
    m_zombiesKilled = 0;

    const auto level = GameCatalog::level(gameLevelNum);
    m_totalZombiesForLevel = level.enemies;
    setupBoard();
    setupTimers();
    connect(this, &MyGameScene::gameWin, this, &MyGameScene::winTheGame);
    connect(this, &MyGameScene::gameLose, this, &MyGameScene::loseTheGame);
}

void MyGameScene::setupBoard() {
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
        textZombieDown->setPlainText(QString("%1/%2").arg(m_zombiesKilled).arg(m_totalZombiesForLevel));
    });

    setSceneRect(0, 0, 1650, 900);

    zombieMap.resize(5);

    QPixmap backgroundPixmap(":/others/Image/grass.jpg");
    backgroundPixmap = backgroundPixmap.scaled((900.0 / backgroundPixmap.height()) * backgroundPixmap.width(),900);
    QGraphicsPixmapItem *backGroundItem = addPixmap(backgroundPixmap);
    backGroundItem->setPos(0, 0);
    backGroundItem->setScale(1);

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
    font.setFamily("Arial");
    font.setPointSize(25);
    QGraphicsTextItem * textShovel = this->addText("R",font);
    textShovel->setPos(QPointF(1230,20));
    textShovel->setZValue(31);

    mapGrid = new Map(9, 5, QSize(121,145), QPointF(380,130));
    addItem(mapGrid);

}

void MyGameScene::setupTimers() {
    memGameTimer = new QTimer(this);
    memGameTimer->start(100);
    memLongGameTimer = new QTimer(this);
    memLongGameTimer->start(500);

    memSkyHeartTimer = new QTimer(this);
    connect(memSkyHeartTimer, &QTimer::timeout, this, &MyGameScene::generateSkyHeart);
    connect(memSkyHeartTimer,&QTimer::timeout,this,[this] {
        generateSkyHeart();
        memSkyHeartTimer->setInterval(6000+QRandomGenerator::global()->bounded(3000));
    });
    memSkyHeartTimer->start(6000+QRandomGenerator::global()->bounded(3000));

    memYellowDogsTimer = new QTimer(this);

    connect(memYellowDogsTimer, &QTimer::timeout, this, &MyGameScene::spawnWave);

    if(gameLevelNum == 10){
        memYellowDogsTimer->start(20000);
    }
    else{
        memYellowDogsTimer->start(15000); // 第一波僵尸在15秒后开始生成
    }
}

void MyGameScene::spawnWave() {
    const auto& level = GameCatalog::level(gameLevelNum);
    int spawnCount = level.perWave;
    if (QRandomGenerator::global()->generateDouble() < level.extraChance) {
        spawnCount++;
    }
    if (QRandomGenerator::global()->generateDouble() < level.extraTwoChance) {
        spawnCount += 2;
    }

    for (int i = 0; i < spawnCount; ++i) {
        int row = QRandomGenerator::global()->bounded(level.maxRow - level.minRow + 1) + level.minRow;

        int type = 0; // 默认为种类0
        if (QRandomGenerator::global()->generateDouble() < level.toughChance) {
            if(QRandomGenerator::global()->generateDouble() < level.quickChance){
                type = 2; //种类2(高移速)
            }
            else{
                type = 1; // 种类1(高血量)
            }
        }

        setAYellowDog(row, type);
    }

    int nextInterval = QRandomGenerator::global()->bounded(level.minInterval, level.maxInterval + 1);
    memYellowDogsTimer->setInterval(nextInterval);
}

void MyGameScene::generateSkyHeart(){
    int startX = QRandomGenerator::global()->bounded(800) + 300;
    QPointF startPos(startX, -100);

    QPointF endPos(startX, 400 + QRandomGenerator::global()->bounded(300));

    addHeartItem(new Heart(startPos, endPos, this));
}

void MyGameScene::setChosenNum(int cardNum){
    chosenNum = qBound(0,cardNum,static_cast<int>(GameCatalog::plants().size())-1);
}

void MyGameScene::generateWhiteHeart(QPointF whitePos){
    QPointF startPos = whitePos + QPointF(0, 0);
    QPointF endPos = whitePos + QPointF(QRandomGenerator::global()->bounded(100) - 50, QRandomGenerator::global()->bounded(80));
    addHeartItem(new Heart(startPos,endPos,this,QEasingCurve::OutBounce));
}

void MyGameScene::generateBullet(int r,int c){
    AudioManager::instance().play("shoot");
    Bullet *blt = new Bullet(r, c, memGameTimer);
    blt->setParent(this);
    this->addItem(blt);

}

void MyGameScene::mousePressEvent(QGraphicsSceneMouseEvent * event){
    if(event->button() == Qt::RightButton) {
        if(Card::currentState() == GameState::PrePlace || Card::currentState() == GameState::Shoveling)
            cancelSelection();
        return;
    }
    if(event->button() != Qt::LeftButton) return;
    if(Card::currentState() != GameState::Paused && Card::currentState() != GameState::GameOver
        && QRectF(1200,0,100,105).contains(event->scenePos())) {
        toggleShovel();
        return;
    }
    if (Card::currentState() == GameState::PrePlace){
        int col, row;
        if(mapGrid->turnPosToMap(event->scenePos(),col,row)){
            if(!dogMap[row * 9 + col] && restHeart >= GameCatalog::plants().at(chosenNum).cost){
                placePlant(row,col);
            }

            QGraphicsScene::mousePressEvent(event);
        }
    }
    else if(Card::currentState() == GameState::Normal){
        Heart::curMousePos = event->scenePos();
        emit sceneClicked();
        QGraphicsScene::mousePressEvent(event);
    }
    else if(Card::currentState() == GameState::Shoveling){
        int col, row;
        if(mapGrid->turnPosToMap(event->scenePos(),col,row)){
            if(dogMap[row * 9 + col]){
                AudioManager::instance().play("uproot");
                new CombatEffect(this,dogMap[row * 9 + col]->sceneBoundingRect().center(),CombatEffect::Uproot);
                dogMap[row * 9 + col]->removeItself();
                cancelSelection();
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
        cancelSelection();
        return;
    }
    if(event->key() == Qt::Key_R){
        toggleShovel();
        return;
    }
    QGraphicsScene::keyPressEvent(event);
}

void MyGameScene::setAYellowDog(int r,int typeNum){
    if(m_isGameOver || r < 0 || r >= 5 || m_zombiesSpawned >= m_totalZombiesForLevel){
        memYellowDogsTimer->stop();
        return;
    }
    m_zombiesSpawned++;

    YellowDogs *zombie = new YellowDogs(r,this,typeNum);
    zombie->setParent(this);
    this->zombieMap[r].append(zombie);
    this->addItem(zombie);
    connect(zombie, &YellowDogs::arrivedYourHome, this, &MyGameScene::gameLose);
    connect(zombie,&YellowDogs::dying,this,[this,r](YellowDogs *zb) {
        zombieMap[r].removeOne(zb);
    });
    connect(zombie, &YellowDogs::pleaseRemoveMe, this,[=](YellowDogs *zb){
        this->removeItem(zb);
        zombieMap[r].removeOne(zb);
        zb->deleteLater();

        if (!m_isGameOver) {
            m_zombiesKilled++;
            emit changeRestZombieNumber();
            checkWinCondition();
        }
    });
    zombie->startMoving();

}

void MyGameScene::removeWhite(int r,int c){
    if (!dogMap[9 * r + c]) return;
    removeItem(dogMap[9 * r + c]);
    dogMap[9 * r + c]->deleteLater();
    dogMap[9 * r + c] = nullptr;
}

void MyGameScene::checkWinCondition()
{
    if (m_isGameOver || m_zombiesSpawned < m_totalZombiesForLevel) {
        return;
    }

    if (m_zombiesKilled >= m_totalZombiesForLevel) {
        emit gameWin();
    }
}

void MyGameScene::finishGame(bool won) {
    if(m_isGameOver) return;
    m_isGameOver = true;
    terminalActivity.pause(this);
    auto *text = addSimpleText(won ? "WIN" : "LOSE",QFont("Arial",150,QFont::Bold));
    text->setBrush(won ? Qt::green : Qt::red);
    const auto rect = text->boundingRect();
    text->setPos(sceneRect().center()-QPointF(rect.width()/2,rect.height()/2));
    text->setZValue(32);
}

void MyGameScene::winTheGame() { finishGame(true); }
void MyGameScene::loseTheGame() { finishGame(false); }

void MyGameScene::cancelSelection() {
    if(Card::currentState() == GameState::Shoveling)
        AudioManager::instance().play("shovelPutdown");
    Card::setGameState(GameState::Normal);
    emit pleaseRemovePreImage();
    emit banTracking();
    shovel->setPos(1200,0);
}

void MyGameScene::toggleShovel() {
    const auto state = Card::currentState();
    if(state == GameState::Paused || state == GameState::GameOver || m_isGameOver) return;
    if(state == GameState::Shoveling) { cancelSelection(); return; }
    cancelSelection();
    Card::setGameState(GameState::Shoveling);
    AudioManager::instance().play("shovelPickup");
    emit allowTracking();
}

void MyGameScene::addHeartItem(Heart *heart) {
    heart->setParent(this);
    addItem(heart);
    connect(heart,&Heart::collected,this,[this,heart] {
        addHeart(heart->value());
        AudioManager::instance().play("collect");
        emit heartCollected();
    });
}

WhiteDogs *MyGameScene::createPlant(int row, int col, const QPointF& center) {
    switch(chosenNum) {
    case 0: return new SingingWhite(row,col,this);
    case 1: return new HeartWhite(this);
    case 2: return new WallWhite;
    case 3: return new LineWhite(row,col,this,center);
    case 4: return new DancingWhite;
    case 5: return new AllHeartWhite(this);
    case 6: return new DblSingWhite(row,col,this);
    case 7: return new MoneyWhite;
    default: return nullptr;
    }
}

void MyGameScene::placePlant(int row, int col) {
    QPointF centerLoc = mapGrid->cellCenter(col,row);
    WhiteDogs *myDog = createPlant(row,col,centerLoc);
    if(!myDog) return;
    emit pleaseRemovePreImage();

    myDog->setPos(centerLoc - QPointF(myDog->pixmap().width() / 2.0, myDog->pixmap().height() / 2.0));
    myDog->setParent(this);
    addItem(myDog);

    dogMap[row * 9 + col] = myDog;

    myDog->setItPos(row, col);//设置当前植物的所在行和列

    connect(myDog,&WhiteDogs::pleaseRemoveMe, this, &MyGameScene::removeWhite);

    cutHeart(myDog->HeartCost());

    Card::setGameState(GameState::Normal);

    AudioManager::instance().play("plant");
    new CombatEffect(this,centerLoc,CombatEffect::Plant);
    emit plantFinished(); //发送种植完成信号

    if(chosenNum == 3){ myDog->startMoving(); }
}

WhiteDogs *MyGameScene::plantAhead(int row, qreal x) const {
    if(row<0 || row>=5) return nullptr;
    WhiteDogs *nearest=nullptr;
    for(int col=0;col<9;++col) {
        auto *plant=dogMap[row*9+col];
        if(plant && plant->getHp()>0 && plant->x()<x && (!nearest || plant->x()>nearest->x())) nearest=plant;
    }
    return nearest;
}
