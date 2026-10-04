#ifndef DANCINGWHITE_H
#define DANCINGWHITE_H

#include <QObject>
#include "whitedogs.h"

class DancingWhite : public WhiteDogs
{
    Q_OBJECT
public:
    explicit DancingWhite();

    int getAtkType() override{ return 1; }

signals:
};

#endif // DANCINGWHITE_H
