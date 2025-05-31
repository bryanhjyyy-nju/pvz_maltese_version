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
    int getChosenNum(){ return chosenNum; }

    int getRestHeart(){ return restHeart; } // 得到剩余爱心的数值

    void cutHeart(int amont){ restHeart -= amont; } //剩余爱心的数值减去消耗爱心数值
    void addHeart(int amont){ restHeart += amont; } //剩余爱心的数量加上收集到爱心的数量

    void generateSkyHeart(); //天空中随机生成爱心
    void generateWhiteHeart(QPointF whitePos); //从小白中产出爱心

private:
    Map *mapGrid = NULL;  //添加地图网咯
    bool *mapOccupied = NULL; //添加占用状态表
    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    int chosenNum = 0;
    int restHeart = 50; // 剩余阳光初始化为500

    QTimer *memSkyHeartTimer; //天空中的爱心生成计时器

signals:
    void plantFinished();
    void heartCollected();
    void sceneClicked();

public slots:
    // void generatedHeartFromWhite(QPointF dogPos);
};

#endif // MYGAMESCENE_H
