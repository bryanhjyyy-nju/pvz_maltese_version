#ifndef BULLET_H
#define BULLET_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QTimer>

class Bullet : public QObject,public QGraphicsPixmapItem
{
    Q_OBJECT
    // Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    explicit Bullet(QPointF bulletPos, QTimer *gameTimer);

signals:
};

#endif // BULLET_H
