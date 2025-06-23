#ifndef MYITEM_H
#define MYITEM_H

#include <QObject>
#include <QMainWindow>
#include <QGraphicsPixmapItem>
#include <QGraphicsObject>
#include <QMovie>
#include <QPainter>
#include <QPropertyAnimation>
#include "myDirection.h"
#include <QTimer>

class MyItem : public QObject,public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    explicit MyItem();
    ~MyItem();

    //重写纯虚函数
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    //重写绘图用于调试
    // void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;

    void setItPos(int r, int c); //设置所在行，所在列
    int getItRow() const { return itRow; }
    int getHp() const { return hp; } //返回血量的函数
    void startMoving(MyDirection dir);
    virtual void stopMoving() {movingAnim->stop();}
    // qreal getMyScale() const{ return myScale; } //返回缩放比例
protected:
    QMovie *movie = NULL; //动画效果
    int hp = 100; // 血量
    int itRow = 0; //所在行
    int itCol = 0; //所在列
    qreal speed = 0.0; //设置速度
    QPropertyAnimation *movingAnim; //设置移动动画
    void setupGifAnimation(const QString& gifPath, qreal scale = 1.0); //加载动画

signals:
};

#endif // MYITEM_H
