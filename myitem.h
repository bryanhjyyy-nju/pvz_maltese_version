#ifndef MYITEM_H
#define MYITEM_H

#include <QObject>
#include <QMainWindow>
#include <QGraphicsPixmapItem>
#include <QGraphicsObject>
#include <QMovie>
#include <QPainter>

class MyItem : public QObject,public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    explicit MyItem();
    ~MyItem();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    int row; //所在行
    int col; //所在列
    int Hp() const { return hp; } //返回血量的函数

protected:
    QMovie *movie = NULL; //动画效果
    int hp = 100; // 血量

    void setupGifAnimation(const QString& gifPath); //加载动画

signals:
};

#endif // MYITEM_H
