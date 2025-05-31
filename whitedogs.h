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

    //放置的位置

    int HeartCost(){ return heartCost; }

protected:
    int heartCost = 0;
    qreal myScale = 1.0;

signals:
};

#endif // WHITEDOGS_H
