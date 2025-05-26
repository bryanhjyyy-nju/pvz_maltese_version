#ifndef WHITEDOGS_H
#define WHITEDOGS_H

#include <QObject>

class WhiteDogs : public QObject
{
    Q_OBJECT
public:
    explicit WhiteDogs(QObject *parent = nullptr);

signals:
};

#endif // WHITEDOGS_H
