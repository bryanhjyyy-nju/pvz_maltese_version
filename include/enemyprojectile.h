#pragma once
#include <QGraphicsObject>
class MyGameScene;
class EnemyProjectile : public QGraphicsObject {
    Q_OBJECT
    friend class BattleSnapshot;
public:
    EnemyProjectile(MyGameScene *scene, int row, const QPointF& origin, int damage);
    QRectF boundingRect() const override { return QRectF(0,0,48,56); }
    void paint(QPainter *,const QStyleOptionGraphicsItem *,QWidget *) override;
    int damage() const { return m_damage; }
private:
    MyGameScene *m_scene;
    int m_row, m_damage;
    bool m_removed = false;
    void checkCollision();
    void disappear();
};
