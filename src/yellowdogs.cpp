#include "yellowdogs.h"
#include "audiomanager.h"
#include "gamecatalog.h"
#include "combateffect.h"
#include "enemyprojectile.h"
#include <QRandomGenerator>
#include <QtMath>

YellowDogs::YellowDogs(int row, MyGameScene *scene, int type,int difficultyWave)
    : targetWhiteDog(nullptr), battleScene(scene), enemyType(qBound(0,type,2)) {
    itRow = row;
    setZValue(5);
    initArgues(enemyType);
    const int extra=qBound(1,difficultyWave,GameCatalog::EndlessDifficultyCapWave)-1;
    hp=qRound(hp*(1+.08*extra));
    atkPower=qRound(atkPower*(1+.025*extra));
    speed*=1+.01*extra;
    initialHealth=hp;
    initialSpeed=speed;
    setPos(QRandomGenerator::global()->bounded(180)+1589-pixmap().width()/2,
           130+145*(row+.5)-pixmap().height()/2);
    tempBackAnim = new GamePropertyAnimation(battleScene->gameSpeed(),this,"pos",this);
    tempBackAnim->setObjectName("enemyKnockback");
    tempBackAnim->setDuration(200);
    connect(tempBackAnim,&QPropertyAnimation::finished,this,[this] { if(!removed) startMoving(); });
    movingAnim = new GamePropertyAnimation(battleScene->gameSpeed(),this,"pos",this);
    movingAnim->setObjectName("enemyMovement");
    movingAnim->setDuration(1000);
    connect(movingAnim,&QPropertyAnimation::finished,this,[this] {
        if(!memIsMoving || removed) return;
        movingAnim->setStartValue(pos());
        movingAnim->setEndValue(pos()-QPointF(speed,0));
        movingAnim->start();
    });
    connect(this,&MyItem::healthChanged,this,&YellowDogs::updateDashSpeed);
    auto animation = [this](const char *property, int duration) {
        auto *result = new GamePropertyAnimation(battleScene->gameSpeed(),this,property,this);
        result->setDuration(duration);
        return result;
    };
    hitAnimation = animation("hitFlash",220);
    biteAnimation = animation("biteProgress",300);
    biteAnimation->setObjectName("enemyBiteAnimation");
    deathAnimation = animation("deathProgress",600);
    connect(deathAnimation,&QPropertyAnimation::finished,this,[this] { emit pleaseRemoveMe(this); });
    // Dash dogs need their own cadence once their low-health skill activates.
    GameTimer *combatTimer=scene->memLongGameTimer;
    if(enemyType==2) {
        dashCombatTimer=new GameTimer(this,scene->gameSpeed());
        dashCombatTimer->setObjectName("dashCombatTimer");
        dashCombatTimer->start(GameCatalog::EnemyBiteIntervalMs);
        combatTimer=dashCombatTimer;
    }
    connect(combatTimer,&QTimer::timeout,this,[this] {
        if(removed || m_isGamePaused || !this->scene()) return;
        if(x()<100) { stopMoving(); emit arrivedYourHome(); return; }
        if(checkCollision()) { stopMoving(); startAttacking(targetWhiteDog); }
        else if(tempBackAnim->state()!=QAbstractAnimation::Running) startMoving();
    });
    if(enemyType == 1) {
        auto *rangedTimer = new GameTimer(this,battleScene->gameSpeed());
        rangedTimer->setObjectName("guitarRangedTimer");
        connect(rangedTimer,&QTimer::timeout,this,&YellowDogs::shootNote);
        rangedTimer->start(GameCatalog::GuitarShotIntervalMs);
    }
}
void YellowDogs::initArgues(int type) {
    const auto& stats = GameCatalog::enemies().at(qBound(0,type,2));
    hp = stats.health;
    speed = stats.minSpeed+QRandomGenerator::global()->bounded(stats.speedRange);
    atkPower = stats.attack;
    setupGifAnimation(stats.image,stats.scale);
    setGameSpeed(battleScene->gameSpeed());
}
void YellowDogs::updateDashSpeed(int health) {
    if(enemyType!=2 || removed || dashAccelerated || health<=0 || health*2>=initialHealth) return;
    dashAccelerated=true;
    speed=initialSpeed*GameCatalog::DashLowHealthMoveMultiplier;
    dashCombatTimer->setIntervalPreservingProgress(qRound(GameCatalog::EnemyBiteIntervalMs/GameCatalog::DashLowHealthBiteMultiplier));
    biteAnimation->setDurationPreservingProgress(qRound(300/GameCatalog::DashLowHealthBiteMultiplier));
    // A bite or knockback keeps its current state; the next walk uses the new speed.
    if(!memIsMoving || movingAnim->state()==QAbstractAnimation::Stopped) return;
    const bool paused=movingAnim->state()==QAbstractAnimation::Paused;
    const QPointF current=pos();
    movingAnim->stop();
    movingAnim->setStartValue(current);
    movingAnim->setEndValue(current-QPointF(speed,0));
    movingAnim->start();
    if(paused) movingAnim->pause();
}
QRectF YellowDogs::boundingRect() const {
    const QRectF sprite=MyItem::boundingRect();
    if(!removed) return sprite.adjusted(-5,0,5,0); // Bite motion shifts by five pixels.
    const qreal diagonal=qSqrt(sprite.width()*sprite.width()+sprite.height()*sprite.height());
    return QRectF(sprite.center()-QPointF(diagonal/2,diagonal/2),QSizeF(diagonal,diagonal)).adjusted(-6,-6,6,6);
}
bool YellowDogs::checkCollision() {
    targetWhiteDog = nullptr;
    const auto& plants=battleScene->plantsInRow(itRow);
    for(auto it=plants.crbegin();it!=plants.crend();++it) {
        auto *plant=*it;
        if(plant->getHp()>0 && sceneBoundingRect().intersects(plant->sceneBoundingRect()) && collidesWithItem(plant)) {
            targetWhiteDog=plant;
            return true;
        }
    }
    return false;
}
void YellowDogs::startAttacking(WhiteDogs *plant) {
    if(removed || !plant || plant->getHp()<=0) return;
    AudioManager::instance().play("bite");
    biteAnimation->stop();
    biteAnimation->setStartValue(0.0); biteAnimation->setEndValue(1.0); biteAnimation->start();
    new CombatEffect(battleScene,plant->sceneBoundingRect().center(),CombatEffect::Bite);
    plant->cutHp(atkPower);
    if(plant->getAtkType()==1) getAttacked(atkPower/2);
    else if(plant->getAtkType()==2 && !removed) {
        tempBackAnim->stop();
        tempBackAnim->setStartValue(pos());
        tempBackAnim->setEndValue(pos()+QPointF(100,0));
        tempBackAnim->start();
    }
    if(plant->getHp()<=0) { plant->removeItself(); targetWhiteDog=nullptr; }
}
void YellowDogs::shootNote() {
    if(enemyType!=1 || removed || m_isGamePaused || !scene() || checkCollision()) return;
    // Shoot forward on every ranged tick, even before plants enter melee range.
    new EnemyProjectile(battleScene,itRow,QPointF(x()+10,130+145*(itRow+.5)),atkPower);
}
void YellowDogs::getAttacked(int attack) {
    if(removed || attack<=0) return;
    AudioManager::instance().play("hit");
    applyDamage(attack);
    hitAnimation->stop();
    hitAnimation->setStartValue(1.0); hitAnimation->setEndValue(0.0); hitAnimation->start();
    new CombatEffect(battleScene,sceneBoundingRect().center(),CombatEffect::Hit);
    emit isAttacked();
    if(hp<=0) removeItself();
}
void YellowDogs::removeItself() {
    if(removed) return;
    prepareGeometryChange(); // Regional redraw must include the rotated death sprite.
    removed=true;
    setHealthVisible(false);
    stopMoving(); tempBackAnim->stop(); hitAnimation->stop(); biteAnimation->stop();
    for(auto *timer : findChildren<QTimer*>()) timer->stop();
    if(movie) movie->setPaused(true);
    targetWhiteDog=nullptr;
    emit dying(this); // Leave the combat list immediately; keep the sprite until its animation finishes.
    deathAnimation->setStartValue(0.0); deathAnimation->setEndValue(1.0);
    deathAnimation->start();
}
void YellowDogs::paint(QPainter *p,const QStyleOptionGraphicsItem *option,QWidget *widget) {
    p->save();
    const auto bounds = MyItem::boundingRect();
    const auto center = bounds.center();
    if(removed) {
        p->setOpacity(1-m_deathProgress);
        p->translate(center);
        p->rotate(-65*m_deathProgress);
        p->scale(1-.35*m_deathProgress,1-.55*m_deathProgress);
        p->translate(-center);
    } else if(m_biteProgress<1) {
        p->translate(5*qSin(m_biteProgress*M_PI*4),0);
    }
    QGraphicsPixmapItem::paint(p,option,widget);
    if(m_hitFlash>0 && !removed) {
        if(flashSourceKey!=pixmap().cacheKey()) {
            flashSourceKey=pixmap().cacheKey(); flashFrame=pixmap();
            QPainter mask(&flashFrame);
            mask.setCompositionMode(QPainter::CompositionMode_SourceIn);
            mask.fillRect(flashFrame.rect(),QColor(255,245,180));
        }
        p->setOpacity(m_hitFlash*.75);
        p->drawPixmap(0,0,flashFrame);
    }
    p->restore();
    p->setRenderHint(QPainter::Antialiasing);
    if(removed) {
        p->setOpacity(1-m_deathProgress);
        p->setPen(QPen(QColor("#c18330"),2)); p->setBrush(QColor("#ffe077"));
        for(int i=0;i<3;++i) {
            qreal angle=m_deathProgress*M_PI*4+i*M_PI*2/3;
            QPointF point(center.x()+qCos(angle)*24,bounds.height()*.22+qSin(angle)*8);
            p->drawEllipse(point,4,4);
        }
    } else if(m_biteProgress<1) {
        p->setOpacity(1-m_biteProgress);
        p->setPen(QPen(QColor("#7a4430"),2)); p->setBrush(QColor("#fff0c7"));
        QPointF jaw(20,bounds.height()*.65);
        p->drawEllipse(jaw,13,8+5*qSin(m_biteProgress*M_PI));
        p->drawLine(jaw-QPointF(7,0),jaw+QPointF(7,0));
    }
}
void YellowDogs::gamePaused() { m_isGamePaused=true; if(movie) movie->setPaused(true); }
void YellowDogs::gameContinued() { m_isGamePaused=false; if(movie && !removed) movie->setPaused(false); }
void YellowDogs::startMoving() {
    if(removed || memIsMoving || tempBackAnim->state()==QAbstractAnimation::Running) return;
    memIsMoving=true;
    movingAnim->setStartValue(pos()); movingAnim->setEndValue(pos()-QPointF(speed,0)); movingAnim->start();
}
void YellowDogs::stopMoving() { memIsMoving=false; if(movingAnim) movingAnim->stop(); }
