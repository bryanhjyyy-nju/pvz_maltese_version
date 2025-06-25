#ifndef WHITEDOGS_H
#define WHITEDOGS_H

#include <QObject>
#include "myitem.h"
// #include "mygamescene.h"

class WhiteDogs : public MyItem
{
    Q_OBJECT
public:
    explicit WhiteDogs(const QString& gifPath, qreal scale = 1.0);

    int HeartCost(){ return heartCost; }

    void cutHp(int atk){ hp -= atk; }

    void setTimer(QTimer *timer){ whiteDogTimer = timer; }

    virtual bool getAtkType(){ return false; }

    //移除自身
    void removeItself(){ emit pleaseRemoveMe(itRow,itCol); }


protected:
    int heartCost = 0;
    QTimer *whiteDogTimer = nullptr;
    // qreal myScale = 1.0;


signals:
    void pleaseRemoveMe(int r,int c);
};

#endif // WHITEDOGS_H
