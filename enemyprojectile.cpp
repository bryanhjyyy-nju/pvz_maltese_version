#include "enemyprojectile.h"
#include "mygamescene.h"
#include "combateffect.h"
#include "whitedogs.h"
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QTimer>
EnemyProjectile::EnemyProjectile(MyGameScene *scene, int row, const QPointF& origin, int damage)
    : m_scene(scene), m_row(row), m_damage(damage) {
    setParent(scene);
    setPos(origin-QPointF(17,25));
    setZValue(8);
    setAcceptedMouseButtons(Qt::NoButton);
    scene->addItem(this);
    auto *movement = new QPropertyAnimation(this,"pos",this);
    movement->setDuration(qMax(1,qRound((x()-300)/220.0*1000)));
    movement->setStartValue(pos()); movement->setEndValue(QPointF(300,y()));
    connect(movement,&QPropertyAnimation::finished,this,&EnemyProjectile::disappear);
    auto *collisionTimer = new QTimer(this);
    connect(collisionTimer,&QTimer::timeout,this,&EnemyProjectile::checkCollision);
    collisionTimer->start(30);
    movement->start();
}
void EnemyProjectile::paint(QPainter *p,const QStyleOptionGraphicsItem *,QWidget *) {
    p->setRenderHint(QPainter::Antialiasing);
    // Draw a quarter rest explicitly, so no music-font installation is needed.
    QPainterPath rest;
    rest.moveTo(20,3); rest.lineTo(11,15); rest.lineTo(22,25);
    rest.lineTo(11,34); rest.cubicTo(2,42,18,49,22,43);
    rest.cubicTo(11,47,10,39,17,38);
    p->setPen(QPen(QColor("#5f358a"),6,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p->drawPath(rest);
    p->setPen(QPen(QColor("#dfb9fa"),2,Qt::SolidLine,Qt::RoundCap));
    p->drawPath(rest);
}
void EnemyProjectile::checkCollision() {
    if(m_removed) return;
    for(auto *item : collidingItems()) {
        auto *plant = dynamic_cast<WhiteDogs*>(item);
        if(!plant || plant->getItRow() != m_row || plant->getHp() <= 0) continue;
        plant->cutHp(m_damage);
        new CombatEffect(m_scene,plant->sceneBoundingRect().center(),CombatEffect::Hit);
        if(plant->getHp() <= 0) plant->removeItself();
        disappear();
        return;
    }
}
void EnemyProjectile::disappear() {
    if(m_removed) return;
    m_removed = true;
    for(auto *timer : findChildren<QTimer*>()) timer->stop();
    for(auto *animation : findChildren<QPropertyAnimation*>()) animation->stop();
    if(scene()) scene()->removeItem(this);
    deleteLater();
}
