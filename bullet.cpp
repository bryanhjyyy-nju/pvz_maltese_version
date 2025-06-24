#include "bullet.h"

Bullet::Bullet(int r, int c, QTimer *gameTimer)
{
    itRow = r;
    //设置阳光图片
    QPixmap pix;
    pix.load(":/others/Image/note.png");
    setPixmap(pix);
    setPos(480 + c,130 + 145 * (r + 0.2)); //设置起始位置
    setZValue(7); //确保在最上层
    connect(this, &Bullet::i_hit_it,this, &Bullet::disappear);
    connect(gameTimer,&QTimer::timeout, this, [=](){
        if(checkCollision()){
            emit i_hit_it(targetZombie);
            targetZombie->getAttacked(atkPower);
        }
        if(x() > 1700){ disappear(); }
    });
    memMovingAnim = new QPropertyAnimation(this, "pos", this);
    memMovingAnim->setDuration(100);
    memMovingAnim->setEasingCurve(QEasingCurve::Linear);
    memMovingAnim->setStartValue(pos());
    memMovingAnim->setEndValue(pos() + QPointF(10, 0));
    connect(memMovingAnim,&QPropertyAnimation::finished, this, [=](){
        memMovingAnim->setStartValue(pos());
        memMovingAnim->setEndValue(pos() + QPointF(10, 0));
        memMovingAnim->start();
    });
    memMovingAnim->start();

}


void Bullet::disappear(){
    scene()->removeItem(this);
    this->deleteLater();
}


bool Bullet::checkCollision(){
    QList<QGraphicsItem*> colliding_items = collidingItems();

    // 遍历所有碰撞的 item
    for (QGraphicsItem *item : colliding_items) {
        // 尝试将 item 转换为 YellowDogs 类型
        YellowDogs *yellowDog = dynamic_cast<YellowDogs*>(item);

        // 如果转换成功，并且它和 Bullet 在同一行，说明发生了有效碰撞
        if (yellowDog && yellowDog->getItRow() == this->getItRow()) {
            // 在这里你可以保存目标，以便后续攻击
            targetZombie = yellowDog;
            return true; // 发现碰撞，返回 true
        }
    }
    return false;
}
