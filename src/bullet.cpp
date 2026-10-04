#include "bullet.h"

Bullet::Bullet(int r, int c, QTimer *gameTimer)
{
    itRow = r;
    //设置阳光图片
    QPixmap pix;
    pix.load(":/others/Image/note.png");
    setPixmap(pix);
    setPos(400 + c * 121,130 + 145 * (r + 0.2)); //设置起始位置
    setZValue(7); //确保在小白上面
    connect(this, &Bullet::i_hit_it,this, &Bullet::disappear);
    connect(gameTimer,&QTimer::timeout, this, [=](){
        if(checkCollision()){
            emit i_hit_it(targetZombie);
            targetZombie->getAttacked(atkPower);
        }
        if(x() > 1700){ disappear(); }
    });
    memMovingAnim = new GamePropertyAnimation(GameSpeed::forObject(gameTimer),this, "pos", this);
    memMovingAnim->setDuration(qMax(1,qRound((1701-x())/speed*1000)));
    memMovingAnim->setEasingCurve(QEasingCurve::Linear);
    memMovingAnim->setStartValue(pos());
    memMovingAnim->setEndValue(QPointF(1701,y()));
    connect(memMovingAnim,&QPropertyAnimation::finished,this,&Bullet::disappear);
    memMovingAnim->start();

}


void Bullet::disappear(){
    if(!scene()) return;
    memMovingAnim->stop();
    scene()->removeItem(this);
    // qDebug() << "removeBullet!";
    emit hasDisappeared();
    this->deleteLater();
}


bool Bullet::checkCollision(){
    targetZombie=nullptr;
    auto *battle=qobject_cast<MyGameScene*>(scene());
    if(!battle) return false;
    const auto& enemies=battle->getZombieMap(itRow);
    for(auto it=enemies.crbegin();it!=enemies.crend();++it) {
        auto *yellowDog=static_cast<YellowDogs*>(*it);
        if(!yellowDog->isDying() && sceneBoundingRect().intersects(yellowDog->sceneBoundingRect()) && collidesWithItem(yellowDog)) {
            targetZombie = yellowDog;
            return true;
        }
    }
    return false;
}

void Bullet::gamePaused(){
    if(memMovingAnim){
        if(memMovingAnim->state() == memMovingAnim->Running){
            memMovingAnim->setPaused(true);
        }
    }
}

void Bullet::gameContinued(){
    if(memMovingAnim){
        if(memMovingAnim->state() == memMovingAnim->Paused){
            memMovingAnim->setPaused(false);
        }
    }
}
