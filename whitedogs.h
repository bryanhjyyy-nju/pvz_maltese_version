#ifndef WHITEDOGS_H
#define WHITEDOGS_H

#include <QObject>
#include "myitem.h"

class WhiteDogs : public MyItem
{
    Q_OBJECT
public:
    explicit WhiteDogs(const QString& gifPath);

    //放置的位置

    int HeartCost(){ return heartCost; }

protected:
    int heartCost = 0;


signals:
};

#endif // WHITEDOGS_H
