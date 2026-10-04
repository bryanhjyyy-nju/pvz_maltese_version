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
#include "gameartwork.h"
#include "bullet.h"
#include <QPainter>
#include <QKeyEvent>
#include "lawn.h"

MyGameScene::MyGameScene(int n,QObject *parent,bool deferStart,bool endless,int firstWave)
    : QGraphicsScene(parent),endlessMode(endless)
{
    speedClock=new GameSpeed(this);
    gameLevelNum = qBound(1,n,GameCatalog::LevelCount);
    m_isGameOver = false;
    m_zombiesSpawned = 0;
    m_zombiesKilled = 0;

    const auto level = GameCatalog::level(gameLevelNum);
    if(endlessMode) nextWave=qMax(1,firstWave)-1;
    else wavePlan = WavePlanner::create(gameLevelNum,*QRandomGenerator::global());
    m_totalZombiesForLevel = WavePlanner::enemyCount(wavePlan);
    restHeart = level.startingHearts;
    setupBoard();
    setupTimers();
    if(deferStart) inputMode=InputMode::Blocked;
    else startGameplay();
    connect(this, &MyGameScene::gameWin, this, &MyGameScene::winTheGame);
    connect(this, &MyGameScene::gameLose, this, &MyGameScene::loseTheGame);
}

void MyGameScene::setupBoard() {
    setSceneRect(0, 0, 1650, 900);
    // Most items move or change frames; rebuilding a spatial index costs more
    // than scanning this small board. Combat already searches within one lane.
    setItemIndexMethod(QGraphicsScene::NoIndex);

    zombieMap.resize(5);

    background.load(":/others/Image/grass.jpg");
    background=background.scaledToHeight(900);
    grass=new Lawn(gameLevelNum);
    grass->setParent(this);
    addItem(grass);

    if(gameLevelNum>=2) {
    auto *shovelBar = new QGraphicsPixmapItem(GameArtwork::shovelSlot());
    shovelBar->setPos(GameArtwork::shovelSlotRect().topLeft());
    shovelBar->setZValue(29);
    shovelBar->setToolTip("点击拿起 / 放下铲子，也可以按 R");
    addItem(shovelBar);
    shovel = new QGraphicsPixmapItem(GameArtwork::cuteShovel());
    if(shovel) shovel->setPos(GameArtwork::shovelHome());
    shovel->setZValue(30);
    shovel->setToolTip("可爱铲子 · 点击拿起 / 放下");
    addItem(shovel);

    }

    mapGrid = new Map(9, 5, QSize(121,145), QPointF(380,130));
    const auto& level=GameCatalog::level(gameLevelNum);
    mapGrid->setPlantableRows(level.minRow,level.maxRow);
    addItem(mapGrid);

}

void MyGameScene::drawBackground(QPainter *painter,const QRectF& rect) {
    QGraphicsScene::drawBackground(painter,rect);
    painter->drawPixmap(0,0,background);
}

void MyGameScene::setupTimers() {
    memGameTimer = new GameTimer(this,gameSpeed());
    memGameTimer->setInterval(100);
    memLongGameTimer = new GameTimer(this,gameSpeed());
    memLongGameTimer->setInterval(GameCatalog::EnemyBiteIntervalMs);

    memSkyHeartTimer = new GameTimer(this,gameSpeed());
    memSkyHeartTimer->setObjectName("skyHeartTimer");
    connect(memSkyHeartTimer,&QTimer::timeout,this,[this] {
        generateSkyHeart();
        memSkyHeartTimer->setInterval(QRandomGenerator::global()->bounded(GameCatalog::SkyHeartMinIntervalMs,GameCatalog::SkyHeartMaxIntervalMs+1));
    });
    memSkyHeartTimer->setInterval(QRandomGenerator::global()->bounded(GameCatalog::SkyHeartMinIntervalMs,GameCatalog::SkyHeartMaxIntervalMs+1));

    memYellowDogsTimer = new GameTimer(this,gameSpeed());
    memYellowDogsTimer->setTimerType(Qt::PreciseTimer); // Preserve long wave gaps accurately across pause.
    memYellowDogsTimer->setObjectName("waveTimer");

    connect(memYellowDogsTimer, &QTimer::timeout, this, &MyGameScene::spawnWave);

    waveStaggerTimer = new GameTimer(this,gameSpeed());
    waveStaggerTimer->setObjectName("waveStaggerTimer");
    connect(waveStaggerTimer,&QTimer::timeout,this,[this] {
        waveStaggerTimer->setInterval(900);
        spawnNextInWave();
    });
    memYellowDogsTimer->setInterval(GameCatalog::level(gameLevelNum).initialDelayMs);
}

void MyGameScene::startGameplay() {
    if(started || m_isGameOver) return;
    started=true; inputMode=InputMode::Normal;
    for(auto *timer : {memGameTimer,memLongGameTimer,memSkyHeartTimer,memYellowDogsTimer}) timer->start();
}

void MyGameScene::spawnWave() {
    if(!started) return;
    if(m_isGameOver || (!endlessMode && nextWave >= wavePlan.size())) { memYellowDogsTimer->stop(); return; }
    if(endlessMode) {
        pendingWave=WavePlanner::endlessWave(++nextWave,*QRandomGenerator::global());
        memYellowDogsTimer->stop();
    } else pendingWave = wavePlan[nextWave++];
    pendingIndex = 0;
    waveRowCounts.fill(0);
    emit waveStarted(nextWave,endlessMode ? 0 : wavePlan.size());
    const bool bigWave=endlessMode && WavePlanner::isEndlessBigWave(nextWave);
    if(bigWave || (!endlessMode && nextWave==wavePlan.size())) {
        if(bigWave) emit bigWaveApproaching();
        else emit finalWaveApproaching();
        waveStaggerTimer->start(1800);
    } else {
        spawnNextInWave();
        if(pendingIndex < pendingWave.size()) waveStaggerTimer->start(900);
    }
    if(endlessMode) return;
    if(nextWave == wavePlan.size()) memYellowDogsTimer->stop();
    else {
        const auto gap=WavePlanner::intervalBeforeWave(gameLevelNum,nextWave+1);
        memYellowDogsTimer->setInterval(QRandomGenerator::global()->bounded(gap.first,gap.second+1));
    }
}

void MyGameScene::spawnNextInWave() {
    if(m_isGameOver || pendingIndex >= pendingWave.size()) { waveStaggerTimer->stop(); return; }
    const auto& level = GameCatalog::level(gameLevelNum);
    // Spread a wave across the least occupied lanes instead of stacking strong enemies.
    QVector<int> rows;
    int smallest = 100000;
    for(int row=level.minRow;row<=level.maxRow;++row) {
        int occupancy=waveRowCounts[row]*3;
        for(auto *item : zombieMap[row])
            occupancy+=GameCatalog::enemies()[static_cast<YellowDogs*>(item)->typeIndex()].weight;
        if(occupancy<smallest) { smallest=occupancy; rows.clear(); }
        if(occupancy==smallest) rows.append(row);
    }
    int row=rows[QRandomGenerator::global()->bounded(rows.size())];
    ++waveRowCounts[row];
    setAYellowDog(row,pendingWave[pendingIndex++]);
    if(pendingIndex >= pendingWave.size()) waveStaggerTimer->stop();
}

void MyGameScene::generateSkyHeart(){
    int startX = QRandomGenerator::global()->bounded(800) + 300;
    QPointF startPos(startX, GameCatalog::SkyHeartStartY);

    QPointF endPos(startX, 400 + QRandomGenerator::global()->bounded(300));

    addHeartItem(new Heart(startPos,endPos,this,QEasingCurve::Linear,nullptr,false,GameCatalog::SkyHeartFallDurationMs));
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
    Bullet *blt = new Bullet(r, c, memGameTimer);
    blt->setParent(this);
    this->addItem(blt);

}

void MyGameScene::mousePressEvent(QGraphicsSceneMouseEvent * event){
    if(inputMode==InputMode::Blocked || Card::currentState()==GameState::Paused || Card::currentState()==GameState::GameOver) return;
    if(event->button() == Qt::RightButton) {
        if(Card::currentState() == GameState::PrePlace || Card::currentState() == GameState::Shoveling)
            cancelSelection();
        return;
    }
    if(event->button() != Qt::LeftButton) return;
    if(Card::currentState() != GameState::Paused && Card::currentState() != GameState::GameOver
        && GameArtwork::shovelSlotRect().contains(event->scenePos())) {
        toggleShovel();
        return;
    }
    if (Card::currentState() == GameState::PrePlace && (inputMode==InputMode::Normal || inputMode==InputMode::PlantPractice)){
        int col, row;
        if(mapGrid->turnPosToMap(event->scenePos(),col,row)){
            if(!dogMap[row * 9 + col] && restHeart >= GameCatalog::plants().at(chosenNum).cost){
                placePlant(row,col);
            }

            QGraphicsScene::mousePressEvent(event);
        }
    }
    else if(Card::currentState() == GameState::Normal && (inputMode==InputMode::Normal || inputMode==InputMode::HeartPractice)){
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
    if(event->key() == Qt::Key_R){
        toggleShovel();
        return;
    }
    QGraphicsScene::keyPressEvent(event);
}

void MyGameScene::setAYellowDog(int r,int typeNum){
    if(!started) return;
    if(m_isGameOver || r < 0 || r >= 5 || (!endlessMode && m_zombiesSpawned >= m_totalZombiesForLevel)){
        memYellowDogsTimer->stop();
        return;
    }
    m_zombiesSpawned++;

    YellowDogs *zombie = new YellowDogs(r,this,typeNum,endlessMode ? nextWave : 1);
    zombie->setParent(this);
    this->zombieMap[r].append(zombie);
    this->addItem(zombie);
    zombie->setHealthVisible(showEnemyHealth);
    connect(zombie, &YellowDogs::arrivedYourHome, this, [this,zombie] {
        if(m_isGameOver) return;
        losingPosition=zombie->sceneBoundingRect().center();
        emit gameLose();
    });
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
    if(r<0 || r>=5 || c<0 || c>=9) return;
    removePlant(dogMap[9*r+c]);
}

void MyGameScene::removePlant(WhiteDogs *plant) {
    if(!plant || plant->scene()!=this) return;
    const int row=plant->getItRow(),col=plant->getItCol();
    // A moving charger may share its original cell with a newer stationary dog.
    if(dogMap[row*9+col]==plant) {
        dogMap[row*9+col]=nullptr;
        emit plantRemoved(row,col);
    }
    plantRows[row].removeOne(plant);
    removeItem(plant);
    plant->deleteLater();
}

void MyGameScene::checkWinCondition()
{
    if(endlessMode) {
        if(!m_isGameOver && pendingIndex>=pendingWave.size() && m_zombiesSpawned>0 && m_zombiesKilled==m_zombiesSpawned && !memYellowDogsTimer->isActive())
            memYellowDogsTimer->start(6000);
        return;
    }
    if (m_isGameOver || m_zombiesSpawned < m_totalZombiesForLevel) {
        return;
    }

    if (m_zombiesKilled >= m_totalZombiesForLevel) {
        emit gameWin();
    }
}

void MyGameScene::finishGame(bool) {
    if(m_isGameOver) return;
    m_isGameOver = true;
    terminalActivity.pause(this);
}

void MyGameScene::winTheGame() { finishGame(true); }
void MyGameScene::loseTheGame() { finishGame(false); }

void MyGameScene::cancelSelection() {
    if(Card::currentState() == GameState::Shoveling)
        AudioManager::instance().play("shovelPutdown");
    Card::setGameState(GameState::Normal);
    emit pleaseRemovePreImage();
    emit banTracking();
    if(shovel) shovel->setPos(GameArtwork::shovelHome());
}

void MyGameScene::toggleShovel() {
    if(!shovel) return;
    if(inputMode!=InputMode::Normal && inputMode!=InputMode::ShovelPractice) return;
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

void MyGameScene::placePlant(int row, int col,bool charge) {
    QPointF centerLoc = mapGrid->cellCenter(col,row);
    WhiteDogs *myDog = createPlant(row,col,centerLoc);
    if(!myDog) return;
    emit pleaseRemovePreImage();

    myDog->setPos(centerLoc - QPointF(myDog->pixmap().width() / 2.0, myDog->pixmap().height() / 2.0));
    myDog->setParent(this);
    myDog->setGameSpeed(gameSpeed());
    addItem(myDog);

    dogMap[row * 9 + col] = myDog;

    myDog->setItPos(row, col);//设置当前小白的所在行和列
    plantRows[row].append(myDog);
    myDog->setHealthVisible(showPlantHealth);

    connect(myDog,&WhiteDogs::pleaseRemoveMe,this,[this,myDog] { removePlant(myDog); });
    if(auto *charger=qobject_cast<LineWhite*>(myDog)) {
        connect(charger,&LineWhite::vacatedPlantingCell,this,[this,row,col,myDog] {
            if(dogMap[row*9+col]!=myDog) return;
            dogMap[row*9+col]=nullptr;
            emit plantRemoved(row,col);
        });
    }

    if(charge) cutHeart(myDog->HeartCost());

    Card::setGameState(GameState::Normal);

    if(charge) {
        AudioManager::instance().play("plant");
        new CombatEffect(this,centerLoc,CombatEffect::Plant);
        emit plantFinished();
    } else myDog->setObjectName("shovelLessonPlant");

    if(chosenNum == 3){ myDog->startMoving(); }
}

void MyGameScene::addTutorialPlant(int row,int col) {
    if(started || row<1 || row>3 || col<0 || col>=9 || dogMap[row*9+col]) return;
    const int previous=chosenNum; chosenNum=0;
    placePlant(row,col,false); chosenNum=previous;
}
void MyGameScene::generateTutorialHeart() {
    if(started) return;
    auto *heart=new Heart(QPointF(910,460),QPointF(910,460),this,QEasingCurve::Linear,nullptr,true);
    heart->setObjectName("tutorialHeart"); addHeartItem(heart);
}

WhiteDogs *MyGameScene::plantAhead(int row, qreal x) const {
    if(row<0 || row>=5) return nullptr;
    WhiteDogs *nearest=nullptr;
    for(auto *plant : plantRows[row]) {
        if(plant && plant->getHp()>0 && plant->x()<x && (!nearest || plant->x()>nearest->x())) nearest=plant;
    }
    return nearest;
}

void MyGameScene::togglePlantHealth() {
    showPlantHealth=!showPlantHealth;
    for(const auto& row : plantRows) for(auto *plant : row) plant->setHealthVisible(showPlantHealth);
    emit healthVisibilityChanged(showPlantHealth,showEnemyHealth);
}
void MyGameScene::toggleEnemyHealth() {
    showEnemyHealth=!showEnemyHealth;
    for(auto *enemy : findChildren<YellowDogs*>())
        enemy->setHealthVisible(showEnemyHealth && !enemy->isDying());
    emit healthVisibilityChanged(showPlantHealth,showEnemyHealth);
}
