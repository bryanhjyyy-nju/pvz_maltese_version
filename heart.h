#ifndef HEART_H
#define HEART_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>
#include <QTimer>
class Heart : public QObject,public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit Heart(QPointF startPos, QPointF endPos, QObject *parent = nullptr);

    int value() const { return 25; } //返回爱心数值

    //开始下落
    void startFall();

signals:
    void collected();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private slots:
    void hasReachedGround(); //到达地面完成
    void hasDisappear(); //消失动画完成

private:
    QPointF memEndPos; //下落终点位置记录
    QPropertyAnimation *memCollectAnim; //爱心收集动画
    QPropertyAnimation *memFallAnim; //爱心下落动画
    QTimer* memDisappearTimer; //爱心消失计时器
    bool memIsCollectable = false; //是否可以收集

};

#endif // HEART_H
