#ifndef MYITEM_H
#define MYITEM_H

#include <QObject>
#include <QMainWindow>
#include <QGraphicsItem>
#include <QGraphicsObject>

class MyItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit MyItem();

    QRectF boundingRect() const override;

signals:
};

#endif // MYITEM_H
