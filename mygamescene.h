#ifndef MYGAMESCENE_H
#define MYGAMESCENE_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "map.h"
#include <QGraphicsTextItem>
#include "whitedogs.h"
#include <QTimer>
#include <QGraphicsPixmapItem>

class MyGameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit MyGameScene(int n = 1,QMainWindow *parent = nullptr);

    ~MyGameScene();

    friend class YellowDogs;
    friend class LineWhite;

    //地图网格占用情况初始化
    bool initMapOccupied(int cols, int rows);

    //设置选择的卡牌序号
    void setChosenNum(int cardNum);
    int getChosenNum(){ return chosenNum; }

    int getRestHeart(){ return restHeart; } // 得到剩余爱心的数值

    void cutHeart(int amont){ restHeart -= amont; } //剩余爱心的数值减去消耗爱心数值
    void addHeart(int amont){ restHeart += amont; } //剩余爱心的数量加上收集到爱心的数量
    void removeWhite(int r, int c); //移除小白

    void generateSkyHeart(); //天空中随机生成爱心
    void generateWhiteHeart(QPointF whitePos); //从小白中产出爱心
    void setAYellowDog(int r, int typeNum = 0); //在第r行产生一只小金毛
    void generateBullet(int r, int c); //产生子弹
    QTimer * getGameTimer(){ return memGameTimer; } //获取gameTimer
    // int getZombieNum(int r){ return zombieMap[r]; } //获取第 r 行的僵尸数量
    const QVector<MyItem *> &getZombieMap(int r){ return zombieMap[r]; }
private:
    int gameLevelNum = 0;

    Map *mapGrid = NULL;  //添加地图网咯
    bool *mapOccupied = NULL; //添加占用状态表
    WhiteDogs *dogMap[45] = {nullptr};
    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    int chosenNum = 0;
    int restHeart = 50; // 剩余阳光初始化为50
    // int zombieMap[5] = {0};
    QVector<QVector<MyItem *>> zombieMap;
    QVector<QGraphicsPixmapItem *> bullets;
    QVector<QGraphicsPixmapItem *> hearts;


    QTimer *memSkyHeartTimer; //天空中的爱心生成计时器
    QTimer *memYellowDogsTimer; //小金毛计时器
    QTimer *memGameTimer; //游戏总的计时器
    // QTimer *memGameLongTimer;

    void checkWinCondition(); // 检查胜利条件的私有函数
    void stopAllTimers();     // 停止所有计时器的辅助函数

    int m_totalZombiesForLevel = 0; // 本关卡总僵尸数
    int m_zombiesSpawned = 0;       // 已生成的僵尸数
    int m_zombiesKilled = 0;        // 已消灭的僵尸数
    bool m_isGameOver = false;      // 标记游戏是否已结束
signals:
    void plantFinished();
    void heartCollected();
    void sceneClicked();
    void gameWin();
    void gameLose();
    // void gameRestart();
    // void gamePause();


public slots:
    // void generatedHeartFromWhite(QPointF dogPos);
    void winTheGame();   // 游戏胜利的槽函数
    void loseTheGame();  // 游戏失败的槽函数
};

#endif // MYGAMESCENE_H
