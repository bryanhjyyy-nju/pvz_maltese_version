#ifndef YELLOWDOGS_H
#define YELLOWDOGS_H

#include <QObject>
#include "myitem.h"
#include "whitedogs.h"
#include "mygamescene.h"

class YellowDogs : public MyItem
{
    Q_OBJECT
    Q_PROPERTY(qreal hitFlash READ hitFlash WRITE setHitFlash)
    Q_PROPERTY(qreal biteProgress READ biteProgress WRITE setBiteProgress)
    Q_PROPERTY(qreal deathProgress READ deathProgress WRITE setDeathProgress)
public:
    explicit YellowDogs(int row, MyGameScene *myScene,int typeNum);
    bool checkCollision();
    void startAttacking(WhiteDogs *tar);
    void getAttacked(int atk);
    void cutHp(int atk){ applyDamage(atk); } //减少血量
    void removeItself();
    bool isDying() const { return removed; }
    int typeIndex() const { return enemyType; }
    qreal hitFlash() const { return m_hitFlash; }
    qreal biteProgress() const { return m_biteProgress; }
    qreal deathProgress() const { return m_deathProgress; }
    void setHitFlash(qreal v) { m_hitFlash=v; update(); }
    void setBiteProgress(qreal v) { m_biteProgress=v; update(); }
    void setDeathProgress(qreal v) { m_deathProgress=v; update(); }
    void paint(QPainter *,const QStyleOptionGraphicsItem *,QWidget *) override;
    void shootNote();

    void initArgues(int typeNum); //初始化血量和速度和图像

    void gamePaused() override;
    void gameContinued() override;

    void startMoving() override;
    virtual void stopMoving() override;

protected:
    bool removed = false;
    WhiteDogs *targetWhiteDog;
    int atkPower = 10;
    bool m_isGamePaused = false;
    QPropertyAnimation *tempBackAnim;
    MyGameScene *battleScene;
    int enemyType;
    qreal m_hitFlash=0, m_biteProgress=1, m_deathProgress=0;
    QPropertyAnimation *hitAnimation, *biteAnimation, *deathAnimation;

signals:
    void isAttacked();
    void attacking(WhiteDogs *tar);
    void pleaseRemoveMe(YellowDogs *zb);
    void dying(YellowDogs *zb);
    void arrivedYourHome();
};

// class GuitarDog : public YellowDogs
// {
//     Q_OBJECT

// public:
//     explicit GuitarDog(int row, MyGameScene *myScene);
//     void initArgues() override;
// };

#endif // YELLOWDOGS_H
