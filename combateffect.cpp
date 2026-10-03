#include "combateffect.h"
#include "mygamescene.h"
#include <QPainter>
#include <QPropertyAnimation>
#include <QtMath>
CombatEffect::CombatEffect(MyGameScene *scene, const QPointF& center, Kind kind) : m_kind(kind) {
    setParent(scene);
    setPos(center);
    setZValue(15);
    setAcceptedMouseButtons(Qt::NoButton);
    scene->addItem(this);
    auto *animation = new QPropertyAnimation(this,"progress",this);
    animation->setDuration(kind == Bite ? 300 : 450);
    animation->setStartValue(0.0); animation->setEndValue(1.0);
    connect(animation,&QPropertyAnimation::finished,this,[this] {
        if(this->scene()) this->scene()->removeItem(this);
        deleteLater();
    });
    animation->start();
}
void CombatEffect::paint(QPainter *p,const QStyleOptionGraphicsItem *,QWidget *) {
    p->setRenderHint(QPainter::Antialiasing);
    p->setOpacity(1-m_progress);
    QColor color = m_kind == Hit ? QColor("#ffbe54") : m_kind == Plant ? QColor("#8bcc65") : QColor("#b88140");
    p->setPen(QPen(color.darker(160),2)); p->setBrush(color);
    for(int i=0;i<7;++i) {
        const qreal angle = i*2*M_PI/7;
        const qreal radius = 15 + m_progress*40;
        QPointF point(qCos(angle)*radius,qSin(angle)*radius);
        if(m_kind == Bite || m_kind == Uproot) p->drawRoundedRect(QRectF(point-QPointF(4,3),QSizeF(8,6)),2,2);
        else {
            p->drawLine(point-QPointF(5,0),point+QPointF(5,0));
            p->drawLine(point-QPointF(0,5),point+QPointF(0,5));
        }
    }
}
