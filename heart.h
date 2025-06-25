#ifndef HEART_H
#define HEART_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>
#include <QTimer>
#include "mygamescene.h"


class Heart : public QObject,public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)


public:
    explicit Heart(QPointF startPos, QPointF endPos,MyGameScene *gameScene,QEasingCurve::Type type = QEasingCurve::Linear, QObject *parent = nullptr);

    QRectF boundingRect() const override;
    int value() const { return 25; } //返回爱心数值

    //开始下落
    void startFall();
    // void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    static QPointF curMousePos;

    void gamePaused();
    void gameContinued();

signals:
    void collected();
    void i_have_disappeared();

protected:

private slots:
    void hasReachedGround(); //到达地面完成
    void hasDisappear(); //消失动画完成

private:
    QPointF memEndPos; //下落终点位置记录
    QPropertyAnimation *memCollectAnim; //爱心收集动画
    QPropertyAnimation *memFallAnim; //爱心下落动画
    QTimer* memDisappearTimer; //爱心消失计时器
    bool isCollectable = true;
    bool isDisappearing = false;
};

#endif // HEART_H
