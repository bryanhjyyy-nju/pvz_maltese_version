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
    bool checkCollision(MyGameScene *myScene);
    void startAttacking(WhiteDogs *tar);
    void getAttacked();


private:
    WhiteDogs *targetWhiteDog;

signals:
    void isAttacked();
    void attacking(WhiteDogs *tar);
};

#endif // YELLOWDOGS_H
