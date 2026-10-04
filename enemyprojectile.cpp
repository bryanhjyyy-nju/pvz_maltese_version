#include "enemyprojectile.h"
#include "mygamescene.h"
#include "combateffect.h"
#include "whitedogs.h"
#include "audiomanager.h"
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QTimer>
EnemyProjectile::EnemyProjectile(MyGameScene *scene, int row, const QPointF& origin, int damage)
    : m_scene(scene), m_row(row), m_damage(damage) {
    setParent(scene);
    setPos(origin-QPointF(24,28));
    setZValue(8);
    setAcceptedMouseButtons(Qt::NoButton);
    scene->addItem(this);
    auto *movement = new GamePropertyAnimation(scene->gameSpeed(),this,"pos",this);
    movement->setDuration(qMax(1,qRound((x()-300)/220.0*1000)));
    movement->setStartValue(pos()); movement->setEndValue(QPointF(300,y()));
    connect(movement,&QPropertyAnimation::finished,this,&EnemyProjectile::disappear);
    auto *collisionTimer = new GameTimer(this,scene->gameSpeed());
    connect(collisionTimer,&QTimer::timeout,this,&EnemyProjectile::checkCollision);
    collisionTimer->start(30);
    movement->start();
}
void EnemyProjectile::paint(QPainter *p,const QStyleOptionGraphicsItem *,QWidget *) {
    p->setRenderHint(QPainter::Antialiasing);
    // A beamed pair of eighth notes, distinct from the plants' single notes.
    QPainterPath note;
    note.addEllipse(QRectF(3,36,16,12));
    note.addEllipse(QRectF(27,30,16,12));
    note.addRoundedRect(QRectF(14,10,5,33),2,2);
    note.addRoundedRect(QRectF(38,4,5,33),2,2);
    QPainterPath beam;
    beam.moveTo(14,10); beam.lineTo(43,3); beam.lineTo(43,13); beam.lineTo(14,20); beam.closeSubpath();
    note=note.united(beam);
    QLinearGradient gold(0,0,0,50);
    gold.setColorAt(0,QColor("#fff093")); gold.setColorAt(.5,QColor("#ffc83b")); gold.setColorAt(1,QColor("#e8a318"));
    p->setPen(QPen(QColor("#8c5b16"),2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p->setBrush(gold); p->drawPath(note);
    p->setPen(QPen(QColor("#fff7bf"),2));
    p->drawLine(6,16,6,24); p->drawLine(2,20,10,20);
}
void EnemyProjectile::checkCollision() {
    if(m_removed) return;
    const auto& plants=m_scene->plantsInRow(m_row);
    for(auto it=plants.crbegin();it!=plants.crend();++it) {
        auto *plant=*it;
        if(plant->getHp()<=0 || !sceneBoundingRect().intersects(plant->sceneBoundingRect()) || !collidesWithItem(plant)) continue;
        AudioManager::instance().play("guitarHit");
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
