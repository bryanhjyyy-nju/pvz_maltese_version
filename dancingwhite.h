#ifndef DANCINGWHITE_H
#define DANCINGWHITE_H

#include <QObject>
#include "whitedogs.h"

class DancingWhite : public WhiteDogs
{
    Q_OBJECT
public:
    explicit DancingWhite();

    bool getAtkType() override{ return true; }

signals:
};

#endif // DANCINGWHITE_H
