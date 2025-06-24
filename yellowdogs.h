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
    void getAttacked(int atk);
    void stopMoving(); //重定义
    void cutHp(int atk){ hp -= atk; } //减少血量
    void removeItself(){ emit pleaseRemoveMe(this); }; //移除自己

private:
    WhiteDogs *targetWhiteDog;
    bool memIsMoving;

signals:
    void isAttacked();
    void attacking(WhiteDogs *tar);
    void pleaseRemoveMe(YellowDogs *zb);
    void arrivedYourHome();
};

#endif // YELLOWDOGS_H
