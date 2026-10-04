#ifndef MYGAMESCENE_H
#define MYGAMESCENE_H

#include <QObject>
#include <QGraphicsScene>
#include "map.h"
#include <QGraphicsTextItem>
#include "whitedogs.h"
#include <QTimer>
#include <QGraphicsPixmapItem>
#include <array>
#include "gamepause.h"
#include "waveplanner.h"
class Lawn;

class MyGameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit MyGameScene(int n = 1,QObject *parent = nullptr,bool deferStart=false,bool endless=false,int firstWave=1);
    enum class InputMode { Blocked,Normal,PlantPractice,HeartPractice,ShovelPractice };
    Q_ENUM(InputMode)
    void startGameplay();
    bool gameplayStarted() const { return started; }
    void setInputMode(InputMode mode) { inputMode=mode; }
    void addTutorialPlant(int row,int col);
    void generateTutorialHeart();

    ~MyGameScene() override = default;

    friend class YellowDogs;
    friend class LineWhite;


    void setChosenNum(int cardNum);
    void cancelSelection();
    void toggleShovel();
    void togglePlantHealth();
    void toggleEnemyHealth();
    bool plantHealthVisible() const { return showPlantHealth; }
    bool enemyHealthVisible() const { return showEnemyHealth; }
    int totalEnemies() const { return m_totalZombiesForLevel; }
    int wavesStarted() const { return nextWave; }
    int getChosenNum() const { return chosenNum; }
    Lawn *lawn() const { return grass; }
    QPointF defeatPosition() const { return losingPosition; }
    bool isEndless() const { return endlessMode; }

    int getRestHeart() const { return restHeart; } // 得到剩余爱心的数值

    void cutHeart(int amont){ restHeart -= amont; } //剩余爱心的数值减去消耗爱心数值
    void addHeart(int amont){ restHeart += amont; } //剩余爱心的数量加上收集到爱心的数量
    void removeWhite(int r, int c); //移除小白

    void generateSkyHeart(); //天空中随机生成爱心
    void generateWhiteHeart(QPointF whitePos); //从小白中产出爱心
    void setAYellowDog(int r, int typeNum = 0); //在第r行产生一只小金毛
    void generateBullet(int r, int c); //产生子弹
    WhiteDogs *plantAhead(int row, qreal x) const;
    QTimer *getGameTimer() const { return memGameTimer; } //获取gameTimer
    const QVector<MyItem *>& getZombieMap(int row) const { return zombieMap.at(row); }
    const QVector<WhiteDogs*>& plantsInRow(int row) const { return plantRows.at(row); }
private:
    int gameLevelNum = 0;
    bool endlessMode=false;
    QPointF losingPosition=QPointF(130,450);

    Map *mapGrid = nullptr;  //添加地图网咯
    Lawn *grass=nullptr;
    bool started=false;
    InputMode inputMode=InputMode::Normal;
    std::array<WhiteDogs*,45> dogMap{};
    std::array<QVector<WhiteDogs*>,5> plantRows;
    void removePlant(WhiteDogs *plant);
    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    void keyPressEvent(QKeyEvent *event) override;

    int chosenNum = 0;
    int restHeart = 50; // 剩余阳光初始化为50
    QVector<QVector<MyItem *>> zombieMap;
    GamePause terminalActivity;
    void setupBoard();
    void setupTimers();
    void spawnWave();
    void spawnNextInWave();
    WavePlanner::Plan wavePlan;
    QVector<int> pendingWave;
    int nextWave = 0;
    int pendingIndex = 0;
    std::array<int,5> waveRowCounts{};
    QTimer *waveStaggerTimer = nullptr;
    WhiteDogs *createPlant(int row, int col, const QPointF& center);
    void placePlant(int row, int col,bool charge=true);
    void addHeartItem(class Heart *heart);
    void finishGame(bool won);


    QTimer *memSkyHeartTimer; //天空中的爱心生成计时器
    QTimer *memYellowDogsTimer; //小金毛计时器
    QTimer *memGameTimer; //游戏总的计时器
    QTimer *memLongGameTimer; //0.5秒更新一次的计时器

    void checkWinCondition(); // 检查胜利条件的私有函数

    int m_totalZombiesForLevel = 0; // 本关卡总金毛数
    int m_zombiesSpawned = 0;       // 已生成的金毛数
    int m_zombiesKilled = 0;        // 已消灭的金毛数
    bool m_isGameOver = false;      // 标记游戏是否已结束
    bool showPlantHealth = false;
    bool showEnemyHealth = false;

    QGraphicsPixmapItem *shovel = nullptr; //铲子
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;

signals:
    void healthVisibilityChanged(bool plants, bool enemies);
    void waveStarted(int wave, int total);
    void finalWaveApproaching();
    void plantFinished();
    void plantRemoved(int row,int col);
    void heartCollected();
    void sceneClicked();
    void gameWin();
    void gameLose();
    void shovelPlant(int r, int c);
    void pleaseRemovePreImage();
    void mouseMovedTo(QPointF mousePos);
    void allowTracking();
    void banTracking();
    void changeRestZombieNumber();


public slots:
    void winTheGame();   // 游戏胜利的槽函数
    void loseTheGame();  // 游戏失败的槽函数
};

#endif // MYGAMESCENE_H
