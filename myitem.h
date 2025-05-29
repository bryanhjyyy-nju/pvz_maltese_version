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

    //重写纯虚函数
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    //重写绘图用于调试
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;

    int itRow; //所在行
    int itCol; //所在列
    int getHp() const { return hp; } //返回血量的函数
    qreal getMyScale() const{ return myScale; } //返回缩放比例
protected:
    QMovie *movie = NULL; //动画效果
    int hp = 100; // 血量
    qreal myScale = 1.0;
    void setupGifAnimation(const QString& gifPath); //加载动画

signals:
};

#endif // MYITEM_H
