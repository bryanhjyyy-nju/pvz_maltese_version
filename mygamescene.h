#ifndef MYGAMESCENE_H
#define MYGAMESCENE_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "map.h"
#include <QGraphicsTextItem>

class MyGameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit MyGameScene(QMainWindow *parent = nullptr);

    ~MyGameScene();

    //地图网格占用情况初始化
    bool initMapOccupied(int cols, int rows);

    //设置选择的卡牌序号
    void setChosenNum(int cardNum);

    int getRestHeart(){ return restHeart; } // 得到剩余爱心的数值

    void cutHeart(int thisHeartCost){ restHeart -= thisHeartCost; } //剩余爱心的数值减去消耗爱心数值

private:
    Map *mapGrid = NULL;  //添加地图网咯
    bool *mapOccupied = NULL; //添加占用状态表
    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    int chosenNum = 0;
    int restHeart = 500; // 剩余阳光初始化为500

signals:
    void plantFinished();

public slots:
    // void generatedHeartFromWhite(QPointF dogPos);
};

#endif // MYGAMESCENE_H
