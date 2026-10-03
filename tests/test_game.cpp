#include <QtTest>
#include "audiomanager.h"
#include <QSettings>
#include "playscene.h"
#include "almanacdialog.h"
#include "mainscene.h"
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

class GameTests : public QObject {
    Q_OBJECT
    QTemporaryDir settingsDirectory;
private slots:
    void initTestCase() {
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
        QCOMPARE(sounds.size(),18);
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
        PlayScene play(8);
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
        ChooseLevelScene picker(nullptr,dir.filePath("progress.json"));
        QSignalSpy home(&picker,&ChooseLevelScene::chooseSceneBack);
        picker.startLevel(4);
        auto *view = picker.play->findChild<QGraphicsView*>();
        view->setFocus();
        QTest::qWait(50);
        QTest::keyClick(view,Qt::Key_Space);
        auto *menu = picker.play->findChild<PauseDialog*>();
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
        QTest::keyClick(view,Qt::Key_Space);
        QTest::mouseClick(menu->findChild<QPushButton*>("mainMenu"),Qt::LeftButton);
        QCOMPARE(home.count(),1);
        QVERIFY(!picker.play);
        QCOMPARE(ProgressStore(dir.filePath("progress.json")).resumeLevel(),4);
    }
    void mouseShovel() {
        PlayScene play(8);
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
        QTest::keyClick(view,Qt::Key_Escape);
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
        PlayScene play(8);
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
        const auto path = dir.filePath("progress.json");
        ChooseLevelScene picker(nullptr,path);
        picker.startLevel(1);
        QVERIFY(picker.play);
        QCOMPARE(ProgressStore(path).resumeLevel(),1);
        picker.play->gameWin();
        QCOMPARE(ProgressStore(path).resumeLevel(),2);
        picker.play->playSceneBack();
        QVERIFY(!picker.play);
        picker.continueGame();
        QVERIFY(picker.play);
        QCOMPARE(picker.play->levelIndex,2);
        picker.play->gameLose();
        picker.play->close();
        QVERIFY(!picker.play);
        QCOMPARE(ProgressStore(path).resumeLevel(),2);
        picker.continueGame();
        QCOMPARE(picker.play->levelIndex,2);
        picker.play->close();
    }
    void cartoonMenuButtons() {
        MainScene menu;
        menu.show();
        QCOMPARE(menu.windowTitle(),QString("小白大战小金毛"));
        auto *start=menu.findChild<QPushButton*>("startGame");
        auto *quit=menu.findChild<QPushButton*>("quitGame");
        QVERIFY(start); QVERIFY(quit);
        QCOMPARE(start->text(),QString("开始游戏"));
        QCOMPARE(quit->text(),QString("退出游戏"));
        QVERIFY(start->icon().isNull()); QVERIFY(quit->icon().isNull());
        QCOMPARE(start->property("color").toString(),QString("sunshine"));
        QTest::mouseClick(start,Qt::LeftButton);
        QVERIFY(!menu.isVisible()); QVERIFY(menu.chooseScene->isVisible());
        menu.chooseScene->chooseSceneBack();
        QVERIFY(menu.isVisible());
        QTest::mouseClick(quit,Qt::LeftButton);
        QVERIFY(!menu.isVisible());
    }
    void progressRoundTrip() {
        QTemporaryDir dir;
        const auto path = dir.filePath("save/progress.json");
        ProgressStore store(path);
        QVERIFY(!store.hasProgress());
        QVERIFY(store.startLevel(3));
        QCOMPARE(ProgressStore(path).resumeLevel(),3);
        QVERIFY(store.completeLevel(3));
        QCOMPARE(ProgressStore(path).resumeLevel(),4);
        QVERIFY(store.completeLevel(10));
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
        QVERIFY(broken.startLevel(2));
        QCOMPARE(ProgressStore(path).resumeLevel(),2);
    }
    void progressWriteFailure() {
        QTemporaryDir dir;
        ProgressStore store(dir.path()); // A directory cannot be replaced by a file.
        QVERIFY(!store.startLevel(2));
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
        PlayScene play(1);
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
        PlayScene play(8);
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
        PlayScene play(8);
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
        PlayScene play(8);
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
