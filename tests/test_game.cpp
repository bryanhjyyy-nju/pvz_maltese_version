#include <QtTest>
#include "audiomanager.h"
#include <QSettings>
#include "map.h"
#include "progressstore.h"
#include <QTemporaryDir>
#include <QFile>
#include "gamecatalog.h"
#include "mygamescene.h"
#include "singingwhite.h"
#include "yellowdogs.h"

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
        QCOMPARE(sounds.size(),10);
        for(auto *sound : sounds) {
            QTRY_VERIFY_WITH_TIMEOUT(sound->status() != QSoundEffect::Loading,5000);
            QCOMPARE(sound->status(),QSoundEffect::Ready);
        }
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
