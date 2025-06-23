#ifndef YELLOWDOGS_H
#define YELLOWDOGS_H

#include <QObject>
#include "myitem.h"
#include "whitedogs.h"
#include "mygamescene.h"

class YellowDogs : public MyItem
{
    Q_OBJECT
public:
    explicit YellowDogs(int row, MyGameScene *myScene);
    bool checkCollision();
    void startAttacking(WhiteDogs *tar);
    void getAttacked();
    void stopMoving(); //重定义

private:
    WhiteDogs *targetWhiteDog;
    bool memIsMoving;

signals:
    void isAttacked();
    void attacking(WhiteDogs *tar);
};

#endif // YELLOWDOGS_H
