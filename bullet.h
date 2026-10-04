#ifndef BULLET_H
#define BULLET_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QPropertyAnimation>
#include "yellowdogs.h"
#include "gamecatalog.h"

class Bullet : public QObject,public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    explicit Bullet(int r, int c, QTimer *gameTimer);
    int getItRow(){ return itRow; }
    void disappear();
    bool checkCollision();

    void gamePaused();
    void gameContinued();


protected:
    GamePropertyAnimation *memMovingAnim;
    YellowDogs *targetZombie = nullptr;
    int itRow = 0;
    int speed = 300;
    int atkPower = GameCatalog::BulletDamage;

signals:
    void i_hit_it(YellowDogs *zb);
    void hasDisappeared();
};

#endif // BULLET_H
