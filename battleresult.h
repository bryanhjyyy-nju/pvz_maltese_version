#pragma once
#include <QWidget>
#include "gamespeed.h"

// A frozen battlefield stays underneath the victory reveal or defeat spotlight.
class BattleResult : public QWidget {
    Q_OBJECT
public:
    BattleResult(bool won,int level,const QPointF& losingEnemy,QWidget *parent,GameSpeed *clock=nullptr);
    void stop();
    bool victory() const { return won; }
    int rewardPlant() const { return won && level<8 ? level : -1; }
signals:
    void returnRequested();
    void nextRequested();
protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private:
    bool won;
    int level;
    QPointF enemy;
    qreal progress=0;
    bool sounded=false;
    GameVariantAnimation timeline;
    class QPushButton *back=nullptr,*next=nullptr;
};
