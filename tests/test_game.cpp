#include <QtTest>
#include "audiomanager.h"
#include <QSettings>
#include "playscene.h"
#include "almanacdialog.h"
#include "mainscene.h"
#include "gamewindow.h"
#include "chooselevelscene.h"
#include "gamepause.h"
#include <QTabWidget>
#include <QGraphicsView>
#include <QPointer>
#include "map.h"
#include "progressstore.h"
#include <QTemporaryDir>
#include <QFile>
#include "gamecatalog.h"
#include "mygamescene.h"
#include "singingwhite.h"
#include "yellowdogs.h"
#include "pausedialog.h"
#include <QPushButton>
#include "enemyprojectile.h"
#include "waveplanner.h"
#include "combateffect.h"
#include "gameartwork.h"
#include <QRandomGenerator>
#include <QFontInfo>
#include "lawn.h"
#include "levelopening.h"
#include "battlebanner.h"
#include "leveltutorial.h"
#include "heart.h"
#include "battleresult.h"

class GameTests : public QObject {
    Q_OBJECT
    QTemporaryDir settingsDirectory;
    QString unlockedPath(const QTemporaryDir& dir) {
        const auto path=dir.filePath("progress.json");
        ProgressStore store(path); store.unlockAll(); return path;
    }
    void completeOpening(PlayScene *play) {
        auto *timeline=play->findChild<LevelOpening*>()->findChild<QVariantAnimation*>("openingTimeline");
        for(int stage=0;stage<4;++stage) timeline->setCurrentTime(timeline->duration());
        auto *banner=play->findChild<BattleBanner*>()->findChild<QVariantAnimation*>("bannerAnimation");
        banner->setCurrentTime(banner->duration());
    }
private slots:
    void previewMatchesActualEnemySize() {
        PlayScene play(10); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *opening=play.findChild<LevelOpening*>();
        const auto movies=opening->findChildren<QMovie*>(); QCOMPARE(movies.size(),12);
        for(int frame : {0,1}) {
            for(auto *movie : movies) { movie->setPaused(true); QVERIFY(movie->jumpToFrame(frame)); }
            int checked=0; QSet<int> types;
            for(auto *item : scene->items()) {
                if(item->data(0).toString()!="enemyPreview") continue;
                auto *preview=dynamic_cast<QGraphicsPixmapItem*>(item); QVERIFY(preview);
                const int type=item->data(1).toInt(); types.insert(type);
                YellowDogs actual(2,scene,type);
                auto *movie=actual.findChild<QMovie*>(); movie->setPaused(true); QVERIFY(movie->jumpToFrame(frame));
                QCOMPARE(preview->pixmap().size(),actual.pixmap().size());
                QCOMPARE(preview->pixmap().toImage().convertToFormat(QImage::Format_ARGB32),actual.pixmap().toImage().convertToFormat(QImage::Format_ARGB32));
                ++checked;
            }
            QCOMPARE(checked,12); QCOMPARE(types.size(),3);
        }
        auto *timeline=opening->findChild<QVariantAnimation*>("openingTimeline"); timeline->setCurrentTime(timeline->duration());
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(play.grab().save(folder+"/preview-battle-size.png"));
        QVERIFY(scene->findChildren<YellowDogs*>().isEmpty()); QVERIFY(!scene->gameplayStarted());
    }
    void skyHeartSpacingUsesLongerIntervals() {
        MyGameScene scene(3);
        auto *timer=scene.findChild<QTimer*>("skyHeartTimer"); QVERIFY(timer && timer->isActive());
        for(int i=0;i<20;++i) {
            QVERIFY(timer->interval()>=5000); QVERIFY(timer->interval()<=6000);
            QMetaObject::invokeMethod(timer,"timeout");
        }
        QCOMPARE(scene.findChildren<Heart*>().size(),20);
        for(auto *heart : scene.findChildren<Heart*>())
            QCOMPARE(heart->findChild<QPropertyAnimation*>("heartFallAnimation")->duration(),6000);
    }
    void skyHeartsFallSlowlyAndRemainCollectable() {
        PlayScene play(3,nullptr,false); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *view=play.findChild<QGraphicsView*>();
        scene->generateSkyHeart(); auto *sky=scene->findChild<Heart*>(); QVERIFY(sky);
        auto *fall=sky->findChild<QPropertyAnimation*>("heartFallAnimation");
        QCOMPARE(fall->duration(),6000);
        fall->setCurrentTime(3000);
        const auto start=fall->startValue().toPointF(),end=fall->endValue().toPointF();
        QVERIFY(QLineF(sky->pos(),(start+end)/2).length()<1);
        play.gamePaused(); const auto position=sky->pos(); QTest::qWait(80); QCOMPARE(sky->pos(),position);
        QCOMPARE(fall->state(),QAbstractAnimation::Paused); play.gameContinued();
        const int hearts=scene->getRestHeart();
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(sky->sceneBoundingRect().center()));
        QCOMPARE(fall->state(),QAbstractAnimation::Stopped);
        auto *collect=sky->findChild<QPropertyAnimation*>("heartCollectAnimation");
        QCOMPARE(collect->state(),QAbstractAnimation::Running); QCOMPARE(collect->duration(),800);
        QTRY_COMPARE_WITH_TIMEOUT(scene->getRestHeart(),hearts+GameCatalog::HeartValue,1200);
        scene->generateWhiteHeart(QPointF(700,450));
        auto *plantHeart=scene->findChild<Heart*>(); QVERIFY(plantHeart);
        QCOMPARE(plantHeart->findChild<QPropertyAnimation*>("heartFallAnimation")->duration(),3000);
    }
    void victoryNextLevelStartsUnlockedBattle() {
        QTemporaryDir dir; const auto path=dir.filePath("progress.json");
        GameWindow root(nullptr,path); root.show(); root.startLevel(1);
        QPointer<PlayScene> first=root.playPage(); first->gameWin();
        auto *result=first->findChild<BattleResult*>(); auto *next=result->findChild<QPushButton*>("resultNext");
        QVERIFY(next); QVERIFY(!next->isVisible());
        auto *timeline=result->findChild<QVariantAnimation*>("resultAnimation"); timeline->setCurrentTime(timeline->duration());
        QVERIFY(next->isVisible());
        for(const auto& size : {QSize(1280,720),QSize(1920,1080)}) {
            root.resize(size); QTest::qWait(20);
            QVERIFY(result->rect().contains(next->geometry()));
            QVERIFY(!next->geometry().intersects(result->findChild<QPushButton*>("resultBack")->geometry()));
        }
        QTest::mouseClick(next,Qt::LeftButton);
        auto *second=root.playPage(); QVERIFY(second && second!=first); QCOMPARE(second->levelIndex,2);
        QVERIFY(second->findChild<LevelOpening*>()); QVERIFY(!second->findChild<MyGameScene*>()->gameplayStarted());
        QCOMPARE(ProgressStore(path).resumeLevel(),2); QVERIFY(ProgressStore(path).hasUnfinishedLevel());
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete); QVERIFY(first.isNull());
        root.close();
        PlayScene last(10,nullptr,false); last.show(); last.gameWin();
        QVERIFY(!last.findChild<QPushButton*>("resultNext"));
    }
    void finalWaveTextAllowsPlantingAndShoveling() {
        PlayScene play(4,nullptr,false); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *view=play.findChild<QGraphicsView*>();
        auto *card=play.findChild<Card*>("plantCard0"); auto *banner=play.findChild<BattleBanner*>();
        scene->addHeart(100); Card::setCurRestHeart(scene->getRestHeart()); emit card->cooldownFinished();
        const auto before=play.grab().toImage();
        emit scene->finalWaveApproaching();
        auto *animation=banner->findChild<QVariantAnimation*>("bannerAnimation"); animation->setCurrentTime(450);
        QVERIFY(banner->isVisible()); QVERIFY(banner->testAttribute(Qt::WA_TransparentForMouseEvents));
        QVERIFY(scene->getGameTimer()->isActive());
        QCOMPARE(play.grab().toImage().pixelColor(1000,700),before.pixelColor(1000,700));
        const auto point=view->viewport()->mapTo(&play,view->mapFromScene(QPointF(805,450)));
        auto *target=play.childAt(point); QCOMPARE(target,view->viewport());
        QTest::mouseClick(card,Qt::LeftButton); QCOMPARE(Card::currentState(),GameState::PrePlace);
        QTest::mouseClick(target,Qt::LeftButton,Qt::NoModifier,target->mapFrom(&play,point));
        QCOMPARE(scene->findChildren<WhiteDogs*>().size(),1);
        scene->toggleShovel(); QCOMPARE(Card::currentState(),GameState::Shoveling);
        QTest::mouseClick(target,Qt::LeftButton,Qt::NoModifier,target->mapFrom(&play,point));
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QVERIFY(scene->findChildren<WhiteDogs*>().isEmpty()); QVERIFY(banner->isVisible());
        QCOMPARE(Card::currentState(),GameState::Normal);
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(play.grab().save(folder+"/final-wave-interactive.png"));
    }
    void goldPavingFallsInOrder() {
        Lawn first(1); first.setRevealProgress(0);
        for(int row=0;row<5;++row) for(int col=0;col<9;++col) QCOMPARE(first.brickFallProgress(row,col),0.0);
        first.setRevealProgress(.5);
        for(int col=0;col<9;++col) QCOMPARE(first.brickFallProgress(2,col),col<4 ? 1.0 : col==4 ? .5 : 0.0);
        QCOMPARE(first.brickFallProgress(0,0),0.0);
        Lawn third(3); third.setRevealProgress(.5);
        for(int row=1;row<=3;++row) for(int col=0;col<9;++col) QCOMPARE(third.brickFallProgress(row,col),1.0);
        for(int row : {0,4}) {
            QCOMPARE(third.brickFallProgress(row,0),1.0);
            QCOMPARE(third.brickFallProgress(row,4),.5);
            QCOMPARE(third.brickFallProgress(row,8),0.0);
        }
        third.setRevealProgress(1);
        for(int row=0;row<5;++row) for(int col=0;col<9;++col) QCOMPARE(third.brickFallProgress(row,col),1.0);
    }
    void firstLevelHasNoShovelOrDefeatedCounter() {
        for(int level : {1,2}) {
            PlayScene play(level,nullptr,false); play.show();
            auto *scene=play.findChild<MyGameScene*>(); auto *view=play.findChild<QGraphicsView*>();
            bool hasShovel=false;
            for(auto *item : scene->items()) {
                if(item->toolTip().startsWith("可爱铲子")) hasShovel=true;
                QVERIFY(!dynamic_cast<QGraphicsTextItem*>(item));
            }
            QCOMPARE(hasShovel,level==2);
            view->setFocus(); QTest::keyClick(view,Qt::Key_R);
            QCOMPARE(Card::currentState(),level==2 ? GameState::Shoveling : GameState::Normal);
            QVERIFY(!play.findChild<QPushButton*>("backToLevels"));
        }
    }
    void savedOpeningResumesPaused() {
        QTemporaryDir dir; const auto path=dir.filePath("progress.json");
        ProgressStore saved(path); QVERIFY(saved.startLevel(1));
        GameWindow root(nullptr,path); root.show(); root.continueGame();
        auto *play=root.playPage(); QVERIFY(play && play->isPaused());
        auto *opening=play->findChild<LevelOpening*>(); auto *timeline=opening->findChild<QVariantAnimation*>("openingTimeline");
        QCOMPARE(timeline->state(),QAbstractAnimation::Paused);
        const auto time=timeline->currentTime(); QTest::qWait(80); QCOMPARE(timeline->currentTime(),time);
        auto *menu=play->findChild<PauseDialog*>(); QVERIFY(menu && menu->isVisible());
        QTimer::singleShot(30,menu,[menu] { auto *dialog=menu->findChild<AlmanacDialog*>(); QVERIFY(dialog); dialog->reject(); });
        QTest::mouseClick(menu->findChild<QPushButton*>("almanac"),Qt::LeftButton);
        QVERIFY(play->isPaused()); QVERIFY(menu->isVisible());
        play->gameContinued(); QCOMPARE(timeline->state(),QAbstractAnimation::Running);
        root.close();
    }
    void victoryRewardsAndDefeatSpotlight() {
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        for(int level : {1,7,8,10}) {
            PlayScene play(level,nullptr,false); play.show();
            play.gameWin(); auto *result=play.findChild<BattleResult*>(); QVERIFY(result && result->victory());
            QCOMPARE(result->rewardPlant(),level<8 ? level : -1);
            auto *animation=result->findChild<QVariantAnimation*>("resultAnimation");
            animation->setCurrentTime(animation->duration());
            if(!folder.isEmpty()) QVERIFY(play.grab().save(folder+QString("/victory-level%1.png").arg(level)));
            QVERIFY(result->findChild<QPushButton*>("resultBack")->isVisible());
            QSignalSpy back(&play,&PlayScene::playSceneBack);
            QTest::mouseClick(result->findChild<QPushButton*>("resultBack"),Qt::LeftButton); QCOMPARE(back.count(),1);
            for(auto *timer : play.findChild<MyGameScene*>()->findChildren<QTimer*>()) QVERIFY(!timer->isActive());
        }
        QTemporaryDir dir; const auto path=unlockedPath(dir);
        GameWindow root(nullptr,path,false); root.show(); root.startLevel(8);
        auto *play=root.playPage(); auto *scene=play->findChild<MyGameScene*>();
        scene->setAYellowDog(2); auto *enemy=static_cast<YellowDogs*>(scene->getZombieMap(2).front());
        enemy->stopMoving(); enemy->setPos(80,430);
        auto *view=play->findChild<QGraphicsView*>();
        const auto before=view->viewport()->grab().toImage(); emit enemy->arrivedYourHome();
        auto *result=play->findChild<BattleResult*>(); QVERIFY(result && !result->victory());
        auto *animation=result->findChild<QVariantAnimation*>("resultAnimation");
        animation->setCurrentTime(qRound(animation->duration()*.38));
        const auto spotlight=play->grab().toImage();
        const auto center=view->viewport()->mapTo(play,view->mapFromScene(scene->defeatPosition()));
        QCOMPARE(spotlight.pixelColor(center),before.pixelColor(view->mapFromScene(scene->defeatPosition())));
        QCOMPARE(spotlight.pixelColor(QPoint(1000,400)),QColor(Qt::black));
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+"/defeat-spotlight.png"));
        QVERIFY(!ProgressStore(path).hasUnfinishedLevel());
        animation->setCurrentTime(qRound(animation->duration()*.70));
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+"/defeat-message.png"));
        animation->setCurrentTime(animation->duration());
        QVERIFY(!root.playPage()); QVERIFY(root.levelPage()->isVisible());
    }
    void cardCooldownAndAffordability() {
        Card card(1); card.heartCost=50; card.coolTime=5000;
        card.setFixedSize(160,225); card.show(); Card::setGameState(GameState::Normal);
        auto capture=[&](const QString& name) {
            const auto pixels=card.grab().toImage(); const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
            if(!folder.isEmpty()) pixels.save(folder+"/card-"+name+".png"); return pixels;
        };
        Card::setCurRestHeart(50); emit card.cooldownFinished();
        QVERIFY(card.isEnabled()); const auto ready=capture("ready");
        const QPoint heart(125,188); QVERIFY(ready.pixelColor(heart).red()>ready.pixelColor(heart).green()*1.5);
        Card::setCurRestHeart(0); emit card.checkHeartEnough();
        QVERIFY(!card.isEnabled()); QCOMPARE(card.getCardState(),CardState::Unable);
        const auto lacking=capture("lacking");
        const auto gray=lacking.pixelColor(heart); QVERIFY(qAbs(gray.red()-gray.green())<5);
        card.startCooldown(); card.gamePaused();
        Card::setCurRestHeart(100); emit card.checkHeartEnough();
        QVERIFY(!card.isEnabled()); QCOMPARE(card.getCardState(),CardState::Cooling);
        const auto cooling=capture("cooling");
        QVERIFY(cooling.pixelColor(heart).value()<lacking.pixelColor(heart).value());
        card.setCoolProgress(.5); const auto half=capture("half");
        const QPoint lower(30,160),upper(30,35);
        QVERIFY(half.pixelColor(lower).value()>cooling.pixelColor(lower).value());
        QCOMPARE(half.pixelColor(upper),cooling.pixelColor(upper));
        emit card.cooldownFinished(); QVERIFY(card.isEnabled());
        QCOMPARE(capture("ready-again").pixelColor(heart),ready.pixelColor(heart));
        Card::setCurRestHeart(0); emit card.checkHeartEnough();
        QCOMPARE(card.getCardState(),CardState::Unable);
        QSignalSpy selected(&card,&Card::cardSelected); QTest::mouseClick(&card,Qt::LeftButton); QCOMPARE(selected.count(),0);
    }
    void progressionAndSaveManagement() {
        QTemporaryDir dir; const auto path=dir.filePath("fresh.json");
        GameWindow root(nullptr,path,false); root.show(); root.showLevels();
        for(int level=1;level<=10;++level)
            QCOMPARE(root.levelPage()->findChild<QPushButton*>(QString("level%1").arg(level))->isVisible(),level==1);
        root.startLevel(2); QVERIFY(!root.playPage());
        for(int level=1;level<=10;++level) {
            root.startLevel(level); QVERIFY(root.playPage());
            root.playPage()->gameWin(); root.playPage()->playSceneBack();
            QCOMPARE(ProgressStore(path).unlockedLevel(),qMin(10,level+1));
        }
        root.showMenu(); auto *endless=root.homePage()->findChild<QPushButton*>("endlessGame");
        QVERIFY(endless->isVisible()); QCOMPARE(endless->text(),QString("开始无尽模式"));
        QTimer::singleShot(30,&root,[&root] {
            auto *dialog=root.findChild<QDialog*>("saveSettingsDialog"); QVERIFY(dialog);
            QTest::mouseClick(dialog->findChild<QPushButton*>("resetSave"),Qt::LeftButton);
        });
        root.showSaveSettings(); QCOMPARE(ProgressStore(path).unlockedLevel(),1);
        QVERIFY(!endless->isVisible());
        QTimer::singleShot(30,&root,[&root] {
            auto *dialog=root.findChild<QDialog*>("saveSettingsDialog"); QVERIFY(dialog);
            QTest::mouseClick(dialog->findChild<QPushButton*>("unlockSave"),Qt::LeftButton);
        });
        root.showSaveSettings(); QCOMPARE(ProgressStore(path).unlockedLevel(),10);
        QVERIFY(endless->isVisible());
        for(const auto& name : {"startGame","resumeGame","quitGame","menuAlmanac"}) {
            auto *button=root.homePage()->findChild<QPushButton*>(name); QVERIFY(button);
            QCOMPARE(button->size(),QSize(430,84)); QCOMPARE(button->property("color").toString(),QString("sunshine"));
        }
    }
    void endlessWavesAndResume() {
        QRandomGenerator random(7);
        for(int wave=1;wave<=20;++wave) {
            int weight=0; for(int type : WavePlanner::endlessWave(wave,random)) weight+=GameCatalog::enemies()[type].weight;
            QCOMPARE(weight,4+2*wave);
        }
        QTemporaryDir dir; const auto path=unlockedPath(dir);
        GameWindow root(nullptr,path,false); root.show(); root.startEndless();
        auto *play=root.playPage(); QVERIFY(play && play->endlessMode);
        auto *scene=play->findChild<MyGameScene*>();
        auto *waveTimer=scene->findChild<QTimer*>("waveTimer"); auto *stagger=scene->findChild<QTimer*>("waveStaggerTimer");
        QSignalSpy win(scene,&MyGameScene::gameWin), final(scene,&MyGameScene::finalWaveApproaching);
        for(int wave=1;wave<=3;++wave) {
            QMetaObject::invokeMethod(waveTimer,"timeout");
            while(stagger->isActive()) QMetaObject::invokeMethod(stagger,"timeout");
            QCOMPARE(scene->wavesStarted(),wave); QVERIFY(!waveTimer->isActive());
            for(auto *enemy : scene->findChildren<YellowDogs*>()) {
                QVERIFY(enemy->getHp()>=GameCatalog::enemies()[enemy->typeIndex()].health);
                enemy->getAttacked(100000);
            }
            QTRY_VERIFY_WITH_TIMEOUT(waveTimer->isActive(),1200);
            QCOMPARE(ProgressStore(path).endlessBest(),wave);
        }
        QCOMPARE(win.count(),0); QCOMPARE(final.count(),0);
        play->showPauseMenu(); play->mainMenuRequested();
        QCOMPARE(root.homePage()->findChild<QPushButton*>("endlessGame")->text(),QString("继续无尽模式"));
        QVERIFY(!root.homePage()->findChild<QPushButton*>("resumeGame")->isEnabled());
        root.startEndless(); QCOMPARE(root.playPage(),play); QVERIFY(play->isPaused());
        QVERIFY(play->findChild<PauseDialog*>()->isVisible());
        play->gameLose(); play->playSceneBack(); root.showMenu();
        QCOMPARE(root.homePage()->findChild<QPushButton*>("endlessGame")->text(),QString("开始无尽模式"));
        QCOMPARE(ProgressStore(path).endlessBest(),3); QVERIFY(!ProgressStore(path).hasEndlessRun());
    }
    void initTestCase() {
        QApplication::setQuitOnLastWindowClosed(false);
        QCoreApplication::setOrganizationName("PvZTests");
        QCoreApplication::setApplicationName("PvZTests");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settingsDirectory.path());
        QSettings().setValue("audio/music",0);
        QSettings().setValue("audio/effects",0);
    }
    void audioAssetsLoad() {
        auto& manager = AudioManager::instance();
        const auto sounds = manager.findChildren<QSoundEffect*>();
        QCOMPARE(sounds.size(),20);
        for(auto *sound : sounds) {
            QTRY_VERIFY_WITH_TIMEOUT(sound->status() != QSoundEffect::Loading,5000);
            QCOMPARE(sound->status(),QSoundEffect::Ready);
        }
    }
    void pausePreservesActivity() {
        QObject root;
        QTimer running(&root), idle(&root);
        running.setInterval(400);
        QSignalSpy ticks(&running,&QTimer::timeout);
        running.start();
        QTest::qWait(100);
        GamePause pause;
        pause.pause(&root);
        QTest::qWait(420);
        QCOMPARE(ticks.count(),0);
        pause.resume();
        QTest::qWait(40);
        pause.pause(&root); // Pause a partial resumed interval again.
        pause.resume();
        QTRY_COMPARE_WITH_TIMEOUT(ticks.count(),1,450);
        QCOMPARE(running.interval(),400);
        QVERIFY(!idle.isActive());
    }
    void battlefieldPauseAndPlacement() {
        PlayScene play(8,nullptr,false);
        play.show();
        auto *scene = play.findChild<MyGameScene*>();
        auto *view = play.findChild<QGraphicsView*>();
        QVERIFY(scene); QVERIFY(view);
        scene->setChosenNum(1);
        Card::setGameState(GameState::PrePlace);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        QCOMPARE(scene->getRestHeart(),GameCatalog::level(8).startingHearts-50);
        auto plants = scene->findChildren<WhiteDogs*>();
        QCOMPARE(plants.size(),1);
        scene->setAYellowDog(2);
        const auto *enemy = scene->getZombieMap(2).front();
        play.gamePaused();
        const auto position = enemy->pos();
        for(auto *timer : scene->findChildren<QTimer*>()) QVERIFY(!timer->isActive());
        QTest::qWait(180);
        QCOMPARE(enemy->pos(),position);
        play.gameContinued();
        QTest::qWait(180);
        QVERIFY(enemy->pos().x() < position.x());
        QVERIFY(plants.front()->findChild<QTimer*>()->isActive());
        play.gamePaused();
    }
    void spacePauseMenu() {
        QTemporaryDir dir;
        GameWindow picker(nullptr,unlockedPath(dir),false);
        picker.show();
        picker.startLevel(4);
        auto *view = picker.playPage()->findChild<QGraphicsView*>();
        view->setFocus();
        QTest::qWait(50);
        QTest::keyClick(view,Qt::Key_Space);
        auto *menu = picker.playPage()->findChild<PauseDialog*>();
        QVERIFY(menu); QVERIFY(menu->isVisible());
        QCOMPARE(Card::currentState(),GameState::Paused);
        QTest::qWait(50);
        bool almanacStayedPaused=false;
        QTimer::singleShot(70,menu,[&] {
            for(auto *dialog : menu->findChildren<QDialog*>()) {
                if(!dynamic_cast<AlmanacDialog*>(dialog)) continue;
                almanacStayedPaused=Card::currentState()==GameState::Paused;
                dialog->reject();
            }
        });
        QTest::mouseClick(menu->findChild<QPushButton*>("almanac"),Qt::LeftButton);
        QVERIFY(almanacStayedPaused);
        QVERIFY(menu->isVisible());
        QTest::keyClick(menu,Qt::Key_Space);
        QVERIFY(!menu->isVisible());
        QCOMPARE(Card::currentState(),GameState::Normal);
        view->setFocus(); QTest::qWait(30);
        QTest::mouseClick(picker.playPage()->findChild<QPushButton*>("pauseBattle"),Qt::LeftButton);
        QVERIFY(menu->isVisible()); QVERIFY(picker.playPage()->isPaused());
        QTest::mouseClick(menu->findChild<QPushButton*>("mainMenu"),Qt::LeftButton);
        QCOMPARE(picker.currentPage(),static_cast<GamePage*>(picker.homePage()));
        QVERIFY(picker.playPage()->isPaused());
        QVERIFY(!menu->isVisible());
        QCOMPARE(ProgressStore(dir.filePath("progress.json")).resumeLevel(),4);
    }
    void mouseShovel() {
        PlayScene play(8,nullptr,false);
        play.show();
        auto *scene = play.findChild<MyGameScene*>();
        auto *view = play.findChild<QGraphicsView*>();
        QGraphicsPixmapItem *shovel=nullptr;
        for(auto *item : scene->items())
            if(item->toolTip().startsWith("可爱铲子")) shovel=dynamic_cast<QGraphicsPixmapItem*>(item);
        QVERIFY(shovel);
        QCOMPARE(shovel->pos(),GameArtwork::shovelHome());
        QVERIFY(GameArtwork::shovelSlotRect().contains(shovel->sceneBoundingRect()));
        auto click = [&](const QPointF& pos) {
            QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(pos));
        };
        click(QPointF(1240,50));
        QCOMPARE(Card::currentState(),GameState::Shoveling);
        QTest::mouseMove(view->viewport(),view->mapFromScene(QPointF(580,500)));
        const auto position=view->mapFromScene(QPointF(580,500));
        QMouseEvent movement(QEvent::MouseMove,QPointF(position),Qt::NoButton,Qt::NoButton,Qt::NoModifier);
        QApplication::sendEvent(view->viewport(),&movement);
        QTRY_VERIFY(shovel->pos()!=GameArtwork::shovelHome());
        click(QPointF(1240,50));
        QCOMPARE(Card::currentState(),GameState::Normal);
        QCOMPARE(shovel->pos(),GameArtwork::shovelHome());
        scene->setChosenNum(1);
        Card::setGameState(GameState::PrePlace);
        click(QPointF(440,490));
        QCOMPARE(scene->findChildren<WhiteDogs*>().size(),1);
        click(QPointF(1240,50));
        click(QPointF(440,490));
        QCOMPARE(Card::currentState(),GameState::Normal);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QVERIFY(scene->findChildren<WhiteDogs*>().isEmpty());
        QTest::keyClick(view,Qt::Key_R);
        QCOMPARE(Card::currentState(),GameState::Shoveling);
        QTest::mouseClick(view->viewport(),Qt::RightButton);
        QCOMPARE(Card::currentState(),GameState::Normal);
        play.gamePaused();
        scene->toggleShovel();
        QCOMPARE(Card::currentState(),GameState::Paused);
    }
    void renderScreens() {
        const QString folder = qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(folder.isEmpty()) QSKIP("Set PVZ_CAPTURE_DIR to export UI review images.");
        QDir().mkpath(folder);
        AlmanacDialog almanac;
        almanac.show(); QTest::qWait(100);
        QVERIFY(almanac.grab().save(folder+"/plants.png"));
        almanac.findChild<QTabWidget*>()->setCurrentIndex(1);
        QTest::qWait(100);
        QVERIFY(almanac.grab().save(folder+"/enemies.png"));
        MainScene menu;
        menu.show(); QTest::qWait(100);
        QVERIFY(menu.grab().save(folder+"/menu.png"));
        PlayScene play(8,nullptr,false);
        play.show(); QTest::qWait(100);
        play.gamePaused();
        QVERIFY(play.grab().save(folder+"/battle.png"));
        play.gameContinued();
        auto *view=play.findChild<QGraphicsView*>();
        view->setFocus(); QTest::qWait(50);
        QTest::keyClick(view,Qt::Key_Space); QTest::qWait(50);
        auto *pause=play.findChild<PauseDialog*>();
        QVERIFY(pause); QVERIFY(pause->isVisible());
        QVERIFY(pause->grab().save(folder+"/pause.png"));
        play.gameContinued();
        auto *scene=play.findChild<MyGameScene*>();
        scene->setChosenNum(1);
        Card::setGameState(GameState::PrePlace);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        for(int i=0;i<3;++i) {
            scene->setAYellowDog(i+1,i);
            auto *enemy=static_cast<YellowDogs*>(scene->getZombieMap(i+1).back());
            enemy->stopMoving(); enemy->setPos(750,130+145*(i+1+.5)-enemy->boundingRect().height()/2);
            if(i==0) enemy->getAttacked(30);
            if(i==1) enemy->shootNote();
            if(i==2) enemy->getAttacked(10000);
        }
        QTest::qWait(120); play.gamePaused();
        QVERIFY(play.grab().save(folder+"/combat.png"));
        QTimer::singleShot(70,&play,[&play,folder] {
            for(auto *dialog : play.findChildren<QDialog*>()) {
                if(dialog->windowTitle()!="声音设置") continue;
                dialog->grab().save(folder+"/audio.png");
                dialog->reject();
            }
        });
        AudioManager::instance().showSettings(&play);
    }
    void continueFlow() {
        QTemporaryDir dir;
        const auto path=dir.filePath("progress.json");
        GameWindow root(nullptr,path,false); root.show();
        auto *resume=root.homePage()->findChild<QPushButton*>("resumeGame");
        QVERIFY(!resume->isEnabled()); root.continueGame(); QVERIFY(!root.playPage());
        root.startLevel(1); auto *play=root.playPage(); QVERIFY(play);
        auto *scene=play->findChild<MyGameScene*>(); scene->setAYellowDog(2);
        auto *enemy=scene->getZombieMap(2).front();
        root.showMenu(); QCOMPARE(root.currentPage(),static_cast<GamePage*>(play));
        play->showPauseMenu(); play->mainMenuRequested();
        QVERIFY(resume->isEnabled()); QVERIFY(play->isPaused());
        const auto position=enemy->pos(); QTest::qWait(120); QCOMPARE(enemy->pos(),position);
        root.continueGame(); QCOMPARE(root.playPage(),play);
        QVERIFY(play->isPaused()); QVERIFY(play->findChild<PauseDialog*>()->isVisible());
        QCOMPARE(enemy->pos(),position);
        play->gameContinued(); QTest::qWait(100); QVERIFY(enemy->x()<position.x());
        play->gameWin(); QVERIFY(!ProgressStore(path).hasUnfinishedLevel());
        play->playSceneBack(); QVERIFY(!root.playPage());
        root.showMenu(); QVERIFY(!resume->isEnabled()); root.continueGame(); QVERIFY(!root.playPage());
        root.startLevel(2); root.playPage()->gameLose(); root.playPage()->playSceneBack();
        root.showMenu(); QVERIFY(!resume->isEnabled());
        // A process restart restores the level checkpoint directly into pause.
        ProgressStore saved(path); QVERIFY(saved.startLevel(2));
        GameWindow restored(nullptr,path,false); restored.show(); restored.continueGame();
        QVERIFY(restored.playPage()); QVERIFY(restored.playPage()->isPaused());
        QVERIFY(restored.playPage()->findChild<PauseDialog*>()->isVisible());
    }
    void independentLiveHealthOverlays() {
        PlayScene play(8,nullptr,false);
        play.show();
        auto *scene=play.findChild<MyGameScene*>();
        auto *view=play.findChild<QGraphicsView*>();
        view->setFocus(); QTest::qWait(40);
        QTest::keyClick(view,Qt::Key_H);
        QVERIFY(scene->plantHealthVisible()); QVERIFY(!scene->enemyHealthVisible());
        scene->setChosenNum(1); Card::setGameState(GameState::PrePlace);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        auto *plant=scene->plantAhead(2,1000);
        QVERIFY(plant); QVERIFY(plant->isHealthVisible());
        QCOMPARE(plant->healthText(),QString::number(plant->getHp()));
        scene->setAYellowDog(2,1);
        auto *enemy=static_cast<YellowDogs*>(scene->getZombieMap(2).back());
        QVERIFY(!enemy->isHealthVisible());
        QTest::keyClick(view,Qt::Key_J);
        QVERIFY(enemy->isHealthVisible()); QVERIFY(plant->isHealthVisible());
        enemy->stopMoving(); enemy->setPos(800,420);
        enemy->getAttacked(30);
        QCOMPARE(enemy->healthText(),QString("450"));
        enemy->startAttacking(plant);
        QCOMPARE(plant->healthText(),QString("440"));
        play.gamePaused();
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(play.grab().save(folder+"/health.png"));
        QTest::keyClick(view,Qt::Key_H);
        QVERIFY(!plant->isHealthVisible()); QVERIFY(enemy->isHealthVisible());
        QCOMPARE(Card::currentState(),GameState::Paused);
        QTest::keyClick(view,Qt::Key_J);
        QVERIFY(!enemy->isHealthVisible());
        QTest::keyClick(view,Qt::Key_J);
        play.gameContinued();
        enemy->getAttacked(10000);
        QVERIFY(!enemy->isHealthVisible());
        scene->toggleEnemyHealth(); scene->toggleEnemyHealth();
        QVERIFY(!enemy->isHealthVisible());
        scene->setAYellowDog(1);
        QVERIFY(scene->getZombieMap(1).back()->isHealthVisible());
        QTest::keyClick(view,Qt::Key_H);
        scene->setChosenNum(1); Card::setGameState(GameState::PrePlace);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(561,490)));
        QCOMPARE(scene->findChildren<WhiteDogs*>().size(),2);
        for(auto *unit : scene->findChildren<WhiteDogs*>()) QVERIFY(unit->isHealthVisible());
    }
    void cartoonMenuButtons() {
        QTemporaryDir dir;
        GameWindow root(nullptr,unlockedPath(dir),false);
        root.show();
        auto *menu=root.homePage();
        QCOMPARE(root.windowTitle(),QString("小白大战小金毛"));
        auto *start=menu->findChild<QPushButton*>("startGame");
        auto *quit=menu->findChild<QPushButton*>("quitGame");
        QVERIFY(start); QVERIFY(quit);
        QCOMPARE(start->text(),QString("选择关卡"));
        QCOMPARE(quit->text(),QString("退出游戏"));
        QVERIFY(start->icon().isNull()); QVERIFY(quit->icon().isNull());
        QCOMPARE(start->property("color").toString(),QString("sunshine"));
        const auto nativeId=root.winId();
        QTest::mouseClick(start,Qt::LeftButton);
        QVERIFY(!menu->isVisible()); QVERIFY(root.levelPage()->isVisible());
        QCOMPARE(root.winId(),nativeId);
        QCOMPARE(root.levelPage()->window(),static_cast<QWidget*>(&root));
        root.levelPage()->backRequested();
        QVERIFY(menu->isVisible());
        QTest::mouseClick(quit,Qt::LeftButton);
        QVERIFY(!root.isVisible());
    }
    void progressRoundTrip() {
        QTemporaryDir dir;
        const auto path = dir.filePath("save/progress.json");
        ProgressStore store(path);
        QVERIFY(!store.hasProgress());
        QVERIFY(!store.startLevel(3));
        QVERIFY(store.completeLevel(1)); QVERIFY(store.completeLevel(2));
        QVERIFY(store.startLevel(3));
        QCOMPARE(ProgressStore(path).resumeLevel(),3);
        QVERIFY(store.completeLevel(3));
        QCOMPARE(ProgressStore(path).resumeLevel(),4);
        QVERIFY(!store.completeLevel(10));
        for(int level=4;level<=10;++level) QVERIFY(store.completeLevel(level));
        QCOMPARE(ProgressStore(path).resumeLevel(),10);
        QVERIFY(store.startLevel(1));
        QCOMPARE(ProgressStore(path).highestCompleted(),10);
        QVERIFY(!store.startLevel(11));
        QCOMPARE(ProgressStore(path).resumeLevel(),1);
        QFile file(path); QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("broken json"); file.close();
        ProgressStore broken(path);
        QVERIFY(!broken.hasProgress()); QVERIFY(!broken.error().isEmpty());
        QCOMPARE(broken.resumeLevel(),1);
        QVERIFY(broken.startLevel(1));
        QCOMPARE(ProgressStore(path).resumeLevel(),1);
    }
    void progressWriteFailure() {
        QTemporaryDir dir;
        ProgressStore store(dir.path()); // A directory cannot be replaced by a file.
        QVERIFY(!store.startLevel(1));
        QVERIFY(!store.error().isEmpty());
        QVERIFY(!store.hasProgress());
    }
    void gridBoundaries() {
        Map map(9,5,QSize(121,145),QPointF(380,130));
        int col, row;
        QVERIFY(!map.turnPosToMap(QPointF(379,130),col,row));
        QVERIFY(!map.turnPosToMap(QPointF(380,129),col,row));
        QVERIFY(map.turnPosToMap(QPointF(380,130),col,row));
        QCOMPARE(col,0); QCOMPARE(row,0);
        QVERIFY(!map.turnPosToMap(QPointF(1469,855),col,row));
        QCOMPARE(map.cellCenter(0,0),QPointF(440,202));
    }
    void catalogMatchesGameplay() {
        MyGameScene scene(1);
        SingingWhite plant(2,0,&scene);
        QCOMPARE(plant.getHp(),GameCatalog::plants()[0].health);
        QCOMPARE(plant.HeartCost(),GameCatalog::plants()[0].cost);
        for(int i=0;i<3;++i) {
            YellowDogs enemy(2,&scene,i);
            QCOMPARE(enemy.getHp(),GameCatalog::enemies()[i].health);
        }
    }
    void fixedWaveBudgetsAndRarity() {
        int counts[3] = {};
        for(int level=1;level<=10;++level) {
            const auto& stats=GameCatalog::level(level);
            for(int seed=1;seed<=500;++seed) {
                QRandomGenerator random(seed);
                const auto plan=WavePlanner::create(level,random);
                QCOMPARE(plan.size(),stats.waves);
                for(const auto& wave : plan) {
                    int sum=0;
                    for(int type : wave) {
                        QVERIFY(type<=stats.maxEnemyType);
                        sum+=GameCatalog::enemies()[type].weight;
                        if(level==10) ++counts[type];
                    }
                    QCOMPARE(sum,stats.waveWeight);
                }
            }
        }
        QVERIFY(counts[0]>counts[1]*5);
        QVERIFY(counts[1]>counts[2]*2);
        QCOMPARE(GameCatalog::level(8).waves*GameCatalog::level(8).waveWeight,28);
        QCOMPARE(GameCatalog::level(9).waves*GameCatalog::level(9).waveWeight,35);
        QCOMPARE(GameCatalog::level(10).waves*GameCatalog::level(10).waveWeight,40);
    }
    void waveScheduling() {
        MyGameScene scene(10);
        auto *waveTimer=scene.findChild<QTimer*>("waveTimer");
        auto *stagger=scene.findChild<QTimer*>("waveStaggerTimer");
        QVERIFY(waveTimer); QVERIFY(stagger);
        QSignalSpy started(&scene,&MyGameScene::waveStarted);
        int previous=0;
        for(int i=0;i<GameCatalog::level(10).waves;++i) {
            QMetaObject::invokeMethod(waveTimer,"timeout",Qt::DirectConnection);
            while(stagger->isActive()) QMetaObject::invokeMethod(stagger,"timeout",Qt::DirectConnection);
            const auto enemies=scene.findChildren<YellowDogs*>();
            int sum=0;
            for(int j=previous;j<enemies.size();++j) sum+=GameCatalog::enemies()[enemies[j]->typeIndex()].weight;
            QCOMPARE(sum,GameCatalog::level(10).waveWeight);
            previous=enemies.size();
        }
        QCOMPARE(started.count(),GameCatalog::level(10).waves);
        QCOMPARE(previous,scene.totalEnemies());
        QVERIFY(!waveTimer->isActive()); QVERIFY(!stagger->isActive());
    }
    void allLevelsFinish_data() {
        QTest::addColumn<int>("level");
        for(int i=1;i<=10;++i) QTest::newRow(qPrintable(QString::number(i))) << i;
    }
    void allLevelsFinish() {
        QFETCH(int,level);
        MyGameScene scene(level);
        QSignalSpy wins(&scene,&MyGameScene::gameWin);
        for(int i=0;i<scene.totalEnemies();++i) scene.setAYellowDog(i%5,i%3);
        for(int row=0;row<5;++row) {
            const auto enemies = scene.getZombieMap(row);
            for(auto *enemy : enemies) {
                auto *target = static_cast<YellowDogs*>(enemy);
                target->getAttacked(10000);
                target->getAttacked(10000); // Duplicate damage must not count another kill.
            }
        }
        QTRY_COMPARE_WITH_TIMEOUT(wins.count(),1,1000);
        for(auto *timer : scene.findChildren<QTimer*>()) QVERIFY(!timer->isActive());
        scene.setAYellowDog(2);
        QVERIFY(scene.getZombieMap(2).isEmpty());
    }
    void loseStopsActivity() {
        PlayScene play(1,nullptr,false);
        auto *scene = play.findChild<MyGameScene*>();
        scene->setAYellowDog(2);
        auto *enemy = static_cast<YellowDogs*>(scene->getZombieMap(2).front());
        QSignalSpy loses(&play,&PlayScene::gameLose);
        enemy->stopMoving();
        enemy->setPos(0,enemy->pos().y());
        QTRY_COMPARE_WITH_TIMEOUT(loses.count(),1,800);
        QCOMPARE(Card::currentState(),GameState::GameOver);
        const auto pos = enemy->pos();
        play.gameContinued();
        QTest::qWait(100);
        QCOMPARE(enemy->pos(),pos);
    }
    void meleeFeedback() {
        PlayScene play(8,nullptr,false);
        play.show();
        auto *scene=play.findChild<MyGameScene*>();
        auto *view=play.findChild<QGraphicsView*>();
        scene->setChosenNum(1);
        Card::setGameState(GameState::PrePlace);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        auto *plant=scene->plantAhead(2,1000);
        QVERIFY(plant);
        scene->setAYellowDog(2,1);
        auto *enemy=static_cast<YellowDogs*>(scene->getZombieMap(2).front());
        enemy->stopMoving(); enemy->setPos(plant->pos()+QPointF(60,0));
        QVERIFY(enemy->checkCollision());
        enemy->shootNote();
        QVERIFY(scene->findChildren<EnemyProjectile*>().isEmpty());
        int health=plant->getHp();
        enemy->startAttacking(plant);
        QCOMPARE(plant->getHp(),health-GameCatalog::enemies()[1].attack);
        QCOMPARE(enemy->biteProgress(),0.0);
        QVERIFY(!scene->findChildren<CombatEffect*>().isEmpty());
        QTest::qWait(70); play.gamePaused();
        auto bite=enemy->biteProgress();
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(play.grab().save(folder+"/bite.png"));
        QTest::qWait(350); QCOMPARE(enemy->biteProgress(),bite);
        play.gameContinued();
        enemy->getAttacked(10000);
        health=plant->getHp();
        enemy->startAttacking(plant); enemy->shootNote();
        QCOMPARE(plant->getHp(),health);
        QVERIFY(scene->findChildren<EnemyProjectile*>().isEmpty());
    }
    void guitarTimerFiresForwardWithoutTarget() {
        PlayScene play(8,nullptr,false);
        auto *scene=play.findChild<MyGameScene*>();
        scene->setAYellowDog(2,1);
        auto *enemy=static_cast<YellowDogs*>(scene->getZombieMap(2).front());
        auto *timer=enemy->findChild<QTimer*>("guitarRangedTimer");
        QVERIFY(timer); QVERIFY(timer->isActive());
        QCOMPARE(timer->interval(),GameCatalog::GuitarShotIntervalMs);
        QCOMPARE(timer->interval(),2000);
        QVERIFY(!scene->plantAhead(2,enemy->x()));
        timer->start(80);
        QTRY_COMPARE_WITH_TIMEOUT(scene->findChildren<EnemyProjectile*>().size(),1,300);
        timer->setInterval(GameCatalog::GuitarShotIntervalMs);
        auto *shot=scene->findChild<EnemyProjectile*>();
        const auto position=shot->pos();
        QTest::qWait(80);
        QVERIFY(shot->x()<position.x());
        QCOMPARE(shot->y(),position.y());
        play.gamePaused();
        const auto frozen=shot->pos();
        QTest::qWait(120);
        QCOMPARE(shot->pos(),frozen);
        QVERIFY(!timer->isActive());
        play.gameContinued();
        QVERIFY(timer->isActive());
    }
    void combatAnimationsAndRange() {
        PlayScene play(8,nullptr,false);
        play.show();
        auto *scene = play.findChild<MyGameScene*>();
        auto *view = play.findChild<QGraphicsView*>();
        scene->setChosenNum(1);
        Card::setGameState(GameState::PrePlace);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        auto *plant = scene->plantAhead(2,1000);
        QVERIFY(plant);
        scene->setAYellowDog(2,1);
        auto *enemy = static_cast<YellowDogs*>(scene->getZombieMap(2).front());
        enemy->stopMoving(); enemy->setPos(750,430);
        auto *rangedTimer=enemy->findChild<QTimer*>("guitarRangedTimer");
        QVERIFY(rangedTimer);
        rangedTimer->start(80);
        QTRY_COMPARE_WITH_TIMEOUT(scene->findChildren<EnemyProjectile*>().size(),1,300);
        rangedTimer->setInterval(GameCatalog::GuitarShotIntervalMs);
        auto shots = scene->findChildren<EnemyProjectile*>();
        QCOMPARE(shots.size(),1);
        QCOMPARE(shots.front()->damage(),GameCatalog::enemies()[1].attack/4);
        play.gamePaused();
        const auto pos = shots.front()->pos();
        QTest::qWait(150); QCOMPARE(shots.front()->pos(),pos);
        play.gameContinued();
        const int health = plant->getHp();
        QTRY_COMPARE_WITH_TIMEOUT(plant->getHp(),health-15,2000);
        enemy->getAttacked(30);
        QVERIFY(enemy->hitFlash()>0);
        enemy->getAttacked(10000);
        QVERIFY(enemy->isDying());
        QVERIFY(scene->getZombieMap(2).isEmpty());
        QSignalSpy removed(enemy,&YellowDogs::pleaseRemoveMe);
        play.gamePaused();
        const auto death = enemy->deathProgress();
        QTest::qWait(650);
        QCOMPARE(enemy->deathProgress(),death);
        QCOMPARE(removed.count(),0);
        play.gameContinued();
        QTRY_COMPARE_WITH_TIMEOUT(removed.count(),1,800);
    }
    void fullScreenNavigationAndInput() {
        QTemporaryDir dir;
        GameWindow menu(nullptr,unlockedPath(dir),false);
        menu.show(); QTest::qWait(40);
        QTest::keyClick(&menu,Qt::Key_F11);
        QTRY_VERIFY(menu.isFullScreen());

        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(menu.grab().save(folder+"/menu-fullscreen.png"));
        QTest::mouseClick(menu.homePage()->findChild<QPushButton*>("startGame"),Qt::LeftButton);
        QVERIFY(menu.levelPage()->isVisible());
        menu.startLevel(8);
        auto *play=menu.playPage();
        QTRY_VERIFY(menu.isFullScreen());
        auto *view=play->findChild<QGraphicsView*>();
        auto *scene=play->findChild<MyGameScene*>();
        menu.setFullScreenEnabled(false);
        for(const auto& size : {QSize(1920,1080),QSize(1280,720),QSize(2560,1440)}) {
            menu.resize(size); QTest::qWait(25);
            QCOMPARE(view->transform().m11(),play->canvasScale());
            const auto point=view->mapFromScene(QPointF(440,490));
            QVERIFY(view->viewport()->rect().contains(point));
            QVERIFY(QLineF(view->mapToScene(point),QPointF(440,490)).length()<2);
        }
        menu.resize(GameWindow::logicalSize());
        QTest::qWait(25);
        menu.setFullScreenEnabled(true);
        QTRY_VERIFY(menu.isFullScreen());
        QTest::qWait(50); // Allow the platform to deliver its final full-screen geometry.
        view->setFocus();
        QTest::keyClick(view,Qt::Key_H);
        auto cards=play->findChildren<Card*>();
        QTest::mouseClick(cards[1],Qt::LeftButton);
        QCOMPARE(Card::currentState(),GameState::PrePlace);
        const QPoint target=view->mapFromScene(QPointF(440,490));
        QTest::mouseMove(view->viewport(),target);
        QVERIFY(view->hasMouseTracking());
        QVERIFY(view->viewport()->hasMouseTracking());
        // Deliver the move through the viewport because the offscreen backend
        // does not reliably dispatch native cursor movement after mode changes.
        QMouseEvent movement(QEvent::MouseMove,QPointF(target),Qt::NoButton,Qt::NoButton,Qt::NoModifier);
        QApplication::sendEvent(view->viewport(),&movement);
        auto *preview=play->findChild<QLabel*>("plantPreview");
        QTRY_VERIFY(preview->isVisible());
        QVERIFY(QLineF(preview->geometry().center(),view->viewport()->mapTo(play,target)).length()<2);
        QCOMPARE(preview->width(),qRound(GameCatalog::plants()[1].iconSize*2*play->canvasScale()));
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,target);
        auto *plant=scene->plantAhead(2,1000);
        QVERIFY(plant); QVERIFY(plant->isHealthVisible());
        QCOMPARE(scene->getRestHeart(),50);
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+"/battle-fullscreen.png"));
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(1240,50)));
        QCOMPARE(Card::currentState(),GameState::Shoveling);
        QTest::mouseClick(view->viewport(),Qt::RightButton);
        QTest::keyClick(view,Qt::Key_Space); QTest::qWait(30);
        auto *pause=play->findChild<PauseDialog*>();
        QVERIFY(pause); QVERIFY(pause->isVisible());
        QTest::keyClick(pause,Qt::Key_H);
        QVERIFY(!scene->plantHealthVisible());
        QCOMPARE(Card::currentState(),GameState::Paused);
        QTest::keyClick(pause,Qt::Key_H);
        QVERIFY(scene->plantHealthVisible());
        QTest::keyClick(pause,Qt::Key_F11);
        QTRY_VERIFY(!menu.isFullScreen());
        QTRY_COMPARE(play->size(),GameWindow::logicalSize());
        QCOMPARE(Card::currentState(),GameState::Paused);
        QVERIFY(pause->isVisible());
        QTest::keyClick(pause,Qt::Key_Space);
        QCOMPARE(Card::currentState(),GameState::Normal);
        QTest::mouseClick(play->findChild<QPushButton*>("fullScreenButton"),Qt::LeftButton);
        QTRY_VERIFY(menu.isFullScreen());
        QTest::keyClick(view,Qt::Key_F11);
        QTRY_VERIFY(!menu.isFullScreen());
        const auto scale=view->transform().m11();
        QCOMPARE(scale,1.0);
        play->gamePaused(); play->mainMenuRequested();
        QTRY_VERIFY(menu.isVisible());
        QVERIFY(!menu.isFullScreen());
        QCOMPARE(menu.size(),GameWindow::logicalSize());
        QCOMPARE(menu.homePage()->findChild<QPushButton*>("startGame")->geometry(),QRect(610,430,430,84));
    }
    void nativeMaximizeAndFullScreenButton() {
        QTemporaryDir dir;
        GameWindow root(nullptr,unlockedPath(dir),false);
        root.show(); QTest::qWait(40);
        QVERIFY(root.windowFlags().testFlag(Qt::WindowMaximizeButtonHint));
        QVERIFY(root.maximumWidth()>root.width());
        auto *button=root.homePage()->findChild<QPushButton*>("fullScreenButton");
        QTest::mouseClick(button,Qt::LeftButton);
        QTRY_VERIFY(root.isFullScreen()); QTest::qWait(40);
        QTest::mouseClick(button,Qt::LeftButton);
        QTRY_VERIFY(!root.isFullScreen());
        QTRY_COMPARE(root.size(),GameWindow::logicalSize());
        root.resize(1200,680); root.move(80,70); QTest::qWait(30);
        const auto geometry=root.geometry();
        root.showMaximized(); QTRY_VERIFY(root.isMaximized());
        QTest::qWait(40);
        QCOMPARE(button->text(),QString("还原窗口 [F11]"));
        QTest::mouseClick(button,Qt::LeftButton);
        QTRY_VERIFY(!root.isMaximized());
        QTRY_COMPARE(root.geometry(),geometry);
        QTest::keyClick(&root,Qt::Key_F11);
        QTRY_VERIFY(root.isFullScreen()); QTest::qWait(40);
        QTest::keyClick(&root,Qt::Key_F11);
        QTRY_COMPARE(root.geometry(),geometry);
    }
    void levelButtonsAndCardArtworkScale() {
        QTemporaryDir dir;
        GameWindow root(nullptr,unlockedPath(dir),false);
        root.show(); root.showLevels();
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        for(const auto& size : {QSize(1650,900),QSize(1280,720),QSize(1920,1080),QSize(2560,1440)}) {
            root.resize(size); QTest::qWait(25);
            auto *page=root.levelPage();
            for(int level=1;level<=10;++level) {
                auto *button=page->findChild<QPushButton*>(QString("level%1").arg(level));
                QVERIFY(button); QVERIFY(button->icon().isNull());
                QCOMPARE(button->width(),button->height());
                QCOMPARE(button->width(),qRound(180*page->canvasScale()));
                QVERIFY(page->rect().contains(button->geometry()));
            }
            if(!folder.isEmpty()) QVERIFY(root.grab().save(folder+QString("/levels-%1.png").arg(size.width())));
        }
        QTest::mouseClick(root.levelPage()->findChild<QPushButton*>("level8"),Qt::LeftButton);
        QVERIFY(root.playPage()); QCOMPARE(root.playPage()->levelIndex,8);
        auto *play=root.playPage();
        const auto cards=play->findChildren<Card*>();
        root.resize(GameWindow::logicalSize()); QTest::qWait(25);
        const QSize base=cards[1]->size();
        QVERIFY(!play->findChild<QLabel*>("cardUnitIcon1"));
        QVERIFY(!play->findChild<QLabel*>("cardCost1"));
        for(const auto& size : {QSize(1280,720),QSize(1920,1080),QSize(2560,1440)}) {
            root.resize(size); QTest::qWait(25);
            const qreal scale=play->canvasScale();
            QCOMPARE(cards[1]->width(),qRound(base.width()*scale));
            QCOMPARE(cards[1]->height(),qRound(base.height()*scale));
            QCOMPARE(play->findChild<QGraphicsView*>()->transform().m11(),scale);
            if(!folder.isEmpty()) QVERIFY(root.grab().save(folder+QString("/cards-%1.png").arg(size.width())));
        }
        // Check frame pixels near the enlarged edge. A source-sized QIcon
        // centered in a larger button leaves these points without artwork.
        Card frame(1);
        frame.setFixedSize(base*2); frame.show(); QTest::qWait(20);
        const QImage actual=frame.grab().toImage();
        const QImage source=QImage(":/others/Image/card.png");
        int samples=0;
        for(const auto& fraction : {QPointF(.1,.15),QPointF(.9,.15),QPointF(.1,.5),QPointF(.9,.5),QPointF(.5,.9)}) {
            const auto expected=source.pixelColor(qRound((source.width()-1)*fraction.x()),qRound((source.height()-1)*fraction.y()));
            if(expected.alpha()<250) continue;
            const auto pixel=actual.pixelColor(qRound((actual.width()-1)*fraction.x()),qRound((actual.height()-1)*fraction.y()));
            QVERIFY(qAbs(pixel.red()-expected.red())<25);
            QVERIFY(qAbs(pixel.green()-expected.green())<25);
            QVERIFY(qAbs(pixel.blue()-expected.blue())<25);
            ++samples;
        }
        QVERIFY(samples>=3);
        QVERIFY(!play->findChild<QPushButton*>("backToLevels"));
        play->gamePaused(); root.showMenu(); root.showLevels();
        QVERIFY(root.playPage()==play); QVERIFY(root.levelPage()->isVisible());
        root.resize(GameWindow::logicalSize()); QTest::qWait(25);
        root.setFullScreenEnabled(true); QTest::qWait(50);
        if(!folder.isEmpty()) QVERIFY(root.grab().save(folder+"/levels-fullscreen.png"));
    }
    void returningStopsOldBattleAndResizingKeepsPreviewAligned() {
        QTemporaryDir dir;
        GameWindow root(nullptr,unlockedPath(dir),false);
        root.show(); root.startLevel(8); QTest::qWait(40);
        QPointer<PlayScene> old=root.playPage();
        auto *view=old->findChild<QGraphicsView*>();
        QSignalSpy moves(old->findChild<MyGameScene*>(),&MyGameScene::mouseMovedTo);
        QTest::mouseClick(old->findChildren<Card*>()[1],Qt::LeftButton);
        const QPointF scenePoint(440,490);
        auto point=view->mapFromScene(scenePoint);
        QMouseEvent movement(QEvent::MouseMove,QPointF(point),Qt::NoButton,Qt::NoButton,Qt::NoModifier);
        QApplication::sendEvent(view->viewport(),&movement);
        auto *preview=old->findChild<QLabel*>("plantPreview");
        QVERIFY(preview->isVisible());
        root.resize(1920,1080); QTest::qWait(30);
        QCOMPARE(preview->width(),qRound(GameCatalog::plants()[1].iconSize*2*old->canvasScale()));
        // The platform may send a cursor move after resizing. The preview
        // follows the most recent scene position in either case.
        point=view->mapFromScene(moves.last().first().toPointF());
        QVERIFY(QLineF(preview->geometry().center(),view->viewport()->mapTo(old,point)).length()<2);
        old->gameLose(); old->playSceneBack();
        QVERIFY(!root.playPage()); QVERIFY(root.levelPage()->isVisible());
        for(auto *timer : old->findChildren<QTimer*>()) QVERIFY(!timer->isActive());
        root.startLevel(3);
        auto *current=root.playPage();
        QVERIFY(current); QCOMPARE(current->levelIndex,3);
        old->gameWin(); old->playSceneBack(); old->mainMenuRequested();
        QCOMPARE(root.playPage(),current);
        QCOMPARE(Card::currentState(),GameState::Normal);
        QCOMPARE(ProgressStore(dir.filePath("progress.json")).resumeLevel(),3);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QVERIFY(old.isNull());
        QCOMPARE(root.currentPage(),static_cast<GamePage*>(current));
    }
    void closeExitsApplication_data() {
        QTest::addColumn<int>("page");
        QTest::newRow("home") << 0;
        QTest::newRow("levels") << 1;
        QTest::newRow("battle") << 2;
        QTest::newRow("paused-battle") << 3;
    }
    void closeExitsApplication() {
        QFETCH(int,page);
        QTemporaryDir dir;
        GameWindow root(nullptr,unlockedPath(dir),false);
        root.show();
        const auto nativeId=root.winId();
        if(page==1) root.showLevels();
        if(page>=2) root.startLevel(4);
        QCOMPARE(root.winId(),nativeId);
        QCOMPARE(root.currentPage()->window(),static_cast<QWidget*>(&root));
        if(page==3) {
            auto *view=root.playPage()->findChild<QGraphicsView*>();
            view->setFocus(); QTest::qWait(30);
            QTest::keyClick(view,Qt::Key_Space);
            QVERIFY(root.playPage()->findChild<PauseDialog*>()->isVisible());
        }
        QTimer watchdog;
        connect(&watchdog,&QTimer::timeout,qApp,[] { qApp->exit(99); });
        watchdog.setSingleShot(true); watchdog.start(2000);
        QSignalSpy closed(qApp,&QApplication::lastWindowClosed);
        QApplication::setQuitOnLastWindowClosed(true);
        QTimer::singleShot(20,&root,&QWidget::close);
        const int result=qApp->exec();
        QApplication::setQuitOnLastWindowClosed(false);
        QCOMPARE(result,0);
        QCOMPARE(closed.count(),1);
        QVERIFY(!root.isVisible());
        for(auto *dialog : root.findChildren<QDialog*>()) QVERIFY(!dialog->isVisible());
        if(root.playPage())
            for(auto *timer : root.playPage()->findChildren<QTimer*>()) QVERIFY(!timer->isActive());
        for(auto *sound : AudioManager::instance().findChildren<QSoundEffect*>()) QVERIFY(!sound->isPlaying());
    }
    void escapeOnlyLeavesFullScreen() {
        QTemporaryDir dir;
        GameWindow root(nullptr,unlockedPath(dir),false);
        root.show(); root.startLevel(8); QTest::qWait(40);
        auto *play=root.playPage();
        auto *view=play->findChild<QGraphicsView*>();
        auto *scene=play->findChild<MyGameScene*>();
        scene->toggleShovel();
        QTest::keyClick(view,Qt::Key_Escape);
        QCOMPARE(Card::currentState(),GameState::Shoveling);
        root.setFullScreenEnabled(true); QTest::qWait(40);
        QTest::keyClick(view,Qt::Key_Escape);
        QTRY_VERIFY(!root.isFullScreen());
        QCOMPARE(Card::currentState(),GameState::Shoveling);
        QTest::mouseClick(view->viewport(),Qt::RightButton);
        view->setFocus(); QTest::keyClick(view,Qt::Key_Space); QTest::qWait(30);
        auto *menu=play->findChild<PauseDialog*>();
        QVERIFY(menu && menu->isVisible());
        root.setFullScreenEnabled(true); QTest::qWait(40);
        QTest::keyClick(menu,Qt::Key_Escape);
        QTRY_VERIFY(!root.isFullScreen());
        QVERIFY(menu->isVisible()); QCOMPARE(Card::currentState(),GameState::Paused);
        QTest::keyClick(menu,Qt::Key_Escape);
        QVERIFY(menu->isVisible());
        QTimer::singleShot(50,menu,[menu] {
            auto *dialog=menu->findChild<AlmanacDialog*>();
            QVERIFY(dialog); QTest::keyClick(dialog,Qt::Key_Escape);
            QVERIFY(dialog->isVisible()); dialog->reject();
        });
        QTest::mouseClick(menu->findChild<QPushButton*>("almanac"),Qt::LeftButton);
        QVERIFY(menu->isVisible()); QCOMPARE(Card::currentState(),GameState::Paused);
    }
    void earlyLanesRestrictPlanting_data() {
        QTest::addColumn<int>("level"); QTest::addColumn<int>("first"); QTest::addColumn<int>("last");
        QTest::newRow("one-lane") << 1 << 2 << 2;
        QTest::newRow("three-lanes") << 2 << 1 << 3;
        QTest::newRow("five-lanes") << 3 << 0 << 4;
    }
    void earlyLanesRestrictPlanting() {
        QFETCH(int,level); QFETCH(int,first); QFETCH(int,last);
        PlayScene play(level,nullptr,false); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *view=play.findChild<QGraphicsView*>();
        scene->setChosenNum(0);
        for(int row=0;row<5;++row) {
            scene->addHeart(100); const int hearts=scene->getRestHeart();
            const int before=scene->findChildren<WhiteDogs*>().size();
            Card::setGameState(GameState::PrePlace);
            QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,202+row*145)));
            const bool allowed=row>=first && row<=last;
            QCOMPARE(scene->findChildren<WhiteDogs*>().size(),before+int(allowed));
            QCOMPARE(scene->getRestHeart(),hearts-(allowed ? 100 : 0));
        }
        auto *lawn=scene->lawn();
        lawn->setRevealProgress(0);
        if(level==1) QCOMPARE(lawn->rowReveal(2),0.0);
        if(level==2) { QCOMPARE(lawn->rowReveal(2),1.0); QCOMPARE(lawn->rowReveal(1),0.0); }
        if(level==3) {
            for(int row=1;row<=3;++row) QCOMPARE(lawn->rowReveal(row),1.0);
            QCOMPARE(lawn->rowReveal(0),0.0); QCOMPARE(lawn->rowReveal(4),0.0);
        }
        lawn->setRevealProgress(.5);
        QCOMPARE(lawn->rowReveal(level==1 ? 2 : level==2 ? 1 : 0),.5);
        lawn->setRevealProgress(1);
        for(int row=0;row<5;++row) QCOMPARE(lawn->rowReveal(row),row>=first && row<=last ? 1.0 : 0.0);
        play.gamePaused();
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(play.grab().save(folder+QString("/lawn-level%1.png").arg(level)));
    }
    void openingSequence_data() {
        QTest::addColumn<int>("level");
        for(int level : {1,2,3,10}) QTest::newRow(qPrintable(QString::number(level))) << level;
    }
    void openingSequence() {
        QFETCH(int,level);
        QTemporaryDir dir;
        GameWindow root(nullptr,unlockedPath(dir));
        root.show(); root.startLevel(level); QTest::qWait(30);
        auto *play=root.playPage(); auto *scene=play->findChild<MyGameScene*>();
        auto *view=play->findChild<QGraphicsView*>(); auto *opening=play->findChild<LevelOpening*>();
        auto *timeline=opening->findChild<QVariantAnimation*>("openingTimeline");
        QVERIFY(!scene->gameplayStarted()); QCOMPARE(scene->wavesStarted(),0);
        for(auto *timer : scene->findChildren<QTimer*>()) QVERIFY(!timer->isActive());
        for(auto *card : play->findChildren<Card*>()) QVERIFY(!card->findChild<QTimer*>()->isActive());
        scene->setAYellowDog(2); QVERIFY(scene->findChildren<YellowDogs*>().isEmpty());
        timeline->setCurrentTime(timeline->duration());
        QCOMPARE(opening->stage(),LevelOpening::Stage::Preview);
        int decorative=0;
        for(auto *item : scene->items()) {
            QVERIFY(!dynamic_cast<QGraphicsTextItem*>(item));
            QVERIFY(!dynamic_cast<QGraphicsRectItem*>(item));
            if(item->data(0).toString()=="enemyPreview") {
                ++decorative; QVERIFY(item->pos().x()>=1580 && item->pos().x()<1930);
            }
        }
        QCOMPARE(decorative,GameCatalog::level(level).maxEnemyType==0 ? 5 : 12);
        QVERIFY(view->mapToScene(view->viewport()->rect().center()).x()>1200);
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+QString("/preview-level%1.png").arg(level)));
        timeline->setCurrentTime(timeline->duration());
        QCOMPARE(opening->stage(),LevelOpening::Stage::PanLeft);
        timeline->setCurrentTime(timeline->duration());
        QCOMPARE(opening->stage(),LevelOpening::Stage::Reveal);
        if(level==3) for(int row=1;row<=3;++row) QCOMPARE(scene->lawn()->rowReveal(row),1.0);
        timeline->setCurrentTime(timeline->duration()/2);
        if(level==3) {
            for(int row=1;row<=3;++row) QCOMPARE(scene->lawn()->rowReveal(row),1.0);
            QCOMPARE(scene->lawn()->rowReveal(0),.5); QCOMPARE(scene->lawn()->rowReveal(4),.5);
        }
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+QString("/unroll-level%1.png").arg(level)));
        root.resize(1920,1080); QTest::qWait(20);
        QCOMPARE(view->transform().m11(),play->canvasScale());
        timeline->setCurrentTime(timeline->duration());
        QCOMPARE(opening->stage(),LevelOpening::Stage::Ready);
        QVERIFY(!scene->gameplayStarted());
        auto *banner=play->findChild<BattleBanner*>(); QVERIFY(banner->isVisible());
        auto *impact=banner->findChild<QVariantAnimation*>("bannerAnimation");
        impact->setCurrentTime(450);
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+QString("/ready-level%1.png").arg(level)));
        impact->setCurrentTime(impact->duration());
        if(level<=2) {
            QVERIFY(!scene->gameplayStarted());
            QVERIFY(play->findChild<LevelTutorial*>()->isVisible());
            return;
        }
        QVERIFY(scene->gameplayStarted()); QCOMPARE(scene->wavesStarted(),0);
        QVERIFY(scene->findChild<QTimer*>("waveTimer")->isActive());
        QVERIFY(scene->findChild<QTimer*>("waveTimer")->remainingTime()>GameCatalog::level(level).initialDelayMs-100);
    }
    void previewUsesSharedProbabilities() {
        QRandomGenerator random(10);
        for(int level=1;level<=10;++level) {
            const auto types=WavePlanner::previewTypes(level,random);
            const int max=GameCatalog::level(level).maxEnemyType;
            for(int type : types) QVERIFY(type>=0 && type<=max);
            if(max==0) { QCOMPARE(types.size(),5); QCOMPARE(types.count(0),5); }
            if(max==1) { QCOMPARE(types.count(0),10); QCOMPARE(types.count(1),2); }
            if(max==2) { QCOMPARE(types.count(0),9); QCOMPARE(types.count(1),2); QCOMPARE(types.count(2),1); }
        }
    }
    void finalWaveWarnsBeforeSpawning() {
        PlayScene play(4,nullptr,false); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *wave=scene->findChild<QTimer*>("waveTimer");
        auto *stagger=scene->findChild<QTimer*>("waveStaggerTimer");
        QSignalSpy warning(scene,&MyGameScene::finalWaveApproaching);
        for(int i=1;i<GameCatalog::level(4).waves;++i) {
            QMetaObject::invokeMethod(wave,"timeout");
            while(stagger->isActive()) QMetaObject::invokeMethod(stagger,"timeout");
        }
        QCOMPARE(warning.count(),0);
        const int before=scene->findChildren<YellowDogs*>().size();
        QMetaObject::invokeMethod(wave,"timeout");
        QCOMPARE(warning.count(),1); QCOMPARE(scene->findChildren<YellowDogs*>().size(),before);
        QCOMPARE(stagger->interval(),1800);
        auto *banner=play.findChild<BattleBanner*>(); QVERIFY(banner->isVisible());
        auto *impact=banner->findChild<QVariantAnimation*>("bannerAnimation"); impact->setCurrentTime(450);
        play.gamePaused(); const int time=impact->currentTime(); QTest::qWait(80);
        QCOMPARE(impact->currentTime(),time); QVERIFY(!stagger->isActive());
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(play.grab().save(folder+"/final-wave.png"));
        play.gameContinued();
        QTRY_VERIFY_WITH_TIMEOUT(scene->findChildren<YellowDogs*>().size()>before,2000);
        QMetaObject::invokeMethod(wave,"timeout"); QCOMPARE(warning.count(),1);
    }
    void firstLevelInteractiveTutorial() {
        QTemporaryDir dir;
        GameWindow root(nullptr,unlockedPath(dir)); root.show(); root.startLevel(1);
        auto *play=root.playPage(); completeOpening(play); QTest::qWait(30);
        auto *scene=play->findChild<MyGameScene*>(); auto *view=play->findChild<QGraphicsView*>();
        auto *tutorial=play->findChild<LevelTutorial*>(); auto *card=play->findChild<Card*>();
        QCOMPARE(tutorial->step(),LevelTutorial::Step::Plant); QVERIFY(!scene->gameplayStarted());
        QCOMPARE(scene->getRestHeart(),100); QVERIFY(card->isEnabled());
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+"/tutorial-plant.png"));
        QTest::mouseClick(card,Qt::LeftButton);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,200)));
        QVERIFY(scene->findChildren<WhiteDogs*>().isEmpty()); QCOMPARE(scene->getRestHeart(),100);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        QCOMPARE(tutorial->step(),LevelTutorial::Step::Heart); QCOMPARE(scene->getRestHeart(),0);
        QVERIFY(card->isCooling()); QVERIFY(!card->findChild<QTimer*>()->isActive());
        auto *heart=scene->findChild<Heart*>("tutorialHeart"); QVERIFY(heart);
        QVERIFY(!heart->findChild<QTimer*>()->isActive());
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+"/tutorial-heart.png"));
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(heart->sceneBoundingRect().center()));
        QTRY_COMPARE_WITH_TIMEOUT(tutorial->step(),LevelTutorial::Step::Controls,1200);
        QCOMPARE(scene->getRestHeart(),25); QVERIFY(!scene->gameplayStarted());
        auto *next=tutorial->findChild<QPushButton*>("finishTutorial"); QVERIFY(!next->isEnabled());
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+"/tutorial-controls.png"));
        view->setFocus(); QTest::qWait(30); QTest::keyClick(view,Qt::Key_Space); QTest::qWait(30);
        auto *pause=play->findChild<PauseDialog*>(); QVERIFY(pause && pause->isVisible());
        QTimer::singleShot(60,pause,[pause] { auto *dialog=pause->findChild<AlmanacDialog*>(); QVERIFY(dialog); dialog->reject(); });
        QTest::mouseClick(pause->findChild<QPushButton*>("almanac"),Qt::LeftButton);
        QVERIFY(next->isEnabled()); QVERIFY(!scene->gameplayStarted());
        QTest::keyClick(pause,Qt::Key_Space);
        QTest::mouseClick(next,Qt::LeftButton);
        QVERIFY(scene->gameplayStarted()); QVERIFY(!tutorial->isVisible());
        QCOMPARE(scene->findChildren<WhiteDogs*>().size(),1);
        QCOMPARE(scene->wavesStarted(),0);
        QVERIFY(scene->findChild<QTimer*>("waveTimer")->remainingTime()>14500);
        QVERIFY(card->findChild<QTimer*>()->isActive());
    }
    void secondLevelRequiresThreeShovelRemovals() {
        QTemporaryDir dir;
        GameWindow root(nullptr,unlockedPath(dir)); root.show(); root.startLevel(2);
        auto *play=root.playPage(); completeOpening(play); QTest::qWait(30);
        auto *scene=play->findChild<MyGameScene*>(); auto *view=play->findChild<QGraphicsView*>();
        auto *tutorial=play->findChild<LevelTutorial*>();
        QCOMPARE(tutorial->step(),LevelTutorial::Step::Shovel); QCOMPARE(scene->getRestHeart(),50);
        QCOMPARE(scene->findChildren<WhiteDogs*>().size(),3); QVERIFY(!scene->gameplayStarted());
        for(auto *plant : scene->findChildren<WhiteDogs*>()) QVERIFY(scene->lawn()->zValue()<plant->zValue());
        for(auto *timer : scene->findChildren<QTimer*>()) QVERIFY(!timer->isActive());
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+"/tutorial-shovel.png"));
        root.setFullScreenEnabled(true); QTest::qWait(40);
        QTest::keyClick(view,Qt::Key_Escape); QTest::qWait(40);
        QCOMPARE(tutorial->step(),LevelTutorial::Step::Shovel);
        for(int row=1;row<=3;++row) {
            QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(1240,50)));
            QCOMPARE(Card::currentState(),GameState::Shoveling);
            QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(682,202+row*145)));
            QCOMPARE(scene->getRestHeart(),50);
            if(row<3) { QVERIFY(!scene->gameplayStarted()); QCOMPARE(tutorial->step(),LevelTutorial::Step::Shovel); }
        }
        QVERIFY(scene->gameplayStarted()); QCOMPARE(tutorial->step(),LevelTutorial::Step::Done);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QVERIFY(scene->findChildren<WhiteDogs*>().isEmpty());
        QCOMPARE(scene->wavesStarted(),0); QCOMPARE(Card::currentState(),GameState::Normal);
    }
    void closingDuringOpeningAndTutorialStopsActivity() {
        QTemporaryDir dir;
        GameWindow root(nullptr,unlockedPath(dir)); root.show(); root.startLevel(2);
        QPointer<PlayScene> old=root.playPage();
        old->gameLose(); old->playSceneBack();
        for(auto *animation : old->findChildren<QAbstractAnimation*>()) QVERIFY(animation->state()!=QAbstractAnimation::Running);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete); QVERIFY(old.isNull());
        root.startLevel(1); completeOpening(root.playPage());
        root.close();
        QVERIFY(!root.isVisible()); QVERIFY(!root.playPage()->findChild<LevelTutorial*>()->isVisible());
        for(auto *timer : root.playPage()->findChildren<QTimer*>()) QVERIFY(!timer->isActive());
    }
    void openingAdvancesAutomaticallyInRealTime() {
        QTemporaryDir dir;
        GameWindow root(nullptr,unlockedPath(dir)); root.show(); root.startLevel(3);
        auto *play=root.playPage(); auto *scene=play->findChild<MyGameScene*>();
        QVERIFY(!scene->gameplayStarted());
        QTRY_VERIFY_WITH_TIMEOUT(scene->gameplayStarted(),10000);
        QCOMPARE(play->findChild<LevelOpening*>()->stage(),LevelOpening::Stage::Complete);
        for(int row=0;row<5;++row) QCOMPARE(scene->lawn()->rowReveal(row),1.0);
        QCOMPARE(scene->wavesStarted(),0);
    }
    void enemyLimit() {
        MyGameScene scene(1);
        for(int i=0;i<8;++i) scene.setAYellowDog(2);
        QCOMPARE(scene.getZombieMap(2).size(),3);
        QSignalSpy win(&scene,&MyGameScene::gameWin);
        const auto enemies = scene.getZombieMap(2);
        for(auto* enemy: enemies) static_cast<YellowDogs*>(enemy)->getAttacked(10000);
        QTRY_COMPARE_WITH_TIMEOUT(win.count(),1,1000);
        QVERIFY(scene.getZombieMap(2).isEmpty());
    }
};
QTEST_MAIN(GameTests)
#include "test_game.moc"
