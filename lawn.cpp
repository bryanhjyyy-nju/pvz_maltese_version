#include "lawn.h"
#include <QPainter>
#include <QRandomGenerator>

Lawn::Lawn(int number) : level(number),soil(1089,725) {
    setObjectName("lawn");
    setZValue(1);
    setAcceptedMouseButtons(Qt::NoButton);
    soil.fill(QColor("#b7a17c"));
    QPainter painter(&soil);
    QRandomGenerator random(42);
    painter.setPen(Qt::NoPen);
    for(int i=0;i<1600;++i) {
        painter.setBrush(i%2 ? QColor("#c7b38e") : QColor("#a58f6f"));
        painter.drawEllipse(QRectF(random.bounded(1089),random.bounded(725),random.bounded(2,7),random.bounded(2,4)));
    }
}
void Lawn::setRevealProgress(qreal value) { progress=qBound(0.0,value,1.0); update(); }
qreal Lawn::rowReveal(int row) const {
    if(row<0 || row>4) return 0;
    if(level>=4) return 1;
    if(level==1) return row==2 ? progress : 0;
    if(level==2) return row==2 ? 1 : (row==1 || row==3) ? progress : 0;
    return row>=1 && row<=3 ? 1 : progress;
}
void Lawn::paint(QPainter *painter,const QStyleOptionGraphicsItem*,QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);
    for(int row=0;row<5;++row) {
        const qreal reveal=rowReveal(row);
        const QRectF lane(380,130+145*row,1089,145);
        if(reveal>=1) continue;
        const qreal edge=lane.left()+lane.width()*reveal;
        const QRectF covered(edge,lane.top(),lane.right()-edge,lane.height());
        painter->drawPixmap(covered,soil,QRectF(edge-380,row*145,covered.width(),145));
        painter->setPen(QPen(QColor("#97805f"),2));
        painter->drawLine(covered.topLeft(),covered.topRight());
        if(reveal>0) {
            // The bright cylindrical edge travels from left to right.
            QLinearGradient roll(edge-18,0,edge+14,0);
            roll.setColorAt(0,QColor("#286e25"));
            roll.setColorAt(.5,QColor("#9bd84f"));
            roll.setColorAt(1,QColor("#356e26"));
            painter->setBrush(roll); painter->setPen(QPen(QColor("#254a1f"),3));
            painter->drawRoundedRect(QRectF(edge-18,lane.top()+3,32,139),12,12);
            painter->setPen(QPen(QColor("#c5ed79"),2));
            for(int y=12;y<140;y+=15) painter->drawLine(QPointF(edge-5,lane.top()+y),QPointF(edge+3,lane.top()+y-5));
        }
    }
}
