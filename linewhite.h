#ifndef LINEWHITE_H
#define LINEWHITE_H

#include <QObject>
#include "whitedogs.h"
#include <QPropertyAnimation>


class LineWhite : public WhiteDogs
{
    Q_OBJECT
public:
    explicit LineWhite();

private:
    QPropertyAnimation *memRunningAnim;

signals:
};

#endif // LINEWHITE_H
