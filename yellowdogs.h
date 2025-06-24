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
    explicit YellowDogs(int row, MyGameScene *myScene,int typeNum);
    bool checkCollision();
    void startAttacking(WhiteDogs *tar);
    void getAttacked(int atk);
    void stopMoving(); //重定义
    void cutHp(int atk){ hp -= atk; } //减少血量
    void removeItself(){ emit pleaseRemoveMe(this); }; //移除自己

    void initArgues(int typeNum); //初始化血量和速度和图像

protected:
    WhiteDogs *targetWhiteDog;
    bool memIsMoving;
    int atkPower = 10;

signals:
    void isAttacked();
    void attacking(WhiteDogs *tar);
    void pleaseRemoveMe(YellowDogs *zb);
    void arrivedYourHome();
};

// class GuitarDog : public YellowDogs
// {
//     Q_OBJECT

// public:
//     explicit GuitarDog(int row, MyGameScene *myScene);
//     void initArgues() override;
// };

#endif // YELLOWDOGS_H
