#include "yellowdogs.h"


YellowDogs::YellowDogs(int row): targetWhiteDog(nullptr){
    itRow = row;

    //用刀叉小黄先实例化一只
    speed = 30;
    setupGifAnimation(":/yellow/Image/forkYellow.gif",0.6);

    movingAnim = new QPropertyAnimation(this, "pos", this);
    movingAnim->setDuration(1000);
    // movingAnim->setLoopCount(-1);
    movingAnim->setEasingCurve(QEasingCurve::Linear);
    qreal distance = speed * 1.0;
    connect(movingAnim, &QPropertyAnimation::finished, this,[=](){
        QPointF target = pos() + QPointF(-distance, 0);
        movingAnim->setStartValue(pos());
        movingAnim->setEndValue(target);
        movingAnim->start();
        // if(x() < 0){
        //     scene()
        //     delete this;
        // }
    });
    setZValue(5);
}

bool YellowDogs::checkCollision(){
    return true;
}
