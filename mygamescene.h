#ifndef MYGAMESCENE_H
#define MYGAMESCENE_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "map.h"

class MyGameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit MyGameScene(QMainWindow *parent = nullptr);

    ~MyGameScene();

    //地图网格占用情况初始化
    bool initMapOccupied(int cols, int rows);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;

private:
    Map *mapGrid = NULL;  //添加地图网咯
    bool *mapOccupied = NULL; //添加占用状态表

signals:
};

#endif // MYGAMESCENE_H
