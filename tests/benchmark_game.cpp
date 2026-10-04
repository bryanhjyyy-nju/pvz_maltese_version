#include <QtTest>
#include <QElapsedTimer>
#include <QPaintEvent>
#include <QSettings>
#include <QTemporaryDir>
#include "audiomanager.h"
#include "card.h"
#include "gamecatalog.h"
#include "playscene.h"
#include "yellowdogs.h"

class PaintProbe : public QObject {
public:
    qint64 pixels=0;
    int paints=0;
    bool eventFilter(QObject*,QEvent *event) override {
        if(event->type()==QEvent::Paint) {
            ++paints;
            for(const auto& rect : static_cast<QPaintEvent*>(event)->region())
                pixels+=qint64(rect.width())*rect.height();
        }
        return false;
    }
};

// Fixed workloads report measurements, rather than asserting machine-specific timings.
class GameBenchmarks : public QObject {
    Q_OBJECT
    QTemporaryDir settings;
private slots:
    void initTestCase() {
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settings.path());
        QCoreApplication::setOrganizationName("PvZBenchmarks");
        QCoreApplication::setApplicationName("Performance");
        AudioManager::instance().setMusicVolume(0);
        AudioManager::instance().setEffectsVolume(0);
        QApplication::setQuitOnLastWindowClosed(false);
    }
    void cardRepaints() {
        QWidget parent; QVector<Card*> cards;
        for(int i=0;i<8;++i) {
            auto *card=new Card(i); card->setParent(&parent);
            card->heartCost=GameCatalog::plants()[i].cost;
            card->coolTime=GameCatalog::plants()[i].cooldownMs;
            card->startCooldown(); cards.append(card);
        }
        QImage image(200,250,QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent); QPainter painter(&image);
        for(auto *card : cards) card->render(&painter);
        QElapsedTimer elapsed; elapsed.start();
        for(int i=0;i<200;++i) for(auto *card : cards) card->render(&painter);
        qInfo("BENCH card_1600_paints_ms=%lld",elapsed.elapsed());
    }
    void spriteFrameUpdates() {
        MyGameScene scene(10,nullptr,true); QVector<YellowDogs*> enemies;
        for(int i=0;i<32;++i) {
            auto *enemy=new YellowDogs(i%5,&scene,0); enemy->setParent(&scene);
            enemy->setHealthVisible(true); enemy->findChild<QMovie*>()->setPaused(true); enemies.append(enemy);
        }
        QElapsedTimer elapsed; elapsed.start();
        for(int i=0;i<80;++i) for(auto *enemy : enemies) {
            auto *movie=enemy->findChild<QMovie*>();
            QVERIFY(movie->jumpToFrame(i%qMax(1,movie->frameCount())));
        }
        qInfo("BENCH sprite_2560_updates_ms=%lld",elapsed.elapsed());
    }
    void battlefieldDirtyArea() {
        PlayScene play(10,nullptr,false); play.resize(1650,900); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *view=play.findChild<QGraphicsView*>();
        scene->getGameTimer()->stop();
        for(int i=0;i<32;++i) {
            scene->setAYellowDog(i%5);
            auto *enemy=static_cast<YellowDogs*>(scene->getZombieMap(i%5).back());
            enemy->setPos(850+(i%8)*75,enemy->y());
        }
        scene->toggleEnemyHealth(); QTest::qWait(100);
        PaintProbe probe; view->viewport()->installEventFilter(&probe);
        QTest::qWait(1000); view->viewport()->removeEventFilter(&probe);
        QVERIFY(probe.paints>0);
        const double fraction=double(probe.pixels)/(qint64(view->viewport()->width())*view->viewport()->height()*probe.paints);
        qInfo("BENCH viewport_paints=%d average_repaint_fraction=%.4f",probe.paints,fraction);
        play.shutdown();
    }
};
QTEST_MAIN(GameBenchmarks)
#include "benchmark_game.moc"
