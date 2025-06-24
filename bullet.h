#ifndef BULLET_H
#define BULLET_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QPropertyAnimation>
#include "yellowdogs.h"

class Bullet : public QObject,public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    explicit Bullet(int r, int c, QTimer *gameTimer);
    int getItRow(){ return itRow; }
    void disappear();
    bool checkCollision();


protected:
    QPropertyAnimation *memMovingAnim;
    YellowDogs *targetZombie = nullptr;
    int itRow = 0;
    int speed = 200;
    int atkPower = 50;

signals:
    void i_hit_it(YellowDogs *zb);
};

#endif // BULLET_H
