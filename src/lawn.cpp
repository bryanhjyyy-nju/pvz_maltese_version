#include "lawn.h"
#include <QPainter>
#include <QtMath>

namespace {
constexpr int Columns=9;
constexpr qreal CellWidth=121,CellHeight=145;

void drawGoldBrick(QPainter& painter,int column) {
    // Broad top, beveled front and a darker right face give the paving depth.
    QPolygonF right;
    right << QPointF(112,7) << QPointF(119,24) << QPointF(119,126)
          << QPointF(109,139) << QPointF(115,115);
    painter.setPen(QPen(QColor("#80501b"),2));
    painter.setBrush(QColor("#b47c20")); painter.drawPolygon(right);
    QPolygonF front;
    front << QPointF(7,115) << QPointF(115,115) << QPointF(109,139) << QPointF(13,139);
    QLinearGradient edge(0,115,0,139);
    edge.setColorAt(0,QColor("#e5a830")); edge.setColorAt(1,QColor("#a86619"));
    painter.setBrush(edge); painter.drawPolygon(front);
    QPolygonF top;
    top << QPointF(14,7) << QPointF(112,7) << QPointF(115,115) << QPointF(7,115);
    QLinearGradient gold(10,10,110,115);
    gold.setColorAt(0,QColor("#fff1a2"));
    gold.setColorAt(.32,column%2 ? QColor("#f8d466") : QColor("#ffe07a"));
    gold.setColorAt(.75,QColor("#e9b83e")); gold.setColorAt(1,QColor("#ce9327"));
    painter.setBrush(gold); painter.drawPolygon(top);
    painter.setBrush(Qt::NoBrush); painter.setPen(QPen(QColor("#fff4b4"),3));
    painter.drawLine(QPointF(16,10),QPointF(109,10));
    painter.drawLine(QPointF(16,10),QPointF(10,110));
    painter.setPen(QPen(QColor("#f9d772"),2));
    painter.drawLine(QPointF(17,126),QPointF(106,126));
    // An inset diamond and reflected glints keep the surface readable.
    QPolygonF seal;
    seal << QPointF(61,47) << QPointF(77,64) << QPointF(61,81) << QPointF(45,64);
    painter.setPen(QPen(QColor("#c79029"),2)); painter.setBrush(QColor(255,232,137,125));
    painter.drawPolygon(seal);
    painter.setPen(QPen(QColor("#fff8ce"),2));
    painter.drawLine(QPointF(94,26),QPointF(94,42));
    painter.drawLine(QPointF(87,34),QPointF(101,34));
}
}

Lawn::Lawn(int number) : level(number) {
    setObjectName("lawn"); setZValue(-10); setAcceptedMouseButtons(Qt::NoButton);
}
void Lawn::setRevealProgress(qreal value) { progress=qBound(0.0,value,1.0); update(); }
qreal Lawn::rowReveal(int row) const {
    if(row<0 || row>4) return 0;
    if(level>=4) return 1;
    if(level==1) return row==2 ? progress : 0;
    if(level==2) return row==2 ? 1 : (row==1 || row==3) ? progress : 0;
    return row>=1 && row<=3 ? 1 : progress;
}
qreal Lawn::brickFallProgress(int row,int column) const {
    if(row<0 || row>=5 || column<0 || column>=Columns) return 0;
    return qBound(0.0,rowReveal(row)*Columns-column,1.0);
}
void Lawn::paint(QPainter *painter,const QStyleOptionGraphicsItem*,QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);
    for(int row=0;row<5;++row) for(int column=0;column<Columns;++column) {
        const qreal fall=brickFallProgress(row,column);
        if(fall>=1) continue;
        const QRectF cell(380+CellWidth*column,130+CellHeight*row,CellWidth,CellHeight);
        painter->save(); painter->setClipRect(cell);
        painter->translate(cell.topLeft());
        // Each cell has its own recessed bed, exposed as its brick falls away.
        painter->setOpacity(1-fall);
        painter->fillRect(QRectF(0,0,CellWidth,CellHeight),QColor("#746236"));
        painter->setPen(Qt::NoPen); painter->setBrush(QColor(42,32,15,100));
        painter->drawRoundedRect(QRectF(4,12,115,133),8,8);
        painter->translate(61,72+qPow(fall,2)*180);
        painter->rotate(fall*18); painter->scale(1-.12*fall,1-.12*fall);
        painter->translate(-61,-72); drawGoldBrick(*painter,column);
        painter->restore();
    }
}
