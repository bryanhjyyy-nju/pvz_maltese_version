#include "audiomanager.h"
#include "playscene.h"
#include "almanacdialog.h"
#include "pausedialog.h"
#include "gameui.h"
#include <QShortcut>
#include "gamecatalog.h"
#include <QPainter>
#include <QTimer>
#include <QLabel>
#include "card.h"
#include <QVector>
#include "mygamescene.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include "levelopening.h"
#include "battlebanner.h"
#include "leveltutorial.h"

PlayScene::PlayScene(int levelNum,QWidget *parent,bool withOpening,bool endless,int firstWave) :
    GamePage(parent),
    levelIndex(levelNum), //维护传进来的关卡号, 加载地图
    endlessMode(endless),
    myGameScene(new MyGameScene(levelNum,this,withOpening,endless,firstWave)),
    myGraphicsView(new QGraphicsView(myGameScene, this))
{

    AudioManager::instance().setBattle(true);
    //先初始化Card的静态成员
    Card::setCurRestHeart(50);
    Card::setGameState(GameState::Normal);
    Card::setSelectedWhite("");

    connect(myGameScene,&MyGameScene::pleaseRemovePreImage,this,&PlayScene::stopShow);
    buildPauseBtn();

    //设置关卡数文字
    setLevelText();

    //设置卡槽
    setCardBar();

    //设置卡牌
    setCardsInBar();

    //初始化预加载图片
    preImageLabel = new QLabel(this);
    preImageLabel->setObjectName("plantPreview");
    preImageLabel->setProperty("manualScale",true);
    preImageLabel->setVisible(false);
    preImageLabel->setAttribute(Qt::WA_TransparentForMouseEvents);


    // The view fills the page; overlays share its logical canvas transform.
    myGraphicsView->setProperty("manualScale",true);
    myGraphicsView->setGeometry(rect());
    myGraphicsView->lower();

    connect(this, &PlayScene::gameLose, this, &PlayScene::finishGame);
    connect(this, &PlayScene::gameWin, this, &PlayScene::finishGame);
    connect(this, &PlayScene::gameLose, this, [=](){
        AudioManager::instance().play("lose");
        QTimer::singleShot(3000,this,[=](){
            emit this->playSceneBack();
        });
    });
    connect(this, &PlayScene::gameWin, this, [=](){
        AudioManager::instance().play("win");
        QTimer::singleShot(3000,this,[=](){
            emit this->playSceneBack();
        });
    });
    //接收游戏胜利失败暂停信号
    connect(myGameScene, &MyGameScene::gameLose, this, &PlayScene::gameLose);
    connect(myGameScene, &MyGameScene::gameWin, this, &PlayScene::gameWin);



    // 配置视图
    myGraphicsView->setRenderHint(QPainter::Antialiasing);  // 抗锯齿
    myGraphicsView->setAlignment(Qt::AlignCenter);
    myGraphicsView->setFrameShape(QFrame::NoFrame);
    myGraphicsView->setBackgroundBrush(QColor("#20291c"));
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

    // Starting resources come from the selected level's configuration.
    Card::setCurRestHeart(myGameScene->getRestHeart());

    //种植以后剩余爱心显示减少
    connect(myGameScene,&MyGameScene::plantFinished, this,[=](){
        restHeartLabel->setText(QString::number(myGameScene->getRestHeart()));
        Card::setCurRestHeart(myGameScene->getRestHeart());
        myCards[myGameScene->getChosenNum()]->startCooldown();
        if(openingActive) myCards[myGameScene->getChosenNum()]->gamePaused();
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
        previewScenePosition=mousePos;
        const auto point=myGraphicsView->viewport()->mapTo(this,myGraphicsView->mapFromScene(mousePos));
        preImageLabel->move(point-QPoint(preImageLabel->width()/2,preImageLabel->height()/2));
        preImageLabel->setVisible(true);
    });
    connect(this,&GamePage::canvasResized,this,&PlayScene::fitBattlefield);
    initializePage(QRect(20,760,250,44));
    banner=new BattleBanner(this);
    connect(myGameScene,&MyGameScene::finalWaveApproaching,this,[this] {
        banner->announce("最后一波小金毛即将来袭！","finalWave");
    });
    if(withOpening) {
        openingActive=true; Card::setGameState(GameState::Paused);
        setBattleHudVisible(false); pauseShortcut->setEnabled(false);
        myGraphicsView->setSceneRect(0,0,2100,900);
        opening=new LevelOpening(levelIndex,myGameScene,banner,this);
        connect(opening,&LevelOpening::cameraMoved,this,[this](qreal offset) { cameraOffset=offset; fitBattlefield(); });
        connect(opening,&LevelOpening::finished,this,&PlayScene::finishOpening);
        opening->start();
    }
}


void PlayScene::setLevelText(){
    //定义字体
    QFont font;
    font.setFamily("华文新魏");
    font.setBold(true);
    font.setPointSize(20);
    QString levStr = endlessMode ? "无尽模式" : QString("第 %1 关").arg(levelIndex);

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
        card->setObjectName(QString("plantCard%1").arg(i));
        card->move(470 + i * (card->width() + 5.5), 10);
        card->whiteType = plant.id;
        card->coolTime = plant.cooldownMs;
        card->heartCost = plant.cost;
        card->setToolTip(GameCatalog::plantDetails(i));


        //将卡牌指针添加到容器中
        myCards.append(card);

        //链接卡牌被选择事件和处理卡牌选择事件
        connect(card, &Card::cardSelected, this, &PlayScene::handleCardSelected);
        connect(card, &Card::cardSelected, myGameScene, [=](){
            myGameScene->setChosenNum(i);
            myGraphicsView->setMouseTracking(true);
            startShow(i);
        });

        //除爱心小狗外，其他小狗一开始就进入冷却
        if(i != 1 && myGameScene->gameplayStarted()) {
            card->startCooldown();
        }

        emit card->checkHeartEnough();

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
    const int size=qRound(plant.iconSize*2*canvasScale());
    pix = pix.scaled(size,size,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    preImageLabel->setFixedSize(size,size);
    preImageLabel->setPixmap(pix);
}

void PlayScene::stopShow(){
    preImageLabel->setVisible(false);
    myGraphicsView->setMouseTracking(false);
}

void PlayScene::gamePaused() {
    if(paused || finished) return;
    paused = true;
    myGameScene->cancelSelection();
    stopShow();
    Card::setGameState(GameState::Paused);
    pausedActivity.pause(this);
    pauseButton->setText("继续 [空格]");
    AudioManager::instance().setPaused(true);
}

void PlayScene::gameContinued() {
    if(!paused || finished) return;
    pausedActivity.resume();
    paused = false;
    if(pauseMenu) pauseMenu->hide();
    if(pauseShortcut) pauseShortcut->setEnabled(true);
    Card::setGameState(GameState::Normal);
    pauseButton->setText("暂停 [空格]");
    myGraphicsView->setFocus();
    AudioManager::instance().setPaused(false);
}

void PlayScene::finishGame() {
    if(finished) return;
    if(pauseMenu) pauseMenu->hide();
    gamePaused();
    finished = true;
    Card::setGameState(GameState::GameOver);
    pauseButton->setEnabled(false);
}

void PlayScene::buildPauseBtn() {
    pauseButton = new QPushButton("暂停 [空格]",this);
    pauseButton->setObjectName("pauseBattle");
    GameUi::styleButton(pauseButton);
    pauseButton->setFocusPolicy(Qt::NoFocus);
    pauseButton->setGeometry(20,220,170,44);
    auto toggle = [this] { togglePauseMenu(); };
    connect(pauseButton,&QPushButton::clicked,this,toggle);
    auto *shortcut = new QShortcut(QKeySequence(Qt::Key_Space),this);
    shortcut->setAutoRepeat(false);
    pauseShortcut = shortcut;
    connect(shortcut,&QShortcut::activated,this,toggle);
    auto *almanac = new QPushButton("植物 / 僵尸图鉴",this);
    GameUi::styleButton(almanac,"gold");
    almanac->setFocusPolicy(Qt::NoFocus);
    almanac->setGeometry(20,280,170,44);
    connect(almanac,&QPushButton::clicked,this,&PlayScene::showAlmanac);
    auto *sound = new QPushButton("声音设置",this);
    GameUi::styleButton(sound,"gold");
    sound->setFocusPolicy(Qt::NoFocus);
    sound->setGeometry(20,340,170,44);
    connect(sound,&QPushButton::clicked,this,&PlayScene::showAudioSettings);
    auto *help = new QLabel(this);
    auto updateHelp = [help,this](bool plants,bool enemies) {
        help->setText((levelIndex==1 ? "铲子：第二关解锁\n" : "点击铲子 / R：拿起或放下\n")+QString("右键：取消选择\nEsc：退出全屏\n空格：暂停 / 继续\nH：植物血量 %1\nJ：金毛血量 %2\nF11：全屏 / 窗口")
            .arg(plants ? "开" : "关").arg(enemies ? "开" : "关"));
    };
    updateHelp(false,false);
    help->setGeometry(20,405,250,235);
    help->setStyleSheet("color:#26392e; font-size:16px; background:rgba(255,253,245,225); border-radius:10px; padding:12px;");
    const auto& level = GameCatalog::level(levelIndex);
    auto *wave = new QLabel(endlessMode ? "无尽模式\n准备防守！" : QString("准备防守！\n共 %1 波进攻").arg(level.waves),this);
    wave->setObjectName("waveStatus");
    wave->setGeometry(20,655,250,85);
    wave->setStyleSheet("background:#ffe3a0; border:3px solid #8d6435; border-radius:16px; padding:12px; color:#65452d; font: bold 18px 'Microsoft YaHei';");
    connect(myGameScene,&MyGameScene::waveStarted,this,[wave](int current,int total) {
        wave->setText(total==0 ? QString("无尽模式 · 第 %1 波\n守住你的草坪！").arg(current) : QString("第 %1 / %2 波\n守住你的草坪！").arg(current).arg(total));
    });
    connect(myGameScene,&MyGameScene::healthVisibilityChanged,this,updateHelp);
}

void PlayScene::togglePauseMenu() {
    if(finished || (openingActive && (!tutorial || tutorial->step()!=LevelTutorial::Step::Controls))) return;
    if(paused) { gameContinued(); return; }
    if(tutorial) tutorial->notePauseUsed();
    AudioManager::instance().play("pause");
    showPauseMenu();
}

void PlayScene::showPauseMenu() {
    if(finished) return;
    gamePaused();
    if(!pauseMenu) {
        pauseMenu = new PauseDialog(this);
        connect(pauseMenu,&PauseDialog::resumeRequested,this,&PlayScene::gameContinued);
        connect(pauseMenu,&PauseDialog::mainMenuRequested,this,&PlayScene::mainMenuRequested);
        connect(pauseMenu,&PauseDialog::almanacRequested,this,&PlayScene::showAlmanac);
        connect(pauseMenu,&PauseDialog::audioRequested,this,&PlayScene::showAudioSettings);
    }
    pauseMenu->move(mapToGlobal(rect().center()) - pauseMenu->rect().center());
    pauseMenu->show();
    pauseShortcut->setEnabled(false);
}
void PlayScene::suspendToMenu() {
    gamePaused();
    for(auto *dialog : findChildren<QDialog*>()) dialog->hide();
    pauseShortcut->setEnabled(false);
}

void PlayScene::showAlmanac() {
    if(finished || (openingActive && !tutorial)) return;
    const bool wasPaused = paused;
    gamePaused();
    AlmanacDialog dialog(pauseMenu && pauseMenu->isVisible() ? static_cast<QWidget*>(pauseMenu) : this);
    dialog.exec();
    if(tutorial) tutorial->noteAlmanacViewed();
    if(!wasPaused) gameContinued();
}

void PlayScene::showAudioSettings() {
    if(finished || (openingActive && !tutorial)) return;
    const bool wasPaused = paused;
    gamePaused();
    AudioManager::instance().showSettings(pauseMenu && pauseMenu->isVisible() ? static_cast<QWidget*>(pauseMenu) : this);
    if(!wasPaused) gameContinued();
}

void PlayScene::shutdown() {
    if(opening) opening->stop();
    if(banner) banner->stop();
    if(tutorial) tutorial->hide();
    gamePaused();
    finished=true;
    myGameScene->disconnect(this);
    // Also stop activity created after the initial pause (e.g. deferred effects).
    pausedActivity.pause(this);
    if(pauseShortcut) pauseShortcut->setEnabled(false);
    for(auto *dialog : findChildren<QDialog*>()) dialog->hide();
    stopShow();
}

void PlayScene::fitBattlefield() {
    myGraphicsView->setGeometry(rect());
    myGraphicsView->resetTransform();
    myGraphicsView->scale(canvasScale(),canvasScale());
    myGraphicsView->centerOn(QPointF(825+cameraOffset,450));
    if(banner) banner->setGeometry(rect());
    if(tutorial) tutorial->fitCanvas(canvasScale(),canvasOffset());
    if(preImageLabel && Card::currentState()==GameState::PrePlace) {
        startShow(myGameScene->getChosenNum());
        const auto point=myGraphicsView->viewport()->mapTo(this,myGraphicsView->mapFromScene(previewScenePosition));
        preImageLabel->move(point-QPoint(preImageLabel->width()/2,preImageLabel->height()/2));
    }
}

void PlayScene::setBattleHudVisible(bool visible) {
    for(auto *widget : findChildren<QWidget*>(QString(),Qt::FindDirectChildrenOnly)) {
        if(widget->property("manualScale").toBool() || widget->isWindow()
            || widget->objectName()=="backToLevels" || widget->objectName()=="fullScreenButton") continue;
        widget->setVisible(visible);
    }
}
void PlayScene::beginGameplay() {
    if(finished) return;
    openingActive=false; cameraOffset=0;
    myGraphicsView->setSceneRect(0,0,1650,900); fitBattlefield();
    setBattleHudVisible(true); pauseShortcut->setEnabled(true);
    Card::setGameState(GameState::Normal);
    for(int i=0;i<myCards.size();++i) {
        if(myCards[i]->isCooling()) myCards[i]->gameContinued();
        else if(i!=1) myCards[i]->startCooldown();
        else emit myCards[i]->cooldownFinished();
    }
    myGameScene->startGameplay();
    myGraphicsView->setFocus();
}

void PlayScene::finishOpening() {
    if(finished) return;
    if(levelIndex>2) { beginGameplay(); return; }
    cameraOffset=0; myGraphicsView->setSceneRect(0,0,1650,900);
    setBattleHudVisible(true);
    tutorial=new LevelTutorial(levelIndex,myGameScene,this);
    connect(tutorial,&LevelTutorial::stepChanged,this,[this](LevelTutorial::Step step) {
        for(auto *card : myCards) card->setEnabled(false);
        Card::setGameState(GameState::Normal);
        const bool controls=step==LevelTutorial::Step::Controls;
        pauseButton->setEnabled(controls); pauseShortcut->setEnabled(controls);
        myGameScene->setInputMode(step==LevelTutorial::Step::Plant ? MyGameScene::InputMode::PlantPractice
            : step==LevelTutorial::Step::Heart ? MyGameScene::InputMode::HeartPractice
            : step==LevelTutorial::Step::Shovel ? MyGameScene::InputMode::ShovelPractice : MyGameScene::InputMode::Blocked);
        restHeartLabel->setText(QString::number(myGameScene->getRestHeart()));
        Card::setCurRestHeart(myGameScene->getRestHeart());
        if(step==LevelTutorial::Step::Plant) emit myCards[0]->cooldownFinished();
        fitBattlefield();
    });
    connect(tutorial,&LevelTutorial::finished,this,[this] {
        pauseButton->setEnabled(true); beginGameplay();
    });
    tutorial->start(); fitBattlefield();
}

bool PlayScene::handleGameKey(QKeyEvent *event) {
    if(event->modifiers()!=Qt::NoModifier) return false;
    if(event->key()!=Qt::Key_H && event->key()!=Qt::Key_J) return false;
    if(!event->isAutoRepeat()) {
        if(event->key()==Qt::Key_H) myGameScene->togglePlantHealth();
        else myGameScene->toggleEnemyHealth();
    }
    return true;
}
