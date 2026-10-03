#pragma once
#include <QGraphicsObject>

// Gold paving covers unavailable cells; new lanes shed their bricks in order.
class Lawn : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(qreal revealProgress READ revealProgress WRITE setRevealProgress)
public:
    explicit Lawn(int level);
    QRectF boundingRect() const override { return QRectF(380,130,1089,725); }
    void paint(QPainter *painter,const QStyleOptionGraphicsItem*,QWidget*) override;
    qreal revealProgress() const { return progress; }
    void setRevealProgress(qreal value);
    qreal rowReveal(int row) const;
    qreal brickFallProgress(int row,int column) const;
private:
    int level;
    qreal progress=1;
};
