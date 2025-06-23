#include "bullet.h"

Bullet::Bullet(QPointF bulletPos, QTimer *gameTimer)
{
    //设置阳光图片
    QPixmap pix;
    pix.load(":/others/Image/note.png");
    setPixmap(pix);
    setPos(bulletPos); //设置起始位置
    setZValue(7); //确保在最上层
}
