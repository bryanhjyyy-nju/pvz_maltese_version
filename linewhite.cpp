#include "gamecatalog.h"
#include "linewhite.h"

LineWhite::LineWhite(int row, int col, MyGameScene *myScene, QPointF cPos):WhiteDogs(":/white/Image/lineWhite.gif",0.45)
{
    const auto& stats = GameCatalog::plants().at(3);
    hp = stats.health;
    heartCost = stats.cost;
    speed = 100;
    setItPos(row, col);
    WhiteDogs *tempPtr = this;
    setPos(cPos - QPointF(tempPtr->pixmap().width() / 2.0, tempPtr->pixmap().height() / 2.0));
    movingAnim = new QPropertyAnimation(this, "pos", this);
    movingAnim->setObjectName("chargeMovement");
    movingAnim->setDuration(1000);
    // movingAnim->setLoopCount(-1);
    qreal distance = speed * 1.0;
    movingAnim->setEasingCurve(QEasingCurve::Linear);
    movingAnim->setEndValue(pos() + QPointF(distance, 0));
    connect(movingAnim,&QPropertyAnimation::valueChanged,this,[this,plantingX=x()](const QVariant& value) {
        if(!plantingCellVacated && value.toPointF().x()>plantingX) {
            plantingCellVacated=true;
            emit vacatedPlantingCell();
        }
    });

    connect(movingAnim, &QPropertyAnimation::finished, this,[=](){
        if(memIsMoving){
            movingAnim->setStartValue(pos());
            movingAnim->setEndValue(pos() + QPointF(distance, 0));
            movingAnim->start();
        }
    });

    connect(myScene->memGameTimer,&QTimer::timeout, this,[=](){
        if (m_isGamePaused){
            this->stopMoving();
            return;
        }
        if (x() > 1700) {
            this->removeItself();
            return;
        }
        if(checkCollision()){
            //攻击逻辑
            targetYellowDog->getAttacked(2000);
        }
    });
}

bool LineWhite::checkCollision(){
    // 获取场景中所有与当前 LineWhite 碰撞的 item
    QList<QGraphicsItem*> colliding_items = collidingItems();

    // 遍历所有碰撞的 item
    for (QGraphicsItem *item : colliding_items) {
        // 尝试将 item 转换为 YellowDogs 类型
        YellowDogs *yellowDog = dynamic_cast<YellowDogs*>(item);

        // 如果转换成功，并且它和 LineDog 在同一行，说明发生了有效碰撞
        if (yellowDog && !yellowDog->isDying() && yellowDog->getItRow() == this->getItRow()) {
            // 在这里你可以保存目标，以便后续攻击
            targetYellowDog = yellowDog;
            return true; // 发现碰撞，返回 true
        }
    }
    targetYellowDog = nullptr;
    return false; // 没有找到碰撞的 WhiteDogs
}

void LineWhite::gamePaused(){
    if(movie){
        movie->setPaused(true);
    }
    m_isGamePaused = true;
    stopMoving();
}

void LineWhite::gameContinued(){
    if(movie){
        movie->setPaused(false);
    }
    m_isGamePaused = false;
    startMoving();
}
