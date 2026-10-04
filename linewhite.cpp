#include "gamecatalog.h"
#include "linewhite.h"

LineWhite::LineWhite(int row, int col, MyGameScene *myScene, QPointF cPos):WhiteDogs(":/white/Image/lineWhite.gif",0.45),battleScene(myScene)
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

    connect(myScene->getGameTimer(),&QTimer::timeout, this,[=](){
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
    const auto& enemies=battleScene->getZombieMap(getItRow());
    for(auto it=enemies.crbegin();it!=enemies.crend();++it) {
        auto *yellowDog=static_cast<YellowDogs*>(*it);
        if(!yellowDog->isDying() && sceneBoundingRect().intersects(yellowDog->sceneBoundingRect()) && collidesWithItem(yellowDog)) {
            targetYellowDog = yellowDog;
            return true;
        }
    }
    targetYellowDog = nullptr;
    return false;
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
