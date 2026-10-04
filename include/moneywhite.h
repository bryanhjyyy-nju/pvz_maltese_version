#ifndef MONEYWHITE_H
#define MONEYWHITE_H

#include <QObject>
#include "whitedogs.h"

class MoneyWhite : public WhiteDogs
{
    Q_OBJECT
public:
    explicit MoneyWhite();

    int getAtkType() override{ return 2; }

signals:
};

#endif // MONEYWHITE_H
