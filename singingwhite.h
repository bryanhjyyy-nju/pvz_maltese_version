#ifndef SINGINGWHITE_H
#define SINGINGWHITE_H

#include <QObject>
#include "whitedogs.h"
#include "mygamescene.h"

class SingingWhite : public WhiteDogs
{
    Q_OBJECT
public:
    explicit SingingWhite(int r, int c,MyGameScene *scene);
    void shootBullet(int r, int c);


protected:


signals:
    void bulletShot(int r,int c);
};

#endif // SINGINGWHITE_H
