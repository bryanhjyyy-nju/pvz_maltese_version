#include <QtTest>
#include <QElapsedTimer>
#include <QPaintEvent>
#include <QSettings>
#include <QTemporaryDir>
#include <QEventLoop>
#include <algorithm>
#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif
#include "audiomanager.h"
#include "card.h"
#include "gamecatalog.h"
#include "playscene.h"
#include "yellowdogs.h"

class PaintProbe : public QObject {
public:
    qint64 pixels=0;
    int paints=0;
    QElapsedTimer clock;
    QVector<qint64> intervals;
    qint64 lastPaint=0;
    bool eventFilter(QObject*,QEvent *event) override {
        if(event->type()==QEvent::Paint) {
            ++paints;
            if(clock.isValid()) {
                const auto now=clock.nsecsElapsed();
                if(lastPaint) intervals.append(now-lastPaint);
                lastPaint=now;
            }
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
    void crowdedBattlefield_data() {
        QTest::addColumn<QSize>("size");
        QTest::newRow("1280x720") << QSize(1280,720);
        QTest::newRow("1920x1080") << QSize(1920,1080);
    }
    void crowdedBattlefield() {
        QFETCH(QSize,size);
        PlayScene play(10,nullptr,false,true); play.resize(size); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *view=play.findChild<QGraphicsView*>();
        for(int row=0;row<5;++row) for(int col=0;col<9;++col) scene->addTutorialPlant(row,col);
        for(int i=0;i<48;++i) {
            scene->setAYellowDog(i%5,i%3);
            auto *enemy=static_cast<YellowDogs*>(scene->getZombieMap(i%5).back());
            enemy->setPos(950+(i%8)*70,enemy->y());
        }
        for(int i=0;i<40;++i) scene->generateBullet(i%5,i%5);
        scene->togglePlantHealth(); scene->toggleEnemyHealth();
        // Freeze combat decisions, keeping sprite clocks and movement active.
        for(auto *timer : scene->findChildren<QTimer*>()) timer->stop();
        QEventLoop warmup; QTimer::singleShot(400,&warmup,&QEventLoop::quit); warmup.exec();
        PaintProbe probe; probe.clock.start(); view->viewport()->installEventFilter(&probe);
        QEventLoop sample; QTimer::singleShot(3000,&sample,&QEventLoop::quit); sample.exec();
        const auto elapsed=probe.clock.elapsed(); view->viewport()->removeEventFilter(&probe);
#ifdef Q_OS_WIN
        PROCESS_MEMORY_COUNTERS memory={}; memory.cb=sizeof(memory);
        if(GetProcessMemoryInfo(GetCurrentProcess(),&memory,sizeof(memory)))
            qInfo("BENCH crowded_%dx%d working_set_mib=%.1f",size.width(),size.height(),memory.WorkingSetSize/1048576.0);
#endif
        QVERIFY(probe.paints>0); std::sort(probe.intervals.begin(),probe.intervals.end());
        const qreal p95=probe.intervals.isEmpty() ? 0 : probe.intervals[int((probe.intervals.size()-1)*.95)]/1000000.0;
        qInfo("BENCH crowded_%dx%d paints_per_second=%.1f interval_p95_ms=%.1f",size.width(),size.height(),probe.paints*1000.0/elapsed,p95);
        GamePause pause; pause.pause(&play);
        QImage image(view->viewport()->size(),QImage::Format_ARGB32_Premultiplied); image.fill(Qt::transparent);
        QPainter painter(&image); view->viewport()->render(&painter);
        QVector<qint64> paintTimes;
        for(int sample=0;sample<5;++sample) {
            QElapsedTimer paintTime; paintTime.start();
            for(int frame=0;frame<120;++frame) view->viewport()->render(&painter);
            paintTimes.append(paintTime.elapsed());
        }
        std::sort(paintTimes.begin(),paintTimes.end());
        qInfo("BENCH crowded_%dx%d median_full_120_paints_ms=%lld",size.width(),size.height(),paintTimes[2]);
        play.shutdown();
    }
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
            enemy->setHealthVisible(true); enemy->findChild<SpriteAnimation*>()->setPaused(true); enemies.append(enemy);
        }
        QElapsedTimer elapsed; elapsed.start();
        for(int i=0;i<80;++i) for(auto *enemy : enemies) {
            auto *movie=enemy->findChild<SpriteAnimation*>();
            QVERIFY(movie->jumpToFrame(i%qMax(1,movie->frameCount())));
        }
        qInfo("BENCH sprite_2560_updates_ms=%.3f",elapsed.nsecsElapsed()/1000000.0);
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
