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
#include "dblsingwhite.h"
#include "allheartwhite.h"
#include "heartwhite.h"
#include "wallwhite.h"
#include "linewhite.h"
#include "dancingwhite.h"
#include "moneywhite.h"
#include <QElapsedTimer>
#include "yellowdogs.h"
#include "pausedialog.h"
#include <QPushButton>
#include "enemyprojectile.h"
#include "waveplanner.h"
#include "combateffect.h"
#include "gameartwork.h"
#include <QRandomGenerator>
#include <QFontInfo>
#include <QMovie>
#include <QImageReader>
#include <QScreen>
#include "lawn.h"
#include "levelopening.h"
#include "battlebanner.h"
#include "leveltutorial.h"
#include "heart.h"
#include "bullet.h"
#include "battleresult.h"
#include "battlesnapshot.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>

class GameTests : public QObject {
    Q_OBJECT
    QTemporaryDir settingsDirectory;
    void verifyGameTerms(QWidget *root) {
        auto widgets=root->findChildren<QWidget*>(); widgets.prepend(root);
        for(auto *widget : widgets) {
            QStringList texts{widget->windowTitle(),widget->toolTip(),widget->whatsThis(),widget->accessibleName(),widget->accessibleDescription()};
            if(auto *label=qobject_cast<QLabel*>(widget)) texts.append(label->text());
            if(auto *button=qobject_cast<QAbstractButton*>(widget)) texts.append(button->text());
            if(auto *tabs=qobject_cast<QTabWidget*>(widget))
                for(int i=0;i<tabs->count();++i) texts.append(tabs->tabText(i));
            for(const auto& text : texts) {
                QVERIFY2(!text.contains("植物"),qPrintable(text));
                QVERIFY2(!text.contains("僵尸"),qPrintable(text));
            }
        }
    }
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
    void speedTimerPreservesRemainingTimeAndPause() {
        QObject root; GameSpeed clock(&root);
        GameTimer timer(&root,&clock),idle(&root,&clock);
        timer.start(400); idle.setInterval(500);
        QSignalSpy ticks(&timer,&QTimer::timeout);
        QTest::qWait(100); const int remaining=timer.remainingTime();
        clock.setMultiplier(2);
        QVERIFY(qAbs(timer.remainingTime()-remaining/2)<12);
        QCOMPARE(timer.gameInterval(),400); QCOMPARE(idle.interval(),250); QVERIFY(!idle.isActive());
        const int fasterRemaining=timer.remainingTime();
        for(int i=0;i<8;++i) { clock.setMultiplier(1); clock.setMultiplier(2); }
        QVERIFY(qAbs(timer.remainingTime()-fasterRemaining)<15);
        GamePause pause; pause.pause(&root); QVERIFY(!timer.isActive());
        clock.setMultiplier(1); QTest::qWait(100); QCOMPARE(ticks.count(),0);
        pause.resume(); QVERIFY(qAbs(timer.remainingTime()-fasterRemaining*2)<30);
        QTRY_COMPARE_WITH_TIMEOUT(ticks.count(),1,500); QCOMPARE(timer.interval(),400);
        // A gameplay callback can replace an interval after a partial tick.
        clock.setMultiplier(2);
        connect(&timer,&QTimer::timeout,&root,[&] { timer.setInterval(900); });
        QTRY_COMPARE_WITH_TIMEOUT(ticks.count(),2,300);
        QCOMPARE(timer.gameInterval(),900); QCOMPARE(timer.interval(),450);
        timer.stop(); clock.setMultiplier(1); QVERIFY(!timer.isActive()); QCOMPARE(timer.interval(),900);
        clock.setMultiplier(2);
        GameTimer single(&root,&clock); single.setSingleShot(true); single.start(160);
        QSignalSpy once(&single,&QTimer::timeout);
        QTRY_COMPARE_WITH_TIMEOUT(once.count(),1,200); QVERIFY(!single.isActive());
        QTest::qWait(100); QCOMPARE(once.count(),1);
    }
    void speedAnimationPreservesPositionLoopsAndPausedState() {
        GameSpeed clock;
        GameVariantAnimation animation(&clock); animation.setDuration(1000); animation.setLoopCount(3);
        animation.setStartValue(0.0); animation.setEndValue(100.0);
        QSignalSpy finished(&animation,&QAbstractAnimation::finished);
        animation.start(); animation.setCurrentTime(2700);
        clock.setMultiplier(2);
        QCOMPARE(animation.duration(),500); QCOMPARE(animation.currentTime(),1350);
        QCOMPARE(animation.currentLoop(),2); QCOMPARE(animation.currentValue().toDouble(),70.0);
        QCOMPARE(animation.state(),QAbstractAnimation::Running); QCOMPARE(finished.count(),0);
        animation.pause(); clock.setMultiplier(1);
        QCOMPARE(animation.currentTime(),2700); QCOMPARE(animation.currentValue().toDouble(),70.0);
        QCOMPARE(animation.state(),QAbstractAnimation::Paused);
        animation.resume(); animation.setCurrentTime(3000); QCOMPARE(finished.count(),1);
        QCOMPARE(animation.state(),QAbstractAnimation::Stopped);
        SpriteAnimation sprite(GameCatalog::enemies()[2].image,GameCatalog::enemies()[2].scale);
        sprite.setGameSpeed(&clock); const int originalDuration=sprite.duration();
        sprite.start(); sprite.setCurrentTime(10); const int frame=sprite.currentFrameNumber();
        clock.setMultiplier(2); QCOMPARE(sprite.currentFrameNumber(),frame);
        QCOMPARE(sprite.duration(),(originalDuration+1)/2); QCOMPARE(sprite.currentTime(),5);
        sprite.pause(); clock.setMultiplier(1);
        QCOMPARE(sprite.state(),QAbstractAnimation::Paused); QCOMPARE(sprite.duration(),originalDuration);
        QCOMPARE(sprite.currentFrameNumber(),frame); QCOMPARE(sprite.currentTime(),10);
    }
    void speedButtonScalesBattleAndNewObjects() {
        PlayScene play(10,nullptr,false); play.resize(1280,720); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *view=play.findChild<QGraphicsView*>();
        auto *button=play.findChild<QPushButton*>("battleSpeed"); QVERIFY(button); QVERIFY(button->isVisible());
        QCOMPARE(play.speedMultiplier(),1); QVERIFY(!button->isChecked());
        scene->addHeart(1500);
        // Include every plant's sprite, shooters, producers, burst gap and charge.
        const int types[]={0,1,5,6,3,2,4,7};
        for(int i=0;i<8;++i) {
            scene->setChosenNum(types[i]); Card::setGameState(GameState::PrePlace);
            QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,
                view->mapFromScene(QPointF(440+(i/5)*242,202+(i%5)*145)));
        }
        QCOMPARE(scene->findChildren<WhiteDogs*>().size(),8);
        scene->setAYellowDog(2,1); scene->generateSkyHeart(); scene->generateBullet(2,0);
        const auto animations=play.findChildren<QAbstractAnimation*>(); QVector<int> durations;
        for(auto *animation : animations) durations.append(animation->duration());
        auto *enemy=scene->findChild<YellowDogs*>(); auto *heart=scene->findChild<Heart*>();
        const QPointF enemyPosition=enemy->pos(),heartPosition=heart->pos();
        QTest::mouseClick(button,Qt::LeftButton); QCOMPARE(play.speedMultiplier(),2);
        QVERIFY(button->isChecked()); QVERIFY(button->text().contains("2×"));
        for(int i=0;i<animations.size();++i) QCOMPARE(animations[i]->duration(),(durations[i]+1)/2);
        QVERIFY(QLineF(enemyPosition,enemy->pos()).length()<2); QVERIFY(QLineF(heartPosition,heart->pos()).length()<2);
        const auto timers=play.findChildren<GameTimer*>(); QVERIFY(timers.size()>15);
        for(auto *timer : timers) {
            QVERIFY(timer->interval()>0);
            QVERIFY2(timer->interval()<=qMax(1,(timer->gameInterval()+1)/2),
                qPrintable(QString("%1 / %2: wall=%3 game=%4").arg(timer->objectName(),timer->parent()->metaObject()->className())
                    .arg(timer->interval()).arg(timer->gameInterval())));
        }
        auto *producer=scene->findChild<AllHeartWhite*>();
        QCOMPARE(producer->findChild<GameTimer*>("doubleHeartTimer")->gameInterval(),12000);
        auto *doubleSinger=scene->findChild<DblSingWhite*>();
        QCOMPARE(doubleSinger->findChild<QTimer*>("doubleBurstTimer")->interval(),800);
        QCOMPARE(doubleSinger->findChild<QTimer*>("doubleSecondShotTimer")->interval(),90);
        scene->generateSkyHeart(); auto *newHeart=scene->findChildren<Heart*>().back();
        QCOMPARE(newHeart->findChild<QPropertyAnimation*>("heartFallAnimation")->duration(),4000);
        QCOMPARE(newHeart->findChild<QPropertyAnimation*>("heartCollectAnimation")->duration(),400);
        QCOMPARE(newHeart->findChild<QPropertyAnimation*>("heartBlinkAnimation")->duration(),250);
        QCOMPARE(newHeart->findChild<QTimer*>()->interval(),1750);
        scene->setAYellowDog(1,1); auto *newEnemy=scene->findChildren<YellowDogs*>().back();
        QCOMPARE(newEnemy->findChild<QTimer*>("guitarRangedTimer")->interval(),1000);
        QCOMPARE(newEnemy->findChild<QPropertyAnimation*>("enemyMovement")->duration(),500);
        newEnemy->shootNote(); auto *projectile=scene->findChild<EnemyProjectile*>(); QVERIFY(projectile);
        QCOMPARE(projectile->findChild<QTimer*>()->interval(),15);
        const int projectileDuration=projectile->findChild<QPropertyAnimation*>()->duration();
        Card::setGameState(GameState::PrePlace); Card::setSelectedWhite("heartWhite");
        play.gamePaused(); const QPointF pausedPosition=newEnemy->pos(); const float cooldown=play.findChild<Card*>()->coolProgress();
        play.setSpeedMultiplier(1); QTest::qWait(100);
        QCOMPARE(newEnemy->pos(),pausedPosition); QCOMPARE(play.findChild<Card*>()->coolProgress(),cooldown);
        QVERIFY(!scene->getGameTimer()->isActive());
        QVERIFY(qAbs(projectile->findChild<QPropertyAnimation*>()->duration()-projectileDuration*2)<=1);
        play.setSpeedMultiplier(2); play.gameContinued();
        QCOMPARE(play.speedMultiplier(),2); QCOMPARE(Card::currentState(),GameState::PrePlace);
        QCOMPARE(Card::selectedWhite(),QString("heartWhite"));
        QTRY_VERIFY_WITH_TIMEOUT(play.findChild<Card*>()->coolProgress()>cooldown,200);
        QTest::mouseClick(button,Qt::LeftButton); QCOMPARE(play.speedMultiplier(),1);
        QCOMPARE(newHeart->findChild<QPropertyAnimation*>("heartFallAnimation")->duration(),8000);
        QCOMPARE(newEnemy->findChild<QPropertyAnimation*>("enemyMovement")->duration(),1000);
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) { play.setSpeedMultiplier(2); QVERIFY(play.grab().save(folder+"/battle-double-speed.png")); }
    }
    void doubleSpeedHeartLifecycleUsesCurrentRate() {
        PlayScene play(8,nullptr,false); auto *scene=play.findChild<MyGameScene*>();
        play.setSpeedMultiplier(2); scene->generateSkyHeart();
        auto *heart=scene->findChild<Heart*>(); auto *fall=heart->findChild<QPropertyAnimation*>("heartFallAnimation");
        fall->setCurrentTime(fall->duration());
        auto *expiry=heart->findChild<QTimer*>(); QCOMPARE(expiry->interval(),1750); QVERIFY(expiry->isActive());
        QMetaObject::invokeMethod(expiry,"timeout"); auto *blink=heart->findChild<QPropertyAnimation*>("heartBlinkAnimation");
        QCOMPARE(blink->duration(),250); QCOMPARE(blink->loopCount(),2);
        blink->setCurrentTime(blink->totalDuration());
        const auto animations=heart->findChildren<QPropertyAnimation*>(); QCOMPARE(animations.size(),4);
        auto *fade=animations.back(); QCOMPARE(fade->duration(),500);
        fade->setCurrentTime(350); const qreal opacity=heart->opacity();
        play.setSpeedMultiplier(1); QCOMPARE(fade->duration(),1000);
        QCOMPARE(fade->state(),QAbstractAnimation::Running); QVERIFY(qAbs(heart->opacity()-opacity)<.005);
    }
    void doubleSpeedOpeningWarningsAndResult() {
        PlayScene openingPlay(3); openingPlay.show();
        auto *button=openingPlay.findChild<QPushButton*>("battleSpeed"); QVERIFY(button->isVisible());
        QTest::mouseClick(button,Qt::LeftButton);
        auto *opening=openingPlay.findChild<LevelOpening*>();
        auto *timeline=opening->findChild<QVariantAnimation*>("openingTimeline"); QCOMPARE(timeline->duration(),500);
        timeline->setCurrentTime(350); const auto value=timeline->currentValue();
        openingPlay.setSpeedMultiplier(1); QCOMPARE(timeline->currentValue(),value);
        openingPlay.setSpeedMultiplier(2);
        completeOpening(&openingPlay); QVERIFY(openingPlay.findChild<MyGameScene*>()->gameplayStarted());
        QCOMPARE(openingPlay.speedMultiplier(),2);
        PlayScene endless(10,nullptr,false,true,5); endless.show(); endless.setSpeedMultiplier(2);
        auto *scene=endless.findChild<MyGameScene*>(); auto *wave=scene->findChild<QTimer*>("waveTimer");
        auto *stagger=scene->findChild<QTimer*>("waveStaggerTimer");
        QMetaObject::invokeMethod(wave,"timeout"); QCOMPARE(scene->wavesStarted(),5);
        QCOMPARE(stagger->interval(),900); QVERIFY(scene->findChildren<YellowDogs*>().isEmpty());
        auto *banner=endless.findChild<BattleBanner*>()->findChild<QVariantAnimation*>("bannerAnimation");
        QCOMPARE(banner->duration(),900); const int remaining=stagger->remainingTime();
        endless.gamePaused(); endless.setSpeedMultiplier(1); QTest::qWait(60);
        QVERIFY(!stagger->isActive()); endless.gameContinued();
        QVERIFY(qAbs(stagger->remainingTime()-remaining*2)<35);
        endless.setSpeedMultiplier(2);
        QTRY_VERIFY_WITH_TIMEOUT(!scene->findChildren<YellowDogs*>().isEmpty(),1100);
        QVERIFY(stagger->interval()>0);
        QCOMPARE(stagger->interval(),qRound(scene->findChild<GameTimer*>("waveStaggerTimer")->gameInterval()/2.0));
        // Victory presentation also follows the chosen rate; the next battle starts at 1x.
        PlayScene victory(8,nullptr,false); QCOMPARE(victory.speedMultiplier(),1); victory.setSpeedMultiplier(2);
        emit victory.gameWin(); auto *result=victory.findChild<BattleResult*>(); QVERIFY(result);
        QCOMPARE(result->findChild<QVariantAnimation*>("resultAnimation")->duration(),1800);
        QVERIFY(!victory.findChild<QPushButton*>("battleSpeed")->isEnabled());
        PlayScene fresh(8,nullptr,false); QCOMPARE(fresh.speedMultiplier(),1);
    }
    void heartsStayCompleteDuringBattlefieldUpdates() {
        if(QGuiApplication::platformName()!="windows") QSKIP("Checks the native Windows backing store without forcing a render.");
        PlayScene play(8,nullptr,false); play.resize(950,518); play.move(30,30); play.show();
        QVERIFY(QTest::qWaitForWindowExposed(&play));
        auto *scene=play.findChild<MyGameScene*>();
        auto *view=play.findChild<QGraphicsView*>();
        for(auto *timer : scene->findChildren<QTimer*>()) timer->stop();
        QVector<Heart*> hearts;
        for(int row=0;row<5;++row) for(int column=0;column<6;++column) {
            const QPointF position(430+column*155,180+row*135);
            auto *heart=new Heart(position,position+QPointF(27,45),scene,QEasingCurve::Linear,scene,true);
            scene->addItem(heart); hearts.append(heart);
        }
        for(int row=0;row<5;++row) {
            scene->setAYellowDog(row,1);
            auto *enemy=scene->getZombieMap(row).back();
            enemy->stopMoving(); enemy->setPos(550,180+row*135);
            for(auto *timer : enemy->findChildren<QTimer*>()) timer->stop();
        }
        QTest::qWait(100);
        auto capture=[&] { return play.screen()->grabWindow(play.winId()).toImage(); };
        for(int frame=0;frame<30;++frame) {
            for(int i=0;i<hearts.size();++i) {
                hearts[i]->setPos(430+(i%6)*155+frame*.37,180+(i/6)*135+frame*.43);
                hearts[i]->setOpacity(frame%7==0 ? .35 : 1.0);
            }
            // An unrelated tiny dirty area must not replace the heart's paint area.
            scene->update(QRectF(710+frame,320+frame,4,4));
            QTest::qWait(25);
            const QImage partial=capture();
            view->viewport()->repaint();
            const QImage complete=capture();
            QVERIFY(!partial.isNull()); QCOMPARE(partial.size(),complete.size());
            int changed=0;
            for(auto *heart : hearts) {
                QRect area=view->mapFromScene(heart->sceneBoundingRect()).boundingRect();
                area.translate(view->viewport()->mapTo(&play,QPoint()));
                area=area.intersected(partial.rect());
                int pinkPixels=0;
                for(int y=area.top();y<=area.bottom();++y) for(int x=area.left();x<=area.right();++x) {
                    const QColor a=partial.pixelColor(x,y),b=complete.pixelColor(x,y);
                    if(qAbs(a.red()-b.red())+qAbs(a.green()-b.green())+qAbs(a.blue()-b.blue())>40) ++changed;
                    if(a.red()>180 && a.red()-a.green()>40 && a.blue()-a.green()>15) ++pinkPixels;
                }
                if(heart->opacity()==1.0) QVERIFY2(pinkPixels>area.width()*area.height()*.35,"The heart must contain its whole pink body, not just a corner.");
            }
            QVERIFY2(changed<40,qPrintable(QString("Frame %1: %2 heart pixels were missing until full repaint").arg(frame).arg(changed)));
        }
    }
    void cachedBackgroundRetainsGrassAcrossResizeAndCamera() {
        PlayScene play(8,nullptr,false); play.resize(1650,900); play.show();
        auto *view=play.findChild<QGraphicsView*>();
        QPixmap grass(":/others/Image/grass.jpg"); const auto source=grass.scaledToHeight(900).toImage();
        const QPointF point(700,300);
        const auto verifyGrass=[&] {
            const auto image=view->viewport()->grab().toImage(); const auto pixel=view->mapFromScene(point);
            QVERIFY(image.rect().contains(pixel));
            const auto actual=image.pixelColor(pixel),expected=source.pixelColor(point.toPoint());
            QVERIFY(actual!=QColor("#20291c"));
            QVERIFY(qAbs(actual.red()-expected.red())<10 && qAbs(actual.green()-expected.green())<10 && qAbs(actual.blue()-expected.blue())<10);
        };
        verifyGrass();
        play.resize(1280,720); verifyGrass();
        play.resize(1920,1080); verifyGrass();
        PlayScene opening(4); opening.show();
        auto *timeline=opening.findChild<LevelOpening*>()->findChild<QVariantAnimation*>("openingTimeline");
        timeline->setCurrentTime(timeline->duration());
        auto *openingView=opening.findChild<QGraphicsView*>();
        const auto preview=openingView->viewport()->grab().toImage();
        QVERIFY(preview.pixelColor(openingView->mapFromScene(point))!=QColor("#20291c"));
        completeOpening(&opening);
        const auto returned=openingView->viewport()->grab().toImage();
        QVERIFY(returned.pixelColor(openingView->mapFromScene(point))!=QColor("#20291c"));
    }
    void sharedSpriteDecoderMatchesEveryOriginalFrame() {
        auto check=[](const QString& path,qreal scale) {
            QMovie original(path); original.setCacheMode(QMovie::CacheAll);
            SpriteAnimation shared(path,scale);
            QImageReader timing(path);
            QCOMPARE(shared.frameCount(),original.frameCount());
            int duration=0;
            for(int frame=0;frame<shared.frameCount();++frame) {
                QVERIFY(original.jumpToFrame(frame)); QVERIFY(shared.jumpToFrame(frame));
                const auto pixels=original.currentPixmap();
                QCOMPARE(shared.currentPixmap().toImage(),pixels.scaled(pixels.size()*scale,Qt::KeepAspectRatio,Qt::SmoothTransformation).toImage());
                QVERIFY(!timing.read().isNull());
                duration+=qMax(10,timing.nextImageDelay());
            }
            QCOMPARE(shared.duration(),duration);
            shared.start(); shared.setCurrentTime(shared.duration()+1); QCOMPARE(shared.currentFrameNumber(),0);
        };
        for(const auto& enemy : GameCatalog::enemies()) check(enemy.image,enemy.scale);
        for(const auto& plant : GameCatalog::plants()) check(plant.image,1.0);
    }
    void dashSpeedThreshold_data() {
        QTest::addColumn<int>("wave");
        QTest::addColumn<int>("health");
        QTest::newRow("campaign") << 1 << 570;
        QTest::newRow("endless-wave-six") << 6 << 798;
        QTest::newRow("endless-wave-fifteen") << 15 << 1208;
    }
    void dashSpeedThreshold() {
        QFETCH(int,wave); QFETCH(int,health);
        MyGameScene scene(10,nullptr,true); YellowDogs enemy(2,&scene,2,wave);
        QCOMPARE(enemy.getHp(),health); enemy.startMoving();
        auto *walk=enemy.findChild<QPropertyAnimation*>("enemyMovement"); QVERIFY(walk);
        const qreal base=walk->startValue().toPointF().x()-walk->endValue().toPointF().x();
        QVERIFY(base>=40*(1+.01*(wave-1))-.0001 && base<=49*(1+.01*(wave-1))+.0001);
        walk->setCurrentTime(300);
        enemy.getAttacked(health/2); QCOMPARE(enemy.getHp(),health/2);
        QCOMPARE(walk->startValue().toPointF().x()-walk->endValue().toPointF().x(),base);
        const QPointF before=enemy.pos();
        enemy.getAttacked(1); QCOMPARE(enemy.pos(),before);
        QCOMPARE(walk->state(),QAbstractAnimation::Running);
        QVERIFY(qAbs(walk->startValue().toPointF().x()-walk->endValue().toPointF().x()-base*2.0)<.0001);
        walk->setCurrentTime(500); QVERIFY(qAbs(enemy.x()-(before.x()-base))<.0001);
        enemy.getAttacked(1);
        QVERIFY(qAbs(walk->startValue().toPointF().x()-walk->endValue().toPointF().x()-base*2.0)<.0001);
    }
    void dashAccelerationPreservesPause() {
        MyGameScene scene(10,nullptr,true); YellowDogs enemy(2,&scene,2); enemy.startMoving();
        auto *walk=enemy.findChild<QPropertyAnimation*>("enemyMovement");
        const qreal base=walk->startValue().toPointF().x()-walk->endValue().toPointF().x();
        GamePause pause; pause.pause(&enemy); const QPointF before=enemy.pos();
        enemy.cutHp(286); QCOMPARE(walk->state(),QAbstractAnimation::Paused);
        QTest::qWait(80); QCOMPARE(enemy.pos(),before);
        pause.resume(); QCOMPARE(walk->state(),QAbstractAnimation::Running);
        walk->setCurrentTime(500); QVERIFY(qAbs(enemy.x()-(before.x()-base))<.0001);
    }
    void dashLowHealthBitesFaster_data() {
        QTest::addColumn<int>("rate"); QTest::addColumn<int>("wave");
        QTest::newRow("campaign-normal") << 1 << 1;
        QTest::newRow("campaign-double") << 2 << 1;
        QTest::newRow("endless-double") << 2 << 15;
    }
    void dashLowHealthBitesFaster() {
        QFETCH(int,rate); QFETCH(int,wave);
        PlayScene play(10,nullptr,false); play.show(); play.setSpeedMultiplier(rate);
        auto *scene=play.findChild<MyGameScene*>(); auto *view=play.findChild<QGraphicsView*>();
        scene->setChosenNum(2); Card::setGameState(GameState::PrePlace);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        auto *defender=scene->plantAhead(2,1000); QVERIFY(defender);
        auto *enemy=new YellowDogs(2,scene,2,wave); enemy->setParent(scene); scene->addItem(enemy);
        enemy->setPos(defender->sceneBoundingRect().center()-enemy->boundingRect().center());
        QVERIFY(enemy->checkCollision());
        auto *combat=enemy->findChild<GameTimer*>("dashCombatTimer"); QVERIFY(combat);
        QCOMPARE(combat->gameInterval(),500);
        const int health=enemy->getHp(); enemy->cutHp(health/2);
        QCOMPARE(combat->gameInterval(),500); // Exactly half health has no bonus.
        QElapsedTimer elapsed; elapsed.start(); QVector<qint64> bites;
        connect(defender,&MyItem::healthChanged,&play,[&] { bites.append(elapsed.elapsed()); });
        const int biteDamage=wave==1 ? 80 : 108;
        QTRY_COMPARE_WITH_TIMEOUT(bites.size(),2,1400);
        QCOMPARE(defender->getHp(),4000-2*biteDamage);
        QVERIFY(qAbs(bites[1]-bites[0]-500.0/rate)<80);
        const int remaining=combat->remainingTime(); enemy->cutHp(1);
        QCOMPARE(combat->gameInterval(),333);
        QVERIFY(qAbs(combat->remainingTime()-remaining*333.0/500)<15);
        QTRY_COMPARE_WITH_TIMEOUT(bites.size(),5,1400);
        QCOMPARE(defender->getHp(),4000-5*biteDamage);
        for(int i=3;i<5;++i) QVERIFY(qAbs(bites[i]-bites[i-1]-333.0/rate)<80);
        enemy->cutHp(1); QCOMPARE(combat->gameInterval(),333); // The bonus does not stack.
        const int pausedHealth=defender->getHp(),pausedRemaining=combat->remainingTime();
        play.gamePaused(); const int otherRate=rate==1 ? 2 : 1; play.setSpeedMultiplier(otherRate);
        QTest::qWait(100); QCOMPARE(defender->getHp(),pausedHealth); QVERIFY(!combat->isActive());
        play.gameContinued();
        QVERIFY(qAbs(combat->remainingTime()-pausedRemaining*double(rate)/otherRate)<25);
        QTRY_COMPARE_WITH_TIMEOUT(bites.size(),6,750); QCOMPARE(defender->getHp(),4000-6*biteDamage);
        scene->removeWhite(2,0); QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QMetaObject::invokeMethod(combat,"timeout"); QCOMPARE(bites.size(),6);
        auto *walk=enemy->findChild<QPropertyAnimation*>("enemyMovement");
        QCOMPARE(walk->state(),QAbstractAnimation::Running);
        enemy->getAttacked(10000); QVERIFY(!combat->isActive());
        QMetaObject::invokeMethod(combat,"timeout"); QCOMPARE(bites.size(),6);
    }
    void dashBiteBonusPreservesPausedAnimationAndTimerProgress() {
        MyGameScene scene(10,nullptr,true); YellowDogs enemy(2,&scene,2); WallWhite defender;
        enemy.startAttacking(&defender);
        auto *bite=enemy.findChild<QPropertyAnimation*>("enemyBiteAnimation");
        QVERIFY(bite); bite->setCurrentTime(240);
        auto *combat=enemy.findChild<GameTimer*>("dashCombatTimer");
        QTest::qWait(40); const int remaining=combat->remainingTime(); const qreal progress=enemy.biteProgress();
        GamePause pause; pause.pause(&enemy); enemy.cutHp(286);
        QCOMPARE(combat->gameInterval(),333); QVERIFY(!combat->isActive());
        QCOMPARE(bite->duration(),200); QCOMPARE(bite->state(),QAbstractAnimation::Paused);
        QVERIFY(qAbs(enemy.biteProgress()-progress)<.005);
        scene.gameSpeed()->setMultiplier(2); QTest::qWait(80);
        QVERIFY(!combat->isActive()); pause.resume();
        QVERIFY(qAbs(combat->remainingTime()-remaining*333.0/500/2)<20);
        QCOMPARE(bite->duration(),100); QCOMPARE(bite->state(),QAbstractAnimation::Running);
    }
    void dashAccelerationPreservesBiteAndKnockback() {
        MyGameScene scene(10,nullptr,true); YellowDogs enemy(2,&scene,2); enemy.startMoving();
        auto *walk=enemy.findChild<QPropertyAnimation*>("enemyMovement");
        const qreal base=walk->startValue().toPointF().x()-walk->endValue().toPointF().x();
        enemy.stopMoving(); enemy.cutHp(285); DancingWhite defender;
        enemy.startAttacking(&defender); QCOMPARE(enemy.getHp(),245); // Forty points reflected from the bite cross the half-health threshold.
        QCOMPARE(walk->state(),QAbstractAnimation::Stopped);
        enemy.startMoving(); QVERIFY(qAbs(walk->startValue().toPointF().x()-walk->endValue().toPointF().x()-base*2.0)<.0001);

        YellowDogs knocked(2,&scene,2); knocked.startMoving();
        auto *knockedWalk=knocked.findChild<QPropertyAnimation*>("enemyMovement");
        const qreal knockedBase=knockedWalk->startValue().toPointF().x()-knockedWalk->endValue().toPointF().x();
        knocked.stopMoving(); knocked.cutHp(285); MoneyWhite repeller; knocked.startAttacking(&repeller);
        auto *back=knocked.findChild<QPropertyAnimation*>("enemyKnockback");
        back->setCurrentTime(100); const QPointF before=knocked.pos(); knocked.cutHp(1);
        QCOMPARE(knocked.pos(),before); QCOMPARE(back->state(),QAbstractAnimation::Running);
        QCOMPARE(knockedWalk->state(),QAbstractAnimation::Stopped);
        back->setCurrentTime(back->duration()); QCOMPARE(knockedWalk->state(),QAbstractAnimation::Running);
        QVERIFY(qAbs(knockedWalk->startValue().toPointF().x()-knockedWalk->endValue().toPointF().x()-knockedBase*2.0)<.0001);
    }
    void dashAccelerationIgnoresOtherTypesAndLethalDamage() {
        MyGameScene scene(10,nullptr,true);
        for(int type=0;type<3;++type) {
            YellowDogs enemy(2,&scene,type); enemy.startMoving();
            auto *walk=enemy.findChild<QPropertyAnimation*>("enemyMovement");
            const QPointF start=walk->startValue().toPointF(),end=walk->endValue().toPointF();
            if(type<2) {
                enemy.cutHp(enemy.getHp()/2+1);
                QCOMPARE(walk->startValue().toPointF(),start); QCOMPARE(walk->endValue().toPointF(),end);
            }
            enemy.getAttacked(enemy.getHp()); QVERIFY(enemy.isDying());
            QCOMPARE(walk->state(),QAbstractAnimation::Stopped);
            QCOMPARE(walk->startValue().toPointF(),start); QCOMPARE(walk->endValue().toPointF(),end);
        }
    }
    void dashWeightAndCampaignAvailability() {
        QCOMPARE(GameCatalog::enemies()[2].weight,3); QCOMPARE(GameCatalog::enemies()[2].health,570);
        const auto details=GameCatalog::enemyDetails(2);
        QVERIFY(details.contains("570")); QVERIFY(details.contains("权重：3")); QVERIFY(details.contains("2 倍")); QVERIFY(details.contains("啃食频率提升至原来的 1.5 倍"));
        for(int level=7;level<=8;++level) {
            int dashCount=0,middleDashCount=0;
            for(int seed=1;seed<=500;++seed) {
                QRandomGenerator random(seed); const auto plan=WavePlanner::create(level,random);
                for(int wave=0;wave<plan.size();++wave) for(int type : plan[wave]) if(type==2) {
                    ++dashCount; QVERIFY(wave>=2);
                    if(wave<plan.size()-2) ++middleDashCount;
                }
                if(level==8) { QVERIFY(plan[plan.size()-2].count(2)>=1); QVERIFY(plan.back().count(2)>=1); }
                else QVERIFY(plan.back().size()>plan[plan.size()-2].size());
            }
            QVERIFY(dashCount>0);
            QVERIFY(middleDashCount>0); // Weight three fits the middle-wave budgets.
        }
    }
    void regionalRedrawBoundsContainBiteAndDeathFrames() {
        MyGameScene scene(10,nullptr,true);
        for(int type=0;type<3;++type) {
            YellowDogs enemy(2,&scene,type);
            const QRectF sprite=enemy.pixmap().rect();
            QVERIFY(enemy.boundingRect().contains(sprite.translated(5,0)));
            QVERIFY(enemy.boundingRect().contains(sprite.translated(-5,0)));
            enemy.removeItself();
            for(int frame=0;frame<=20;++frame) {
                const qreal progress=frame/20.0; QTransform transform;
                transform.translate(sprite.center().x(),sprite.center().y());
                transform.rotate(-65*progress); transform.scale(1-.35*progress,1-.55*progress);
                transform.translate(-sprite.center().x(),-sprite.center().y());
                QVERIFY(enemy.boundingRect().contains(transform.mapRect(sprite)));
            }
        }
    }
    void sharedSpritePixelsKeepIndependentAnimationAndHealth() {
        MyGameScene scene(10,nullptr,true);
        auto *first=new YellowDogs(2,&scene,0),*second=new YellowDogs(2,&scene,0);
        for(auto *enemy : {first,second}) { enemy->setParent(&scene); scene.addItem(enemy); enemy->setHealthVisible(true); }
        auto *a=first->findChild<SpriteAnimation*>(),*b=second->findChild<SpriteAnimation*>();
        a->setPaused(true); b->setPaused(true); QVERIFY(a->jumpToFrame(1)); QVERIFY(b->jumpToFrame(1));
        QCOMPARE(first->pixmap().cacheKey(),second->pixmap().cacheKey());
        QCOMPARE(first->pixmap().toImage(),a->currentPixmap().toImage());
        const auto sharedKey=first->pixmap().cacheKey();
        QVERIFY(b->jumpToFrame(2)); QCOMPARE(a->currentFrameNumber(),1);
        QVERIFY(b->jumpToFrame(1)); QCOMPARE(second->pixmap().cacheKey(),sharedKey);
        first->getAttacked(30); QCOMPARE(first->healthText(),QString("270")); QCOMPARE(second->healthText(),QString("300"));
        QVERIFY(first->hitFlash()>0); QCOMPARE(second->hitFlash(),qreal(0));
        b->setPaused(false); QTRY_VERIFY_WITH_TIMEOUT(b->currentFrameNumber()!=1,1000); QCOMPARE(a->currentFrameNumber(),1);
    }
    void releasedChargerRemainsInCombatLists() {
        PlayScene play(8,nullptr,false); play.show(); auto *scene=play.findChild<MyGameScene*>();
        scene->addHeart(125); scene->setChosenNum(3); Card::setGameState(GameState::PrePlace);
        auto *view=play.findChild<QGraphicsView*>();
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        auto *charger=scene->findChild<LineWhite*>(); QVERIFY(charger);
        const qreal origin=charger->x(); QTRY_VERIFY_WITH_TIMEOUT(charger->x()>origin,300);
        charger->findChild<QPropertyAnimation*>("chargeMovement")->pause(); charger->setPos(700,charger->y());
        QCOMPARE(scene->plantAhead(2,1000),static_cast<WhiteDogs*>(charger));
        const int initialHealth=charger->getHp();
        auto *note=new EnemyProjectile(scene,2,charger->sceneBoundingRect().center(),15);
        note->findChild<QPropertyAnimation*>()->pause(); QMetaObject::invokeMethod(note->findChild<QTimer*>(),"timeout");
        QCOMPARE(charger->getHp(),initialHealth-15);
        scene->setAYellowDog(1); auto *other=static_cast<YellowDogs*>(scene->getZombieMap(1).front()); other->stopMoving();
        other->setPos(charger->sceneBoundingRect().center()-other->boundingRect().center()); QVERIFY(!charger->checkCollision());
        scene->setAYellowDog(2); auto *enemy=static_cast<YellowDogs*>(scene->getZombieMap(2).front()); enemy->stopMoving();
        enemy->setPos(charger->sceneBoundingRect().center()-enemy->boundingRect().center());
        QVERIFY(enemy->checkCollision()); enemy->startAttacking(charger); QCOMPARE(charger->getHp(),initialHealth-15-GameCatalog::enemies()[0].attack);
        QVERIFY(charger->checkCollision()); QMetaObject::invokeMethod(scene->getGameTimer(),"timeout");
        QVERIFY(enemy->isDying()); QCOMPARE(other->getHp(),300);
    }
    void singleFlightNoteKeepsSpeedPauseAndRowCollision() {
        PlayScene play(8,nullptr,false); auto *scene=play.findChild<MyGameScene*>(); scene->generateBullet(2,0);
        QPointer<Bullet> note=scene->findChild<Bullet*>(); auto *flight=note->findChild<QPropertyAnimation*>();
        QCOMPARE(flight->endValue().toPointF().x(),qreal(1701)); flight->setCurrentTime(1000);
        QVERIFY(qAbs(note->x()-700)<1); play.gamePaused(); const auto position=note->pos(); QTest::qWait(100); QCOMPARE(note->pos(),position);
        play.gameContinued();
        scene->setAYellowDog(1); auto *other=static_cast<YellowDogs*>(scene->getZombieMap(1).front()); other->stopMoving();
        other->setPos(note->sceneBoundingRect().center()-other->boundingRect().center()); QVERIFY(!note->checkCollision());
        scene->setAYellowDog(2); auto *enemy=static_cast<YellowDogs*>(scene->getZombieMap(2).front()); enemy->stopMoving();
        enemy->setPos(note->sceneBoundingRect().center()-enemy->boundingRect().center()); QVERIFY(note->checkCollision());
        QMetaObject::invokeMethod(scene->getGameTimer(),"timeout"); QCOMPARE(enemy->getHp(),270); QCOMPARE(other->getHp(),300);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete); QVERIFY(note.isNull());
        scene->generateBullet(2,0); note=scene->findChild<Bullet*>(); flight=note->findChild<QPropertyAnimation*>();
        flight->setCurrentTime(flight->duration()); QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete); QVERIFY(note.isNull());
    }
    void chargingWhiteFreesItsCellAndCannotRemoveReplacement() {
        PlayScene play(8,nullptr,false); play.show(); auto *scene=play.findChild<MyGameScene*>();
        auto *view=play.findChild<QGraphicsView*>(); auto *chargeCard=play.findChild<Card*>("plantCard3");
        QCOMPARE(chargeCard->heartCost,125); QCOMPARE(chargeCard->coolTime,30000);
        QVERIFY(chargeCard->toolTip().contains("冷却：30 秒")); QVERIFY(chargeCard->toolTip().contains("125 爱心"));
        scene->addHeart(25); scene->heartCollected(); chargeCard->cooldownFinished();
        QTest::mouseClick(chargeCard,Qt::LeftButton);
        auto click=[&] { QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490))); };
        click(); QPointer<LineWhite> charger=scene->findChild<LineWhite*>(); QVERIFY(charger);
        QCOMPARE(charger->HeartCost(),125); QCOMPARE(scene->getRestHeart(),0); QVERIFY(chargeCard->isCooling());
        const auto initial=charger->pos(); QTRY_VERIFY_WITH_TIMEOUT(charger->x()>initial.x(),300);
        play.gamePaused(); const auto pausedPos=charger->pos(); QTest::qWait(80); QCOMPARE(charger->pos(),pausedPos); play.gameContinued();
        scene->addHeart(50); scene->heartCollected(); QTest::mouseClick(play.findChild<Card*>("plantCard1"),Qt::LeftButton); click();
        auto *replacement=scene->findChild<HeartWhite*>(); QVERIFY(replacement); QCOMPARE(scene->getRestHeart(),0);
        QCOMPARE(scene->plantsInRow(2).size(),2); scene->togglePlantHealth(); QVERIFY(charger->isHealthVisible());
        QSignalSpy removed(scene,&MyGameScene::plantRemoved);
        charger->setPos(1701,charger->y()); QMetaObject::invokeMethod(scene->getGameTimer(),"timeout");
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QVERIFY(charger.isNull()); QCOMPARE(scene->plantsInRow(2).size(),1);
        QCOMPARE(scene->plantsInRow(2).front(),static_cast<WhiteDogs*>(replacement)); QCOMPARE(removed.count(),0);
        scene->toggleShovel(); click(); QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QVERIFY(scene->plantsInRow(2).isEmpty()); QCOMPARE(removed.count(),1);
    }
    void multipleChargersCanShareAReleasedOrigin() {
        PlayScene play(8,nullptr,false); play.show(); auto *scene=play.findChild<MyGameScene*>();
        auto *view=play.findChild<QGraphicsView*>(); scene->addHeart(1000);
        for(int i=0;i<2;++i) {
            scene->setChosenNum(3); Card::setGameState(GameState::PrePlace);
            QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
            const auto chargers=scene->findChildren<LineWhite*>(); QCOMPARE(chargers.size(),i+1);
            auto *newest=chargers.back(); const qreal initial=newest->x();
            QTRY_VERIFY_WITH_TIMEOUT(newest->x()>initial,300);
        }
        auto chargers=scene->findChildren<LineWhite*>(); QCOMPARE(scene->plantsInRow(2).size(),2);
        for(auto *charger : chargers) { charger->cutHp(10000); charger->removeItself(); }
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QVERIFY(scene->plantsInRow(2).isEmpty()); QVERIFY(scene->findChildren<LineWhite*>().isEmpty());
    }
    void doubleSingerFiresPairedBurstsAndPausesBetweenShots() {
        PlayScene play(8,nullptr,false); play.show(); auto *scene=play.findChild<MyGameScene*>();
        auto *view=play.findChild<QGraphicsView*>(); scene->addHeart(1000); scene->setChosenNum(6);
        Card::setGameState(GameState::PrePlace);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        auto *singer=scene->findChild<DblSingWhite*>(); QVERIFY(singer);
        auto *burst=singer->findChild<QTimer*>("doubleBurstTimer"); auto *second=singer->findChild<QTimer*>("doubleSecondShotTimer");
        QCOMPARE(burst->interval(),GameCatalog::plants()[0].actionIntervalMs); QCOMPARE(burst->interval(),1600);
        QCOMPARE(second->interval(),180); QVERIFY(second->isSingleShot());
        QSignalSpy shots(singer,&DblSingWhite::bulletShot); QElapsedTimer clock; QVector<qint64> times; clock.start();
        connect(singer,&DblSingWhite::bulletShot,&play,[&] { times.append(clock.elapsed()); });
        QTest::qWait(100); QCOMPARE(shots.count(),0);
        scene->setAYellowDog(2); auto *enemy=static_cast<YellowDogs*>(scene->getZombieMap(2).front()); enemy->stopMoving();
        QTRY_COMPARE_WITH_TIMEOUT(shots.count(),4,4000);
        QVERIFY(times[1]-times[0]>=120 && times[1]-times[0]<=400);
        QVERIFY(times[3]-times[2]>=120 && times[3]-times[2]<=400);
        QVERIFY(times[2]-times[0]>=1450 && times[2]-times[0]<=1850);
        burst->stop(); QMetaObject::invokeMethod(burst,"timeout"); QCOMPARE(shots.count(),5);
        QVERIFY(second->isActive()); play.gamePaused(); QTest::qWait(300); QCOMPARE(shots.count(),5); QVERIFY(!second->isActive());
        play.gameContinued(); QTRY_COMPARE_WITH_TIMEOUT(shots.count(),6,500);
        burst->stop(); QMetaObject::invokeMethod(burst,"timeout"); QCOMPARE(shots.count(),7);
        enemy->getAttacked(10000); QTest::qWait(250); QCOMPARE(shots.count(),7);
        scene->setAYellowDog(2); QMetaObject::invokeMethod(scene->getGameTimer(),"timeout");
        burst->stop(); QMetaObject::invokeMethod(burst,"timeout"); QCOMPARE(shots.count(),8);
        scene->removeWhite(2,0); QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QTest::qWait(250); QCOMPARE(shots.count(),8);
    }
    void fullHeartProducesTwoCollectableHeartsAtNormalInterval() {
        PlayScene play(8,nullptr,false); play.show(); auto *scene=play.findChild<MyGameScene*>();
        auto *view=play.findChild<QGraphicsView*>(); scene->addHeart(1000); scene->setChosenNum(5);
        Card::setGameState(GameState::PrePlace);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        auto *producer=scene->findChild<AllHeartWhite*>(); QVERIFY(producer);
        auto *timer=producer->findChild<QTimer*>("doubleHeartTimer"); HeartWhite normal(scene);
        QCOMPARE(timer->interval(),normal.findChild<QTimer*>()->interval()); QCOMPARE(timer->interval(),12000);
        QSignalSpy produced(producer,&AllHeartWhite::heartGenerated);
        play.gamePaused(); QVERIFY(!timer->isActive()); play.gameContinued(); QVERIFY(timer->isActive());
        QMetaObject::invokeMethod(timer,"timeout"); QCOMPARE(produced.count(),2);
        const auto hearts=scene->findChildren<Heart*>(); QCOMPARE(hearts.size(),2);
        QVERIFY(QLineF(hearts[0]->pos(),hearts[1]->pos()).length()>=130);
        for(auto *heart : hearts) heart->findChild<QPropertyAnimation*>("heartFallAnimation")->stop();
        const int resources=scene->getRestHeart();
        for(int i=0;i<hearts.size();++i) {
            QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(hearts[i]->sceneBoundingRect().center()));
            QTRY_COMPARE_WITH_TIMEOUT(scene->getRestHeart(),resources+25*(i+1),1200);
        }
        QVERIFY(scene->findChildren<Heart*>().isEmpty());
    }
    void pausePreservesPlantAndShovelSelection() {
        QTemporaryDir dir; GameWindow root(nullptr,unlockedPath(dir),false);
        root.show(); root.startLevel(4);
        auto *play=root.playPage(); auto *scene=play->findChild<MyGameScene*>();
        auto *view=play->findChild<QGraphicsView*>(); auto *preview=play->findChild<QLabel*>("plantPreview");
        auto cards=play->findChildren<Card*>(); QVERIFY(cards.size()>1);
        const auto readyCard=cards.at(1)->grab().toImage();
        QTest::mouseClick(cards.at(1),Qt::LeftButton);
        const auto selectedCard=cards.at(1)->grab().toImage();
        const QPoint faceCenter=readyCard.rect().center();
        QVERIFY(selectedCard.pixelColor(faceCenter).value()<readyCard.pixelColor(faceCenter).value());
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(selectedCard.save(folder+"/card-selected.png"));
        QTest::mouseClick(view->viewport(),Qt::RightButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        QCOMPARE(Card::currentState(),GameState::Normal);
        QCOMPARE(cards.at(1)->grab().toImage(),readyCard);
        QTest::mouseClick(cards.at(1),Qt::LeftButton);
        scene->mouseMovedTo(QPointF(440,490));
        QCOMPARE(Card::currentState(),GameState::PrePlace); QVERIFY(preview->isVisible());
        const auto selected=Card::selectedWhite(); const int chosen=scene->getChosenNum();
        play->showPauseMenu(); QVERIFY(!preview->isVisible());
        QCOMPARE(cards.at(1)->grab().toImage(),selectedCard);
        auto *menu=play->findChild<PauseDialog*>();
        QTest::mouseClick(menu->findChild<QPushButton*>("mainMenu"),Qt::LeftButton);
        root.continueGame(); root.resize(1280,720); QTest::qWait(30);
        QTest::mouseClick(menu->findChild<QPushButton*>("resume"),Qt::LeftButton);
        QCOMPARE(Card::currentState(),GameState::PrePlace); QCOMPARE(Card::selectedWhite(),selected);
        QCOMPARE(scene->getChosenNum(),chosen); QVERIFY(preview->isVisible()); QVERIFY(view->hasMouseTracking());
        const auto resumedCard=cards.at(1)->grab().toImage();
        QVERIFY(resumedCard.pixelColor(resumedCard.rect().center()).value()<readyCard.pixelColor(faceCenter).value());
        auto click=[&](QPointF pos) { QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(pos)); };
        click(QPointF(440,490)); QCOMPARE(scene->findChildren<WhiteDogs*>().size(),1);
        scene->toggleShovel(); QCOMPARE(Card::currentState(),GameState::Shoveling);
        QGraphicsItem *shovel=nullptr;
        for(auto *item : scene->items()) if(item->toolTip().startsWith("可爱铲子")) shovel=item;
        QVERIFY(shovel); shovel->setPos(600,450); const auto position=shovel->pos();
        play->showPauseMenu(); play->showPauseMenu();
        QCOMPARE(shovel->pos(),position);
        play->gameContinued();
        QCOMPARE(Card::currentState(),GameState::Shoveling); QVERIFY(shovel->pos()!=GameArtwork::shovelHome());
        QVERIFY(view->hasMouseTracking()); click(QPointF(440,490));
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QVERIFY(scene->findChildren<WhiteDogs*>().isEmpty());
    }
    void pauseRestartResetsCurrentBattle() {
        QTemporaryDir dir; const auto path=unlockedPath(dir);
        GameWindow root(nullptr,path,false); root.show(); root.startLevel(4);
        QPointer<PlayScene> old=root.playPage(); auto *scene=old->findChild<MyGameScene*>();
        scene->setAYellowDog(2); old->showPauseMenu();
        auto *menu=old->findChild<PauseDialog*>(); auto *restart=menu->findChild<QPushButton*>("restart");
        QVERIFY(restart && restart->isVisible());
        for(auto *button : menu->findChildren<QPushButton*>()) QVERIFY(menu->rect().contains(button->mapTo(menu,QPoint())+button->rect().bottomRight()));
        QTest::mouseClick(restart,Qt::LeftButton);
        QVERIFY(root.playPage()!=old); QCOMPARE(root.playPage()->levelIndex,4);
        QVERIFY(!root.playPage()->isPaused()); QCOMPARE(Card::currentState(),GameState::Normal);
        QVERIFY(root.playPage()->findChild<MyGameScene*>()->findChildren<YellowDogs*>().isEmpty());
        for(auto *timer : scene->findChildren<QTimer*>()) QVERIFY(!timer->isActive());
        QCOMPARE(ProgressStore(path).highestCompleted(),10);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete); QVERIFY(old.isNull());
        root.playPage()->showPauseMenu(); root.startEndless(); old=root.playPage();
        auto *endlessScene=old->findChild<MyGameScene*>(); endlessScene->waveStarted(8,0);
        QCOMPARE(ProgressStore(path).endlessBest(),8); old->showPauseMenu();
        QTest::mouseClick(old->findChild<PauseDialog*>()->findChild<QPushButton*>("restart"),Qt::LeftButton);
        QVERIFY(root.playPage()!=old); QVERIFY(root.playPage()->endlessMode); QVERIFY(!root.playPage()->isPaused());
        QCOMPARE(ProgressStore(path).endlessCheckpoint(),1); QCOMPARE(ProgressStore(path).endlessBest(),8);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete); QVERIFY(old.isNull());
    }
    void gameTextUsesSmallWhiteAndGoldenDogs() {
        MainScene home; ChooseLevelScene levels;
        QCOMPARE(home.findChild<QPushButton*>("menuAlmanac")->text(),QString("小白 / 金毛图鉴"));
        verifyGameTerms(&home); verifyGameTerms(&levels);
        PlayScene play(2,nullptr,false); play.show(); play.showPauseMenu();
        verifyGameTerms(&play);
        AlmanacDialog almanac(&play); verifyGameTerms(&almanac);
        QCOMPARE(almanac.windowTitle(),QString("草坪图鉴 · 小白与金毛"));
        auto *tabs=almanac.findChild<QTabWidget*>();
        QCOMPARE(tabs->tabText(0),QString("小白卡片")); QCOMPARE(tabs->tabText(1),QString("金毛卡片"));
        PlayScene lesson(1); lesson.show(); completeOpening(&lesson);
        verifyGameTerms(&lesson);
    }
    void previewMatchesActualEnemySize() {
        PlayScene play(10); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *opening=play.findChild<LevelOpening*>();
        const auto movies=opening->findChildren<SpriteAnimation*>(); QCOMPARE(movies.size(),12);
        for(int frame : {0,1}) {
            for(auto *movie : movies) { movie->setPaused(true); QVERIFY(movie->jumpToFrame(frame)); }
            int checked=0; QSet<int> types;
            for(auto *item : scene->items()) {
                if(item->data(0).toString()!="enemyPreview") continue;
                auto *preview=dynamic_cast<QGraphicsPixmapItem*>(item); QVERIFY(preview);
                const int type=item->data(1).toInt(); types.insert(type);
                YellowDogs actual(2,scene,type);
                auto *movie=actual.findChild<SpriteAnimation*>(); movie->setPaused(true); QVERIFY(movie->jumpToFrame(frame));
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
            QVERIFY(timer->interval()>=12000); QVERIFY(timer->interval()<=15000);
            QMetaObject::invokeMethod(timer,"timeout");
        }
        QCOMPARE(scene.findChildren<Heart*>().size(),20);
        for(auto *heart : scene.findChildren<Heart*>())
            QCOMPARE(heart->findChild<QPropertyAnimation*>("heartFallAnimation")->duration(),8000);
    }
    void cartoonHeartsStayInsideVisibleCanvas() {
        QTemporaryDir dir; GameWindow root(nullptr,unlockedPath(dir),false); root.show(); root.startLevel(3);
        auto *play=root.playPage(); auto *scene=play->findChild<MyGameScene*>(); auto *view=play->findChild<QGraphicsView*>();
        scene->generateSkyHeart(); auto *heart=scene->findChild<Heart*>(); QVERIFY(heart);
        auto *fall=heart->findChild<QPropertyAnimation*>("heartFallAnimation"); fall->pause(); fall->setCurrentTime(0);
        QCOMPARE(heart->opacity(),qreal(1)); QCOMPARE(heart->pixmap().size(),QSize(108,100));
        const QPixmap old(":/others/Image/heart.png"); QVERIFY(heart->boundingRect().width()>old.width()*.6);
        QVERIFY(heart->boundingRect().height()>old.height()*.6);
        QCOMPARE(heart->pixmap().toImage().pixelColor(0,0).alpha(),0);
        QCOMPARE(heart->pos().y(),qreal(140));
        for(const auto& size : {QSize(825,600),QSize(1280,720),QSize(1920,1080),QSize(2560,1440)}) {
            root.resize(size); QTest::qWait(20);
            const auto canvas=view->mapFromScene(QRectF(0,0,1650,900)).boundingRect();
            const auto visibleHeart=view->mapFromScene(heart->sceneBoundingRect()).boundingRect();
            QVERIFY(canvas.contains(visibleHeart)); QVERIFY(view->viewport()->rect().contains(visibleHeart));
        }
        root.setFullScreenEnabled(true); QTest::qWait(40);
        QVERIFY(view->mapFromScene(QRectF(0,0,1650,900)).boundingRect().contains(view->mapFromScene(heart->sceneBoundingRect()).boundingRect()));
        QVERIFY(view->viewport()->rect().contains(view->mapFromScene(heart->sceneBoundingRect()).boundingRect()));
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+"/cartoon-heart-fullscreen.png"));
        QMetaObject::invokeMethod(heart->findChild<QTimer*>(),"timeout");
        auto *blink=heart->findChild<QPropertyAnimation*>("heartBlinkAnimation");
        blink->setCurrentTime(blink->totalDuration());
        for(auto *animation : heart->findChildren<QPropertyAnimation*>())
            if(animation->propertyName()=="opacity") QCOMPARE(animation->startValue().toReal(),qreal(1));
    }
    void skyHeartsFallSlowlyAndRemainCollectable() {
        PlayScene play(3,nullptr,false); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *view=play.findChild<QGraphicsView*>();
        scene->generateSkyHeart(); auto *sky=scene->findChild<Heart*>(); QVERIFY(sky);
        auto *fall=sky->findChild<QPropertyAnimation*>("heartFallAnimation");
        QCOMPARE(fall->duration(),8000);
        fall->setCurrentTime(4000);
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
    void heartsBlinkTwiceBeforeDisappearingAndRemainCollectable() {
        PlayScene play(3,nullptr,false); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *view=play.findChild<QGraphicsView*>();
        scene->generateSkyHeart(); QPointer<Heart> heart=scene->findChild<Heart*>();
        auto *fall=heart->findChild<QPropertyAnimation*>("heartFallAnimation"); fall->setCurrentTime(fall->duration());
        auto *expiry=heart->findChild<QTimer*>(); QCOMPARE(expiry->interval(),3500); QVERIFY(expiry->isActive());
        expiry->stop(); QMetaObject::invokeMethod(expiry,"timeout");
        auto *blink=heart->findChild<QPropertyAnimation*>("heartBlinkAnimation");
        QCOMPARE(blink->loopCount(),2); QCOMPARE(blink->totalDuration(),1000);
        for(int time : {0,250,500,750}) {
            blink->setCurrentTime(time); QVERIFY(qAbs(heart->opacity()-(time%500==0 ? 1.0 : .2))<.01);
        }
        play.gamePaused(); const auto opacity=heart->opacity(); QTest::qWait(90);
        QCOMPARE(blink->state(),QAbstractAnimation::Paused); QCOMPARE(heart->opacity(),opacity);
        play.gameContinued(); QCOMPARE(blink->state(),QAbstractAnimation::Running);
        const int resources=scene->getRestHeart();
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(heart->sceneBoundingRect().center()));
        QCOMPARE(blink->state(),QAbstractAnimation::Stopped); QCOMPARE(heart->opacity(),qreal(1));
        QTRY_COMPARE_WITH_TIMEOUT(scene->getRestHeart(),resources+25,1200); QVERIFY(heart.isNull());
        scene->generateSkyHeart(); heart=scene->findChild<Heart*>(); QSignalSpy gone(heart,&Heart::i_have_disappeared);
        heart->findChild<QPropertyAnimation*>("heartFallAnimation")->stop();
        QMetaObject::invokeMethod(heart->findChild<QTimer*>(),"timeout");
        blink=heart->findChild<QPropertyAnimation*>("heartBlinkAnimation"); blink->setCurrentTime(blink->totalDuration());
        QTRY_COMPARE_WITH_TIMEOUT(gone.count(),1,1200); QVERIFY(heart.isNull());
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
    void endlessCyclesAndEnemyUnlocks() {
        const QVector<int> earlyWeights{1,1,3,4,10,3,3,5,6,20,5,5,7,8,30};
        int seen[61][3]={};
        for(int seed=1;seed<=500;++seed) {
            QRandomGenerator random(seed);
            for(int wave=1;wave<=60;++wave) {
                const auto enemies=WavePlanner::endlessWave(wave,random);
                const bool big=wave%5==0;
                QCOMPARE(WavePlanner::isEndlessBigWave(wave),big);
                const int cycle=(wave-1)/5;
                const int offsets[]={0,0,2,3};
                const int expected=wave<=15 ? earlyWeights[wave-1] : big ? (cycle+1)*10 : 1+cycle*2+offsets[(wave-1)%5];
                QCOMPARE(WavePlanner::endlessWaveWeight(wave),expected);
                int weight=0;
                for(int type : enemies) {
                    QVERIFY(type>=0 && type<3); ++seen[wave][type];
                    weight+=GameCatalog::enemies()[type].weight;
                    if(wave<5) QCOMPARE(type,0);
                }
                QCOMPARE(weight,expected);
                if(wave<=2) QCOMPARE(enemies.size(),1);
                if(wave==10) QVERIFY(enemies.count(1)>=1);
            }
        }
        for(int wave=5;wave<=60;++wave) {
            QVERIFY(seen[wave][1]>0); QVERIFY(seen[wave][2]>0);
        }
    }
    void endlessProbabilitiesSettleAtFifteen() {
        for(int type : {1,2}) {
            QCOMPARE(WavePlanner::endlessEnemyLikelihood(type,4),0.0);
            double previous=0;
            for(int wave=5;wave<=15;++wave) {
                const double chance=WavePlanner::endlessEnemyLikelihood(type,wave);
                QVERIFY(chance>previous); previous=chance;
                QVERIFY(qAbs(chance-3.0*(wave-4)/11)<.000001);
            }
            QCOMPARE(previous,3.0);
        }
        for(int wave=15;wave<=1000;++wave) {
            QCOMPARE(WavePlanner::endlessEnemyLikelihood(0,wave),4.0);
            QCOMPARE(WavePlanner::endlessEnemyLikelihood(1,wave),3.0);
            QCOMPARE(WavePlanner::endlessEnemyLikelihood(2,wave),3.0);
        }
    }
    void endlessIntervalsAndSpawnWindows() {
        int previous=20001;
        for(int wave=1;wave<=100;++wave) {
            const bool big=wave%5==0;
            const int expected=(20000-500*qBound(0,wave-5,20))*(big ? 2 : 1);
            QCOMPARE(WavePlanner::endlessIntervalAfterWave(wave),expected);
            const int base=expected/(big ? 2 : 1);
            QVERIFY(base<=previous); previous=base;
            if(wave>=25) QCOMPARE(expected,big ? 20000 : 10000);
        }
        QRandomGenerator random(71);
        for(int wave : {1,4,5,10,25,60,500}) {
            for(int seed=0;seed<100;++seed) {
                const auto roster=WavePlanner::endlessWave(wave,random);
                const auto delays=WavePlanner::endlessSpawnDelays(wave,roster.size(),random);
                QCOMPARE(delays.size(),roster.size()); QCOMPARE(delays[0],0);
                int duration=0;
                for(int i=1;i<delays.size();++i) { QVERIFY(delays[i]>0); duration+=delays[i]; }
                if(delays.size()==1) QCOMPARE(duration,0);
                else {
                    const bool big=wave%5==0;
                    QVERIFY(duration>=(big ? 8000 : 3000));
                    QVERIFY(duration<=(big ? 12000 : 6000));
                }
            }
        }
        QVERIFY(WavePlanner::endlessSpawnDelays(5,0,random).isEmpty());
    }
    void endlessSpawnsThenCountsDownWithLiveEnemies() {
        PlayScene play(10,nullptr,false,true,3); play.setSpeedMultiplier(2);
        auto *scene=play.findChild<MyGameScene*>();
        auto *timer=scene->findChild<GameTimer*>("waveTimer");
        auto *stagger=scene->findChild<GameTimer*>("waveStaggerTimer");
        QCOMPARE(timer->gameInterval(),28000);
        QSignalSpy waves(scene,&MyGameScene::waveStarted),win(scene,&MyGameScene::gameWin);
        QElapsedTimer elapsed; elapsed.start();
        QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection);
        QCOMPARE(scene->findChildren<YellowDogs*>().size(),1); QVERIFY(!timer->isActive());
        QTest::qWait(50); const int remaining=stagger->remainingTime(); play.gamePaused();
        const auto positions=scene->findChildren<YellowDogs*>(); const auto position=positions[0]->pos();
        QVERIFY(!stagger->isActive()); QTest::qWait(80);
        QCOMPARE(scene->findChildren<YellowDogs*>().size(),positions.size()); QCOMPARE(positions[0]->pos(),position);
        play.gameContinued(); QVERIFY(stagger->isActive());
        QVERIFY(qAbs(stagger->remainingTime()-remaining)<50);
        QTRY_COMPARE_WITH_TIMEOUT(scene->findChildren<YellowDogs*>().size(),3,3500);
        QVERIFY(elapsed.elapsed()>=1400); QVERIFY(elapsed.elapsed()<3400);
        QVERIFY(!stagger->isActive()); QVERIFY(timer->isActive());
        QCOMPARE(timer->gameInterval(),20000); QCOMPARE(timer->interval(),10000);
        // A live preceding wave does not block the next one.
        timer->start(80);
        QTRY_COMPARE_WITH_TIMEOUT(scene->wavesStarted(),4,300);
        QCOMPARE(waves.count(),2); QCOMPARE(scene->findChildren<YellowDogs*>().size(),4);
        QVERIFY(!timer->isActive());
        while(stagger->isActive()) QMetaObject::invokeMethod(stagger,"timeout",Qt::DirectConnection);
        QCOMPARE(scene->findChildren<YellowDogs*>().size(),7); QVERIFY(timer->isActive());
        const int rest=timer->remainingTime();
        play.gamePaused(); play.setSpeedMultiplier(1); QTest::qWait(50); QVERIFY(!timer->isActive());
        play.gameContinued(); QVERIFY(qAbs(timer->remainingTime()-rest*2)<60);
        // Clearing enemies does not restart or shorten the already running rest.
        for(auto *enemy : scene->findChildren<YellowDogs*>()) enemy->getAttacked(100000);
        QTRY_VERIFY_WITH_TIMEOUT(scene->findChildren<YellowDogs*>().isEmpty(),1000);
        QCOMPARE(timer->gameInterval(),20000); QVERIFY(timer->remainingTime()<rest*2-400);
        QCOMPARE(win.count(),0);
        scene->gameLose(); QVERIFY(!timer->isActive()); QVERIFY(!stagger->isActive());
    }
    void endlessEnemyStatsStopGrowing_data() {
        QTest::addColumn<int>("wave");
        for(int wave : {15,16,20,1000}) QTest::newRow(qPrintable(QString::number(wave))) << wave;
    }
    void endlessEnemyStatsStopGrowing() {
        QFETCH(int,wave);
        MyGameScene scene(10,nullptr,true,true,wave);
        const int health[]={636,1018,1208},damage[]={68,81,108};
        for(int type=0;type<3;++type) {
            YellowDogs enemy(2,&scene,type,wave); QCOMPARE(enemy.getHp(),health[type]);
            WallWhite defender; const int before=defender.getHp(); enemy.startAttacking(&defender);
            QCOMPARE(before-defender.getHp(),damage[type]);
            enemy.startMoving(); auto *walk=enemy.findChild<QPropertyAnimation*>("enemyMovement"); QVERIFY(walk);
            const qreal speed=walk->startValue().toPointF().x()-walk->endValue().toPointF().x();
            const auto& stats=GameCatalog::enemies()[type];
            QVERIFY(speed>=stats.minSpeed*1.14-.0001 && speed<=(stats.minSpeed+stats.speedRange-1)*1.14+.0001);
            if(type==2) {
                enemy.getAttacked(health[type]/2+1);
                QVERIFY(qAbs(walk->startValue().toPointF().x()-walk->endValue().toPointF().x()-speed*2.0)<.0001);
            }
        }
    }
    void endlessWaveAnnouncements_data() {
        QTest::addColumn<int>("wave"); QTest::addColumn<bool>("big");
        QTest::newRow("small-four") << 4 << false;
        QTest::newRow("first-big") << 5 << true;
        QTest::newRow("second-big") << 10 << true;
        QTest::newRow("settled-big") << 15 << true;
        QTest::newRow("settled-small") << 16 << false;
        QTest::newRow("fourth-big") << 20 << true;
        QTest::newRow("growing-big") << 25 << true;
    }
    void endlessWaveAnnouncements() {
        QFETCH(int,wave); QFETCH(bool,big);
        PlayScene play(10,nullptr,false,true,wave); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *view=play.findChild<QGraphicsView*>();
        auto *timer=scene->findChild<QTimer*>("waveTimer"),*stagger=scene->findChild<QTimer*>("waveStaggerTimer");
        auto *banner=play.findChild<BattleBanner*>(); auto *animation=banner->findChild<QVariantAnimation*>("bannerAnimation");
        QSignalSpy warning(scene,&MyGameScene::bigWaveApproaching),final(scene,&MyGameScene::finalWaveApproaching),win(scene,&MyGameScene::gameWin);
        const auto before=play.grab().toImage();
        QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection);
        QCOMPARE(scene->wavesStarted(),wave); QVERIFY(!timer->isActive());
        QCOMPARE(warning.count(),big ? 1 : 0); QCOMPARE(final.count(),0);
        if(big) {
            QVERIFY(scene->findChildren<YellowDogs*>().isEmpty());
            QVERIFY(banner->isVisible()); QCOMPARE(banner->accessibleName(),QString("一大波小金毛即将来袭"));
            QVERIFY(banner->testAttribute(Qt::WA_TransparentForMouseEvents));
            QVERIFY(scene->getGameTimer()->isActive()); QCOMPARE(stagger->interval(),1800);
            animation->setCurrentTime(450);
            QCOMPARE(play.grab().toImage().pixelColor(1000,700),before.pixelColor(1000,700));
            const auto point=view->viewport()->mapTo(&play,view->mapFromScene(QPointF(805,450)));
            auto *target=play.childAt(point); QCOMPARE(target,view->viewport());
            auto *card=play.findChild<Card*>("plantCard0"); emit card->cooldownFinished();
            QTest::mouseClick(card,Qt::LeftButton); QCOMPARE(Card::currentState(),GameState::PrePlace);
            QTest::mouseClick(target,Qt::LeftButton,Qt::NoModifier,target->mapFrom(&play,point));
            QCOMPARE(scene->findChildren<WhiteDogs*>().size(),1);
            scene->toggleShovel();
            QTest::mouseClick(target,Qt::LeftButton,Qt::NoModifier,target->mapFrom(&play,point));
            QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
            QVERIFY(scene->findChildren<WhiteDogs*>().isEmpty());
            const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
            if(!folder.isEmpty()) QVERIFY(play.grab().save(folder+QString("/endless-big-wave-%1.png").arg(wave)));
            const int remaining=stagger->remainingTime(); play.gamePaused();
            const int bannerTime=animation->currentTime(); QTest::qWait(80);
            QCOMPARE(animation->currentTime(),bannerTime); QVERIFY(!stagger->isActive());
            QVERIFY(scene->findChildren<YellowDogs*>().isEmpty());
            play.gameContinued(); QVERIFY(stagger->isActive());
            QVERIFY(qAbs(stagger->remainingTime()-remaining)<50);
            QMetaObject::invokeMethod(stagger,"timeout",Qt::DirectConnection);
            QCOMPARE(scene->findChildren<YellowDogs*>().size(),1); QVERIFY(stagger->interval()>0);
        } else {
            QVERIFY(!banner->isVisible()); QCOMPARE(scene->findChildren<YellowDogs*>().size(),1);
            QVERIFY(stagger->interval()>0);
        }
        int spawnDuration=0;
        while(stagger->isActive()) {
            QVERIFY(!timer->isActive());
            spawnDuration+=scene->findChild<GameTimer*>("waveStaggerTimer")->gameInterval();
            QMetaObject::invokeMethod(stagger,"timeout",Qt::DirectConnection);
        }
        QVERIFY(spawnDuration>=(big ? 8000 : 3000));
        QVERIFY(spawnDuration<=(big ? 12000 : 6000));
        const auto enemies=scene->findChildren<YellowDogs*>();
        int guitars=0,weight=0;
        for(auto *enemy : enemies) {
            const int type=enemy->typeIndex(); if(type==1) ++guitars;
            weight+=GameCatalog::enemies()[type].weight;
            if(wave<5) QCOMPARE(type,0);
            QCOMPARE(enemy->getHp(),qRound(GameCatalog::enemies()[type].health*(1+.08*(qMin(wave,15)-1))));
        }
        if(wave==10) QVERIFY(guitars>=1);
        QCOMPARE(weight,WavePlanner::endlessWaveWeight(wave));
        QVERIFY(timer->isActive()); QCOMPARE(timer->interval(),WavePlanner::endlessIntervalAfterWave(wave));
        if(big) animation->setCurrentTime(animation->duration());
        QVERIFY(!banner->isVisible());
        for(auto *enemy : enemies) enemy->getAttacked(100000);
        QTRY_VERIFY_WITH_TIMEOUT(scene->findChildren<YellowDogs*>().isEmpty(),1000);
        QVERIFY(timer->isActive()); QCOMPARE(timer->interval(),WavePlanner::endlessIntervalAfterWave(wave));
        QCOMPARE(win.count(),0); QCOMPARE(final.count(),0);
    }
    void endlessCheckpointRetainsBigWaveStage_data() {
        QTest::addColumn<int>("wave");
        QTest::newRow("guitar-checkpoint") << 10;
        QTest::newRow("settled-checkpoint") << 25;
    }
    void endlessCheckpointRetainsBigWaveStage() {
        QFETCH(int,wave);
        QTemporaryDir dir; const auto path=unlockedPath(dir);
        ProgressStore saved(path); QVERIFY(saved.startEndless()); QVERIFY(saved.recordEndlessWave(wave));
        GameWindow root(nullptr,path,false); root.show(); root.startEndless();
        auto *play=root.playPage(); QVERIFY(play); QVERIFY(play->isPaused());
        auto *scene=play->findChild<MyGameScene*>(); auto *banner=play->findChild<BattleBanner*>();
        auto *timer=scene->findChild<QTimer*>("waveTimer"),*stagger=scene->findChild<QTimer*>("waveStaggerTimer");
        QCOMPARE(scene->wavesStarted(),wave-1); QVERIFY(!timer->isActive());
        play->gameContinued(); QSignalSpy warning(scene,&MyGameScene::bigWaveApproaching);
        QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection);
        QCOMPARE(scene->wavesStarted(),wave); QCOMPARE(warning.count(),1); QVERIFY(banner->isVisible());
        QCOMPARE(banner->accessibleName(),QString("一大波小金毛即将来袭"));
        QVERIFY(scene->findChildren<YellowDogs*>().isEmpty()); QCOMPARE(stagger->interval(),1800);
        QCOMPARE(ProgressStore(path).endlessCheckpoint(),wave); QCOMPARE(ProgressStore(path).endlessBest(),wave);
        while(stagger->isActive()) QMetaObject::invokeMethod(stagger,"timeout",Qt::DirectConnection);
        const auto enemies=scene->findChildren<YellowDogs*>();
        int weight=0; for(auto *enemy : enemies) weight+=GameCatalog::enemies()[enemy->typeIndex()].weight;
        QCOMPARE(weight,WavePlanner::endlessWaveWeight(wave));
        QVERIFY(timer->isActive()); QCOMPARE(timer->interval(),WavePlanner::endlessIntervalAfterWave(wave));
        if(wave==10) {
            int guitars=0; for(auto *enemy : enemies) if(enemy->typeIndex()==1) ++guitars;
            QVERIFY(guitars>=1);
        }
        root.close(); QVERIFY(!banner->isVisible()); QVERIFY(!stagger->isActive()); QVERIFY(!timer->isActive());
    }
    void endlessWavesAndResume() {
        QRandomGenerator random(7);
        for(int wave=1;wave<=20;++wave) {
            const auto enemies=WavePlanner::endlessWave(wave,random);
            int weight=0; for(int type : enemies) weight+=GameCatalog::enemies()[type].weight;
            QCOMPARE(weight,WavePlanner::endlessWaveWeight(wave));
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
            QCOMPARE(scene->wavesStarted(),wave); QVERIFY(waveTimer->isActive());
            for(auto *enemy : scene->findChildren<YellowDogs*>()) {
                QVERIFY(enemy->getHp()>=GameCatalog::enemies()[enemy->typeIndex()].health);
                enemy->getAttacked(100000);
            }
            QTRY_VERIFY_WITH_TIMEOUT(scene->findChildren<YellowDogs*>().isEmpty(),1200);
            QVERIFY(waveTimer->isActive());
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
        QCOMPARE(sounds.size(),16);
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
    void closedBattleRestoresBoardPaused_data() {
        QTest::addColumn<bool>("endless"); QTest::addColumn<bool>("closePaused");
        QTest::newRow("campaign-playing") << false << false;
        QTest::newRow("campaign-paused") << false << true;
        QTest::newRow("endless-playing") << true << false;
        QTest::newRow("endless-paused") << true << true;
    }
    void closedBattleRestoresBoardPaused() {
        QFETCH(bool,endless); QFETCH(bool,closePaused);
        QTemporaryDir dir; const auto path=unlockedPath(dir);
        QJsonObject snapshot;
        {
            GameWindow root(nullptr,path,false); root.show();
            if(endless) root.startEndless(); else root.startLevel(8);
            auto *play=root.playPage(); auto *scene=play->findChild<MyGameScene*>();
            auto *view=play->findChild<QGraphicsView*>(); scene->addHeart(10000); play->setSpeedMultiplier(2);
            for(int type=0;type<8;++type) {
                scene->setChosenNum(type); Card::setGameState(GameState::PrePlace);
                QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,
                    view->mapFromScene(QPointF(440+121*(type%4),202+145*(type/4))));
            }
            QCOMPARE(scene->findChildren<WhiteDogs*>().size(),8);
            auto *wall=scene->findChild<WallWhite*>(); wall->cutHp(123);
            auto *wave=scene->findChild<GameTimer*>("waveTimer");
            auto *stagger=scene->findChild<GameTimer*>("waveStaggerTimer");
            QMetaObject::invokeMethod(wave,"timeout",Qt::DirectConnection);
            // Include all enemy types, an accelerated dash and a dying dog.
            scene->setAYellowDog(4,1); scene->setAYellowDog(3,2); scene->setAYellowDog(4,0);
            auto *dash=static_cast<YellowDogs*>(scene->getZombieMap(3).last()); dash->getAttacked(dash->getHp()/2+1);
            static_cast<YellowDogs*>(scene->getZombieMap(4).last())->getAttacked(100000);
            scene->generateSkyHeart(); scene->generateWhiteHeart(QPointF(850,350));
            scene->generateBullet(2,2); new EnemyProjectile(scene,4,QPointF(1400,650),81);
            scene->togglePlantHealth(); scene->toggleEnemyHealth(); QTest::qWait(65);
            // Select an available card, then close from playing or paused.
            auto *card=play->findChild<Card*>("plantCard1"); emit card->cooldownFinished();
            QTest::mouseClick(card,Qt::LeftButton); QCOMPARE(Card::currentState(),GameState::PrePlace);
            if(closePaused) play->showPauseMenu();
            QVERIFY(root.close()); snapshot=ProgressStore(path).battleSnapshot();
            QVERIFY2(BattleSnapshot::isValid(snapshot),qPrintable(QString::fromUtf8(QJsonDocument(snapshot).toJson())));
            QVERIFY(!snapshot.isEmpty());
            QVERIFY(!wave->isActive()); QVERIFY(!stagger->isActive());
        }
        // Rebuild all objects from the disk file, with no in-memory page reuse.
        GameWindow restored(nullptr,path,true); restored.show();
        if(endless) restored.startEndless(); else restored.continueGame();
        auto *play=restored.playPage(); QVERIFY(play); QVERIFY(play->isPaused());
        QVERIFY(play->findChild<PauseDialog*>()->isVisible()); QCOMPARE(play->speedMultiplier(),2);
        QVERIFY(!play->findChild<LevelOpening*>());
        auto *scene=play->findChild<MyGameScene*>(); QCOMPARE(scene->wavesStarted(),snapshot["scene"].toObject()["nextWave"].toInt());
        QCOMPARE(scene->getRestHeart(),snapshot["scene"].toObject()["hearts"].toInt());
        QCOMPARE(scene->findChild<WallWhite*>()->getHp(),3877);
        const auto again=BattleSnapshot::capture(*play);
        for(const auto& key : {"plants","enemies","hearts","bullets","notes"}) {
            const auto before=snapshot[key].toArray(),after=again[key].toArray(); QCOMPARE(after.size(),before.size());
            for(int i=0;i<before.size();++i) {
                QCOMPARE(after[i].toObject()["pos"],before[i].toObject()["pos"]);
                QCOMPARE(after[i].toObject()["hp"],before[i].toObject()["hp"]);
                QCOMPARE(after[i].toObject()["type"],before[i].toObject()["type"]);
            }
        }
        QCOMPARE(again["cards"].toArray()[0].toObject()["progress"],snapshot["cards"].toArray()[0].toObject()["progress"]);
        auto *enemy=scene->getZombieMap(3).last(); const auto pos=enemy->pos(); const int hearts=scene->getRestHeart();
        QTest::qWait(150); QCOMPARE(enemy->pos(),pos); QCOMPARE(scene->getRestHeart(),hearts);
        for(auto *timer : play->findChildren<GameTimer*>()) QVERIFY(!timer->isActive());
        const auto frozen=BattleSnapshot::capture(*play);
        QCOMPARE(frozen["scene"].toObject()["pendingIndex"],again["scene"].toObject()["pendingIndex"]);
        play->gameContinued(); QCOMPARE(Card::currentState(),GameState::PrePlace);
        QTRY_VERIFY_WITH_TIMEOUT(enemy->x()<pos.x(),500);
        QTRY_VERIFY_WITH_TIMEOUT(scene->findChildren<YellowDogs*>().size()<snapshot["enemies"].toArray().size(),1000);
        QVERIFY(scene->findChild<Heart*>()->findChild<QPropertyAnimation*>("heartFallAnimation")->state()==QAbstractAnimation::Running);
        // Pending units and rest retain their saved remaining timing.
        auto *stagger=scene->findChild<GameTimer*>("waveStaggerTimer");
        while(stagger->isActive()) QMetaObject::invokeMethod(stagger,"timeout",Qt::DirectConnection);
        QCOMPARE(scene->wavesStarted(),snapshot["scene"].toObject()["nextWave"].toInt());
        if(endless) QVERIFY(scene->findChild<GameTimer*>("waveTimer")->isActive());
        QVERIFY(restored.close()); QVERIFY(!ProgressStore(path).battleSnapshot().isEmpty());
    }
    void battleSnapshotRejectsCorruptionAndClearsOnNewGame() {
        QTemporaryDir dir; const auto path=unlockedPath(dir);
        GameWindow root(nullptr,path,false); root.show(); root.startLevel(3); QVERIFY(root.close());
        ProgressStore store(path); const auto valid=store.battleSnapshot(); QVERIFY(BattleSnapshot::isValid(valid));
        auto broken=valid; auto scene=broken["scene"].toObject(); scene["pendingIndex"]=-1; broken["scene"]=scene;
        QVERIFY(!store.saveBattle(broken)); QCOMPARE(ProgressStore(path).battleSnapshot(),valid);
        broken=valid; auto cards=broken["cards"].toArray(); auto card=cards[0].toObject();
        card["activity"]=QJsonObject{{"timers",QJsonArray{}},{"animations",QJsonArray{}}}; cards[0]=card; broken["cards"]=cards;
        QVERIFY(!BattleSnapshot::isValid(broken));
        // A damaged optional snapshot keeps unlocked progress and the legacy checkpoint.
        QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly)); auto json=QJsonDocument::fromJson(file.readAll()).object(); file.close();
        json["battle"]=broken; QVERIFY(file.open(QIODevice::WriteOnly)); file.write(QJsonDocument(json).toJson()); file.close();
        ProgressStore damaged(path); QCOMPARE(damaged.highestCompleted(),10); QVERIFY(damaged.hasUnfinishedLevel());
        QVERIFY(damaged.battleSnapshot().isEmpty()); QVERIFY(damaged.saveBattle(valid));
        QVERIFY(damaged.unlockAll()); QCOMPARE(ProgressStore(path).battleSnapshot(),valid);
        QVERIFY(damaged.startLevel(2)); QVERIFY(ProgressStore(path).battleSnapshot().isEmpty());
        QVERIFY(damaged.reset()); QVERIFY(ProgressStore(path).battleSnapshot().isEmpty());
    }
    void savedHeartsKeepAllLifecycleStages() {
        PlayScene original(8,nullptr,false); auto *scene=original.findChild<MyGameScene*>();
        QVector<Heart*> hearts;
        for(int i=0;i<4;++i) {
            auto *heart=new Heart(QPointF(500+i*150,250),QPointF(500+i*150,450),scene,QEasingCurve::Linear,scene);
            scene->addItem(heart); hearts.append(heart);
        }
        hearts[0]->findChild<QPropertyAnimation*>("heartFallAnimation")->setCurrentTime(700);
        for(int i=1;i<3;++i) {
            auto *fall=hearts[i]->findChild<QPropertyAnimation*>("heartFallAnimation"); fall->setCurrentTime(fall->duration());
            QMetaObject::invokeMethod(hearts[i]->findChild<GameTimer*>(),"timeout",Qt::DirectConnection);
            auto *blink=hearts[i]->findChild<QPropertyAnimation*>("heartBlinkAnimation");
            blink->setCurrentTime(i==1 ? 250 : blink->duration()*2);
        }
        auto *fade=hearts[2]->findChild<QPropertyAnimation*>("heartFadeAnimation"); QVERIFY(fade); fade->setCurrentTime(250);
        Heart::curMousePos=hearts[3]->sceneBoundingRect().center(); scene->sceneClicked();
        auto *collect=hearts[3]->findChild<QPropertyAnimation*>("heartCollectAnimation"); QCOMPARE(collect->state(),QAbstractAnimation::Running);
        collect->setCurrentTime(200);
        const auto saved=BattleSnapshot::capture(original); QVERIFY(BattleSnapshot::isValid(saved));
        PlayScene restored(8,nullptr,false); QVERIFY(BattleSnapshot::restore(restored,saved));
        auto *board=restored.findChild<MyGameScene*>(); const auto loaded=board->findChildren<Heart*>(); QCOMPARE(loaded.size(),4);
        for(int i=0;i<4;++i) {
            QCOMPARE(loaded[i]->pos(),hearts[i]->pos()); QCOMPARE(loaded[i]->opacity(),hearts[i]->opacity());
            for(auto *a : loaded[i]->findChildren<QPropertyAnimation*>()) QVERIFY(a->state()!=QAbstractAnimation::Running);
        }
        const int before=board->getRestHeart(); restored.gameContinued();
        QTRY_COMPARE_WITH_TIMEOUT(board->getRestHeart(),before+25,900);
        QTRY_COMPARE_WITH_TIMEOUT(board->findChildren<Heart*>().size(),2,1400);
        QVERIFY(loaded[0]->findChild<QPropertyAnimation*>("heartFallAnimation")->state()==QAbstractAnimation::Running);
    }
    void restoredLastDeathFinishesCampaignOnce() {
        QTemporaryDir dir; const auto path=unlockedPath(dir);
        {
            GameWindow root(nullptr,path,false); root.show(); root.startLevel(3);
            auto *scene=root.playPage()->findChild<MyGameScene*>(); auto *wave=scene->findChild<GameTimer*>("waveTimer");
            auto *stagger=scene->findChild<GameTimer*>("waveStaggerTimer");
            while(scene->wavesStarted()<GameCatalog::level(3).waves) {
                QMetaObject::invokeMethod(wave,"timeout",Qt::DirectConnection);
                while(stagger->isActive()) QMetaObject::invokeMethod(stagger,"timeout",Qt::DirectConnection);
            }
            const auto enemies=scene->findChildren<YellowDogs*>();
            for(int i=0;i<enemies.size()-1;++i) enemies[i]->getAttacked(100000);
            QTRY_COMPARE_WITH_TIMEOUT(scene->findChildren<YellowDogs*>().size(),1,1000);
            auto *last=enemies.last(); last->getAttacked(100000);
            last->findChildren<QPropertyAnimation*>().last()->setCurrentTime(200);
            QVERIFY(root.close()); QVERIFY(!ProgressStore(path).battleSnapshot().isEmpty());
        }
        GameWindow root(nullptr,path,true); root.show(); root.continueGame(); auto *play=root.playPage();
        auto *scene=play->findChild<MyGameScene*>(); QSignalSpy win(scene,&MyGameScene::gameWin);
        QVERIFY(play->isPaused()); QCOMPARE(scene->findChildren<YellowDogs*>().size(),1);
        QVERIFY(scene->findChild<YellowDogs*>()->isDying());
        QTest::qWait(120); QCOMPARE(win.count(),0);
        play->gameContinued(); QTRY_COMPARE_WITH_TIMEOUT(win.count(),1,700);
        QTest::qWait(100); QCOMPARE(win.count(),1); QVERIFY(play->isFinished());
        QVERIFY(!ProgressStore(path).hasUnfinishedLevel()); QVERIFY(ProgressStore(path).battleSnapshot().isEmpty());
        QVERIFY(root.close()); QVERIFY(ProgressStore(path).battleSnapshot().isEmpty());
    }
    void restoredBigWaveRetainsWarningAndRemainingSpawnDelay() {
        QTemporaryDir dir; const auto path=unlockedPath(dir); ProgressStore store(path);
        QVERIFY(store.startEndless()); QVERIFY(store.recordEndlessWave(5));
        int remaining=0; QJsonObject saved;
        {
            GameWindow root(nullptr,path,false); root.show(); root.startEndless(); auto *play=root.playPage();
            play->gameContinued(); play->setSpeedMultiplier(2); auto *scene=play->findChild<MyGameScene*>();
            QMetaObject::invokeMethod(scene->findChild<GameTimer*>("waveTimer"),"timeout",Qt::DirectConnection);
            QTest::qWait(60); remaining=scene->findChild<GameTimer*>("waveStaggerTimer")->remainingTime();
            QVERIFY(root.close()); saved=ProgressStore(path).battleSnapshot();
        }
        GameWindow root(nullptr,path,true); root.show(); root.startEndless(); auto *play=root.playPage();
        auto *scene=play->findChild<MyGameScene*>(); auto *banner=play->findChild<BattleBanner*>();
        QVERIFY(play->isPaused()); QCOMPARE(scene->wavesStarted(),5); QVERIFY(scene->findChildren<YellowDogs*>().isEmpty());
        QVERIFY(banner->isVisible()); QCOMPARE(banner->accessibleName(),QString("一大波小金毛即将来袭"));
        auto *animation=banner->findChild<QVariantAnimation*>("bannerAnimation"); const int time=animation->currentTime();
        QTest::qWait(100); QCOMPARE(animation->currentTime(),time);
        play->gameContinued(); auto *stagger=scene->findChild<GameTimer*>("waveStaggerTimer");
        QVERIFY(qAbs(stagger->remainingTime()-remaining)<60);
        QTRY_COMPARE_WITH_TIMEOUT(scene->findChildren<YellowDogs*>().size(),1,1000);
        while(stagger->isActive()) QMetaObject::invokeMethod(stagger,"timeout",Qt::DirectConnection);
        int weight=0; for(auto *enemy : scene->findChildren<YellowDogs*>()) weight+=GameCatalog::enemies()[enemy->typeIndex()].weight;
        QCOMPARE(weight,10); QCOMPARE(scene->wavesStarted(),5);
        QCOMPARE(scene->findChild<GameTimer*>("waveTimer")->gameInterval(),40000);
        QVERIFY(root.close());
    }
    void failedBattleSaveKeepsWindowAndBattlePaused() {
        QTemporaryDir dir; const auto path=unlockedPath(dir); GameWindow root(nullptr,path,false);
        root.show(); root.startLevel(2); auto *play=root.playPage(); auto *scene=play->findChild<MyGameScene*>();
        scene->setAYellowDog(2); const auto pos=scene->getZombieMap(2).first()->pos();
        QVERIFY(QFile::remove(path)); QVERIFY(QDir().mkdir(path));
        QVERIFY(!root.close()); QVERIFY(root.isVisible()); QVERIFY(play->isPaused());
        QTRY_VERIFY_WITH_TIMEOUT(root.findChild<QDialog*>("battleSaveError"),500);
        auto *error=root.findChild<QDialog*>("battleSaveError"); QVERIFY(error->isVisible());
        QVERIFY(!error->findChild<QLabel*>()->text().isEmpty());
        QTest::mouseClick(error->findChild<QPushButton*>(),Qt::LeftButton);
        QCOMPARE(scene->getZombieMap(2).first()->pos(),pos);
        QVERIFY(QDir().rmdir(path)); QVERIFY(root.close()); QVERIFY(!ProgressStore(path).battleSnapshot().isEmpty());
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
                for(int index=0;index<plan.size();++index) {
                    const auto& wave=plan[index];
                    int sum=0;
                    for(int type : wave) {
                        QVERIFY(type<=stats.maxEnemyType);
                        sum+=GameCatalog::enemies()[type].weight;
                        if(level==10) ++counts[type];
                    }
                    if(index<stats.waveRules.size() && stats.waveRules[index].weight>0) {
                        const auto& rule=stats.waveRules[index];
                        QCOMPARE(sum,rule.weight);
                        QVERIFY(wave.count(1)>=rule.minGuitars); QVERIFY(wave.count(2)>=rule.minDashes);
                        if(rule.maxEnemyType>=0) for(int type : wave) QVERIFY(type<=rule.maxEnemyType);
                    } else if(level>=2 && level<=8 && index<2) {
                        const int count=index==0 ? 1 : qMax(1,stats.waveWeight-1);
                        QCOMPARE(wave.size(),count); QCOMPARE(sum,count);
                        for(int type : wave) QCOMPARE(type,0);
                    } else if(level>=2 && level<=8 && index>=stats.waves-2) {
                        if(level>=4 && level<=7) {
                            QCOMPARE(sum,index==stats.waves-1 ? stats.late.finalWeight : stats.late.penultimateWeight);
                            QVERIFY(wave.count(1)>=1);
                            for(int earlier=0;earlier<index;++earlier) QVERIFY(plan[earlier].size()<wave.size());
                            continue;
                        }
                        const int count=(level==2 ? stats.waveWeight : stats.waveWeight+1)+(index==stats.waves-1 ? 1 : 0);
                        const int budget=level>=7 ? qMax(stats.waveWeight,GameCatalog::enemies()[2].weight) : stats.waveWeight;
                        QCOMPARE(wave.size(),count); QVERIFY(sum>=count); QVERIFY(sum<=count+budget-1);
                        for(int earlier=0;earlier<index;++earlier) QVERIFY(plan[earlier].size()<wave.size());
                    } else QCOMPARE(sum,stats.waveWeight);
                }
            }
        }
        QVERIFY(counts[0]>counts[1]);
        QVERIFY(counts[1]>counts[2]);
    }
    void strongerLateWavesPreserveOpeningAndGuaranteeGuitars() {
        const int penultimate[]={8,9,10,12},final[]={10,11,12,14},total[]={24,29,34,38};
        for(int level=4;level<=7;++level) {
            int extraGuitars=0,middleGuitars=0,middleWaves=0;
            for(int seed=1;seed<=500;++seed) {
                QRandomGenerator random(seed); const auto plan=WavePlanner::create(level,random);
                QCOMPARE(plan[0],QVector<int>({0}));
                QCOMPARE(plan[1],QVector<int>(GameCatalog::level(level).waveWeight-1,0));
                int totalWeight=0;
                for(int wave=0;wave<plan.size();++wave) {
                    int weight=0; for(int type : plan[wave]) weight+=GameCatalog::enemies()[type].weight;
                    totalWeight+=weight;
                    if(wave>=plan.size()-2) {
                        QCOMPARE(weight,wave==plan.size()-1 ? final[level-4] : penultimate[level-4]);
                        QVERIFY(plan[wave].count(1)>=1); extraGuitars+=plan[wave].count(1)-1;
                        for(int earlier=0;earlier<wave;++earlier) QVERIFY(plan[earlier].size()<plan[wave].size());
                    } else if(wave>=2) { middleGuitars+=plan[wave].count(1); ++middleWaves; }
                }
                QCOMPARE(totalWeight,total[level-4]);
            }
            QVERIFY(extraGuitars>0); // The guarantee still allows additional weighted picks.
            QVERIFY(middleGuitars>middleWaves*.23); // Previous eligible draw probability was about 1/6.
            QCOMPARE(WavePlanner::enemyLikelihood(1,level),.45);
        }
        QCOMPARE(WavePlanner::enemyLikelihood(1,8),.60);
    }
    void strongerLateWavesSpawnWithLongerFinalGap() {
        const int penultimate[]={8,9,10,12},final[]={10,11,12,14};
        const int minGap[]={35000,38000,40000,30000},maxGap[]={42000,45000,48000,36000};
        for(int level=4;level<=7;++level) {
            MyGameScene scene(level); auto *waveTimer=scene.findChild<QTimer*>("waveTimer");
            auto *stagger=scene.findChild<QTimer*>("waveStaggerTimer"); const int waves=GameCatalog::level(level).waves;
            int previous=0;
            for(int wave=1;wave<=waves;++wave) {
                QMetaObject::invokeMethod(waveTimer,"timeout");
                if(wave==waves-1) {
                    QVERIFY(waveTimer->interval()>=minGap[level-4] && waveTimer->interval()<=maxGap[level-4]);
                    QVERIFY(waveTimer->interval()>=GameCatalog::level(level).maxInterval);
                    const int remaining=waveTimer->remainingTime(); GamePause pause; pause.pause(&scene);
                    QTest::qWait(70); QVERIFY(!waveTimer->isActive()); pause.resume();
                    QVERIFY(qAbs(waveTimer->remainingTime()-remaining)<50);
                } else if(level==7 && (wave==2 || wave==3)) {
                    QVERIFY(waveTimer->interval()>=18000 && waveTimer->interval()<=22000);
                } else if(level==7 && wave==4) {
                    QVERIFY(waveTimer->interval()>=20000 && waveTimer->interval()<=24000);
                } else if(wave<waves) {
                    QVERIFY(waveTimer->interval()>=GameCatalog::level(level).minInterval);
                    QVERIFY(waveTimer->interval()<=GameCatalog::level(level).maxInterval);
                }
                while(stagger->isActive()) QMetaObject::invokeMethod(stagger,"timeout");
                const auto enemies=scene.findChildren<YellowDogs*>(); int weight=0,guitars=0;
                for(int i=previous;i<enemies.size();++i) {
                    const int type=enemies[i]->typeIndex(); weight+=GameCatalog::enemies()[type].weight;
                    if(type==1) ++guitars;
                }
                if(wave>=waves-1) {
                    QCOMPARE(weight,wave==waves ? final[level-4] : penultimate[level-4]); QVERIFY(guitars>=1);
                } else if(wave<=2) QCOMPARE(guitars,0);
                previous=enemies.size();
            }
            QCOMPARE(previous,scene.totalEnemies()); QVERIFY(!waveTimer->isActive());
        }
    }
    void campaignRarityKeepsEndlessProbabilities() {
        for(int level=4;level<=7;++level) QCOMPARE(WavePlanner::enemyLikelihood(1,level),.45);
        QCOMPARE(WavePlanner::enemyLikelihood(1,8),.60);
        QCOMPARE(WavePlanner::enemyLikelihood(1,9),1.20);
        QCOMPARE(WavePlanner::enemyLikelihood(1,10),.30);
        for(int level=1;level<=8;++level) QCOMPARE(WavePlanner::enemyLikelihood(2,level),.08);
        QCOMPARE(WavePlanner::enemyLikelihood(2,9),.10);
        QCOMPARE(WavePlanner::enemyLikelihood(2,10),.15);
        QCOMPARE(WavePlanner::enemyLikelihood(0),1.0);
        QCOMPARE(WavePlanner::enemyLikelihood(1),.20);
        QCOMPARE(WavePlanner::enemyLikelihood(2),.08);
    }
    void configuredCampaignWaves_data() {
        QTest::addColumn<int>("level");
        for(const char *column : {"weights","guitars","dashes","maxTypes","minGaps","maxGaps"})
            QTest::addColumn<QVector<int>>(column);
        QTest::newRow("level-seven") << 7
            << QVector<int>({1,3,4,4,12,14})
            << QVector<int>({0,0,0,0,1,1}) << QVector<int>({0,0,0,0,0,0})
            << QVector<int>({0,0,2,2,2,2})
            << QVector<int>({22000,25000,18000,18000,20000,30000})
            << QVector<int>({22000,30000,22000,22000,24000,36000});
        QTest::newRow("level-eight") << 8
            << QVector<int>({1,2,3,5,7,12,15})
            << QVector<int>({0,0,0,1,1,0,0}) << QVector<int>({0,0,0,0,0,1,1})
            << QVector<int>({0,0,2,2,2,2,2})
            << QVector<int>({25000,25000,22000,22000,22000,30000,34000})
            << QVector<int>({25000,30000,28000,28000,28000,36000,40000});
        QTest::newRow("level-nine") << 9
            << QVector<int>({1,2,3,6,10,16,20})
            << QVector<int>({0,0,0,1,1,0,0}) << QVector<int>({0,0,0,0,0,2,2})
            << QVector<int>({0,0,0,2,2,2,2})
            << QVector<int>({25000,27000,24000,24000,24000,34000,38000})
            << QVector<int>({25000,32000,30000,30000,30000,40000,44000});
        QTest::newRow("level-ten") << 10
            << QVector<int>({1,1,2,7,11,15,19,23})
            << QVector<int>({0,0,0,1,1,1,1,1}) << QVector<int>({0,0,0,0,0,1,1,1})
            << QVector<int>({0,0,0,2,2,2,2,2})
            << QVector<int>({28000,27000,24000,24000,24000,24000,36000,40000})
            << QVector<int>({28000,32000,30000,30000,30000,30000,42000,46000});
    }
    void configuredCampaignWaves() {
        QFETCH(int,level); QFETCH(QVector<int>,weights); QFETCH(QVector<int>,guitars);
        QFETCH(QVector<int>,dashes); QFETCH(QVector<int>,maxTypes);
        QFETCH(QVector<int>,minGaps); QFETCH(QVector<int>,maxGaps);
        for(int seed=1;seed<=1000;++seed) {
            QRandomGenerator random(seed); const auto plan=WavePlanner::create(level,random);
            QCOMPARE(plan.size(),weights.size());
            int total=0;
            for(int wave=0;wave<plan.size();++wave) {
                int weight=0;
                for(int type : plan[wave]) { QVERIFY(type<=maxTypes[wave]); weight+=GameCatalog::enemies()[type].weight; }
                QCOMPARE(weight,weights[wave]);
                QVERIFY(plan[wave].count(1)>=guitars[wave]); QVERIFY(plan[wave].count(2)>=dashes[wave]);
                total+=plan[wave].size();
                QCOMPARE(WavePlanner::intervalBeforeWave(level,wave+1),qMakePair(minGaps[wave],maxGaps[wave]));
            }
            QCOMPARE(WavePlanner::enemyCount(plan),total);
        }
    }
    void configuredWavesSpawnAndPreserveGaps_data() { configuredCampaignWaves_data(); }
    void configuredWavesSpawnAndPreserveGaps() {
        QFETCH(int,level); QFETCH(QVector<int>,weights); QFETCH(QVector<int>,guitars);
        QFETCH(QVector<int>,dashes); QFETCH(QVector<int>,maxTypes);
        QFETCH(QVector<int>,minGaps); QFETCH(QVector<int>,maxGaps);
        MyGameScene scene(level); auto *timer=scene.findChild<QTimer*>("waveTimer");
        auto *stagger=scene.findChild<QTimer*>("waveStaggerTimer");
        QSignalSpy warning(&scene,&MyGameScene::finalWaveApproaching),win(&scene,&MyGameScene::gameWin);
        QCOMPARE(timer->interval(),minGaps[0]);
        int previous=0;
        for(int wave=0;wave<weights.size();++wave) {
            QVERIFY(QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection));
            if(wave==weights.size()-1) { QCOMPARE(warning.count(),1); QCOMPARE(stagger->interval(),1800); }
            else {
                QVERIFY(timer->interval()>=minGaps[wave+1] && timer->interval()<=maxGaps[wave+1]);
                QVERIFY(timer->isActive());
            }
            while(stagger->isActive()) QMetaObject::invokeMethod(stagger,"timeout",Qt::DirectConnection);
            const auto enemies=scene.findChildren<YellowDogs*>(); int weight=0,guitarCount=0,dashCount=0;
            for(int i=previous;i<enemies.size();++i) {
                const int type=enemies[i]->typeIndex(); QVERIFY(type<=maxTypes[wave]);
                weight+=GameCatalog::enemies()[type].weight;
                if(type==1) ++guitarCount; if(type==2) ++dashCount;
            }
            QCOMPARE(weight,weights[wave]); QVERIFY(guitarCount>=guitars[wave]); QVERIFY(dashCount>=dashes[wave]);
            previous=enemies.size();
            if(wave==2) {
                const int remaining=timer->remainingTime(); GamePause pause; pause.pause(&scene);
                QTest::qWait(70); QVERIFY(!timer->isActive()); pause.resume();
                QVERIFY(qAbs(timer->remainingTime()-remaining)<50);
            }
        }
        QCOMPARE(previous,scene.totalEnemies()); QVERIFY(!timer->isActive()); QVERIFY(!stagger->isActive());
        for(auto *enemy : scene.findChildren<YellowDogs*>()) enemy->getAttacked(10000);
        QTRY_COMPARE_WITH_TIMEOUT(win.count(),1,1000);
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
            QCOMPARE(sum,GameCatalog::level(10).waveRules[i].weight);
            previous=enemies.size();
        }
        QCOMPARE(started.count(),GameCatalog::level(10).waves);
        QCOMPARE(previous,scene.totalEnemies());
        QVERIFY(!waveTimer->isActive()); QVERIFY(!stagger->isActive());
    }
    void gradualWavesSpawnTheirPlannedCounts() {
        for(int level=2;level<=8;++level) {
            MyGameScene scene(level); auto *waveTimer=scene.findChild<QTimer*>("waveTimer");
            auto *stagger=scene.findChild<QTimer*>("waveStaggerTimer");
            int previous=0,largest=0;
            for(int wave=0;wave<GameCatalog::level(level).waves;++wave) {
                QMetaObject::invokeMethod(waveTimer,"timeout");
                while(stagger->isActive()) QMetaObject::invokeMethod(stagger,"timeout");
                const int spawned=scene.findChildren<YellowDogs*>().size()-previous;
                if(wave==0) QCOMPARE(spawned,1);
                if(level==2) QCOMPARE(spawned,QVector<int>({1,1,2,3}).at(wave));
                if(level<=7 && wave>=GameCatalog::level(level).waves-2) QVERIFY(spawned>largest);
                largest=qMax(largest,spawned); previous+=spawned;
            }
            QCOMPARE(previous,scene.totalEnemies()); QVERIFY(!waveTimer->isActive()); QVERIFY(!stagger->isActive());
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
    void guitarProjectileMatchesBiteDamage_data() {
        QTest::addColumn<int>("wave"); QTest::addColumn<int>("plantType");
        for(int wave : {1,6}) for(int type : {2,4,7})
            QTest::newRow(qPrintable(QString("wave-%1-plant-%2").arg(wave).arg(type))) << wave << type;
    }
    void guitarProjectileMatchesBiteDamage() {
        QFETCH(int,wave); QFETCH(int,plantType);
        PlayScene play(10,nullptr,false,true,wave); play.show();
        auto *scene=play.findChild<MyGameScene*>(); auto *view=play.findChild<QGraphicsView*>();
        for(auto *timer : scene->findChildren<QTimer*>()) timer->stop();
        scene->setChosenNum(plantType); Card::setGameState(GameState::PrePlace);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        auto *plant=scene->plantAhead(2,1000); QVERIFY(plant);
        auto *enemy=new YellowDogs(2,scene,1,wave); enemy->setParent(scene); scene->addItem(enemy);
        for(auto *timer : enemy->findChildren<QTimer*>()) timer->stop();
        const int initialHealth=plant->getHp();
        enemy->startAttacking(plant);
        const int biteDamage=initialHealth-plant->getHp();
        QCOMPARE(biteDamage,qRound(GameCatalog::enemies()[1].attack*(1+.025*(wave-1))));
        for(auto *animation : enemy->findChildren<QPropertyAnimation*>()) animation->stop();
        enemy->setPos(1000,430);
        const int enemyHealth=enemy->getHp(); const QPointF enemyPosition=enemy->pos();
        enemy->shootNote();
        auto shots=scene->findChildren<EnemyProjectile*>(); QCOMPARE(shots.size(),1);
        auto *shot=shots.front(); QCOMPARE(shot->damage(),biteDamage);
        for(auto *animation : shot->findChildren<QPropertyAnimation*>()) animation->stop();
        auto *collision=shot->findChild<QTimer*>(); QVERIFY(collision); collision->stop();
        shot->setPos(plant->sceneBoundingRect().center()-shot->boundingRect().center());
        const int beforeHit=plant->getHp();
        QVERIFY(QMetaObject::invokeMethod(collision,"timeout",Qt::DirectConnection));
        QCOMPARE(plant->getHp(),beforeHit-biteDamage); QVERIFY(!shot->scene());
        // Equal damage does not give ranged notes the melee-only defensive effects.
        QCOMPARE(enemy->getHp(),enemyHealth); QCOMPARE(enemy->pos(),enemyPosition);
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
        QCOMPARE(shots.front()->damage(),GameCatalog::enemies()[1].attack);
        play.gamePaused();
        const auto pos = shots.front()->pos();
        QTest::qWait(150); QCOMPARE(shots.front()->pos(),pos);
        play.gameContinued();
        const int health = plant->getHp();
        QTRY_COMPARE_WITH_TIMEOUT(plant->getHp(),health-GameCatalog::enemies()[1].attack,2000);
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
            if(max==1) { QCOMPARE(types.count(0),8); QCOMPARE(types.count(1),4); }
            if(max==2 && level==7) { QCOMPARE(types.count(0),8); QCOMPARE(types.count(1),3); QCOMPARE(types.count(2),1); }
            else if(max==2 && level==8) { QCOMPARE(types.count(0),7); QCOMPARE(types.count(1),4); QCOMPARE(types.count(2),1); }
            else if(max==2 && level==9) { QCOMPARE(types.count(0),5); QCOMPARE(types.count(1),6); QCOMPARE(types.count(2),1); }
            else if(max==2 && level==10) { QCOMPARE(types.count(0),8); QCOMPARE(types.count(1),3); QCOMPARE(types.count(2),1); }
            else if(max==2) { QCOMPARE(types.count(0),9); QCOMPARE(types.count(1),2); QCOMPARE(types.count(2),1); }
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
        verifyGameTerms(play);
        QCOMPARE(scene->getRestHeart(),100); QVERIFY(card->isEnabled());
        const auto folder=qEnvironmentVariable("PVZ_CAPTURE_DIR");
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+"/tutorial-plant.png"));
        QTest::mouseClick(card,Qt::LeftButton);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,200)));
        QVERIFY(scene->findChildren<WhiteDogs*>().isEmpty()); QCOMPARE(scene->getRestHeart(),100);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(QPointF(440,490)));
        QCOMPARE(tutorial->step(),LevelTutorial::Step::Heart); QCOMPARE(scene->getRestHeart(),0);
        verifyGameTerms(play);
        QVERIFY(card->isCooling()); QVERIFY(!card->findChild<QTimer*>()->isActive());
        auto *heart=scene->findChild<Heart*>("tutorialHeart"); QVERIFY(heart);
        QVERIFY(!heart->findChild<QTimer*>()->isActive());
        if(!folder.isEmpty()) QVERIFY(play->grab().save(folder+"/tutorial-heart.png"));
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,view->mapFromScene(heart->sceneBoundingRect().center()));
        QTRY_COMPARE_WITH_TIMEOUT(tutorial->step(),LevelTutorial::Step::Controls,1200);
        QCOMPARE(scene->getRestHeart(),25); QVERIFY(!scene->gameplayStarted());
        verifyGameTerms(play);
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
