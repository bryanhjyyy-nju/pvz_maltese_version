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
        QCOMPARE(sounds.size(),17);
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
        QCOMPARE(scene->getRestHeart(),0);
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
        auto click = [&](const QPointF& pos) {
            QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(pos));
        };
        click(QPointF(1240,50));
        QCOMPARE(Card::currentState(),GameState::Shoveling);
        click(QPointF(1240,50));
        QCOMPARE(Card::currentState(),GameState::Normal);
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
    void allLevelsFinish_data() {
        QTest::addColumn<int>("level");
        for(int i=1;i<=10;++i) QTest::newRow(qPrintable(QString::number(i))) << i;
    }
    void allLevelsFinish() {
        QFETCH(int,level);
        MyGameScene scene(level);
        QSignalSpy wins(&scene,&MyGameScene::gameWin);
        const auto& config = GameCatalog::level(level);
        for(int i=0;i<config.enemies;++i) scene.setAYellowDog(i%5,i%3);
        for(int row=0;row<5;++row) {
            const auto enemies = scene.getZombieMap(row);
            for(auto *enemy : enemies) {
                auto *target = static_cast<YellowDogs*>(enemy);
                target->getAttacked(10000);
                target->getAttacked(10000); // Duplicate damage must not count another kill.
            }
        }
        QCOMPARE(wins.count(),1);
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
    void enemyLimit() {
        MyGameScene scene(1);
        for(int i=0;i<8;++i) scene.setAYellowDog(2);
        QCOMPARE(scene.getZombieMap(2).size(),3);
        QSignalSpy win(&scene,&MyGameScene::gameWin);
        const auto enemies = scene.getZombieMap(2);
        for(auto* enemy: enemies) static_cast<YellowDogs*>(enemy)->getAttacked(10000);
        QCOMPARE(win.count(),1);
        QVERIFY(scene.getZombieMap(2).isEmpty());
    }
};
QTEST_MAIN(GameTests)
#include "test_game.moc"
