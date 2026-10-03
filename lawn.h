#pragma once
#include <QGraphicsObject>
#include <QPixmap>

// Dirt covers the original lawn artwork until each new lane is unrolled.
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
private:
    int level;
    qreal progress=1;
    QPixmap soil;
};
