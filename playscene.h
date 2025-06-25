#ifndef PLAYSCENE_H
#define PLAYSCENE_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVector>
#include "mygamescene.h"
#include "card.h"
#include <QLabel>


class PlayScene : public QMainWindow
{
    Q_OBJECT
public:
    explicit PlayScene(QWidget *parent = nullptr);

    //内部成员记录关卡号
    const int levelIndex;

    //构造函数：第几关
    PlayScene(int levelNum);

    void gamePaused();
    void gameContinued();

private:
    //注：此处卡牌的小狗是直接画上去的
    //枚举白色小狗类型，冷却时间，花费爱心数量
    QVector<QString> whiteTypes = {"singingWhite", //相当于豌豆射手
                                   "heartWhite", //相当于向日葵
                                   "wallWhite", //相当于坚果
                                   "lineWhite", //相当于小推车
                                   "dancingWhite", //减速效果
                                   "allHeartWhite", //产爱心更快的向日葵
                                   "dblSingWhite", //发射子弹更快的豌豆射手
                                   "moneyWhite"}; //毫无效果
    QVector<int> heartCosts = {100, 50,50,50,50,125,200,0}; //消耗爱心数量枚举
    QVector<int> coolTimes = {7500,5000,15000,7500,7500,12500,15000,10000}; //冷却时间枚举

    QVector<Card *> myCards;

    //设置卡片上小狗的大小
    QVector<int> sizes = {110,85,80,80,85,90,80,90};

    //对卡片上小狗位置微调
    QVector<int> Xs = {-13,0,0,0,5,-5,0,0};
    QVector<int> Ys = {-3,5,5,10,0,0,0,0};

    QVector<QString> whiteImages = {":/white/Image/singingWhite.gif",
                                    ":/white/Image/heartWhite.gif",
                                    ":/white/Image/wallWhite.gif",
                                    ":/white/Image/lineWhite.gif",
                                    ":/white/Image/dancingWhite.gif",
                                    ":/white/Image/allHeartWhite.gif",
                                    ":/white/Image/dblSingWhite.gif",
                                    ":/white/Image/moneyWhite.gif"};

    void buildBackBtn();
    void buildPauseBtn();
    void setLevelText();
    void setCardBar();
    void setCardsInBar();

    MyGameScene* myGameScene;  // 图形场景
    QGraphicsView* myGraphicsView;    // 可视化视图
    QLabel *restHeartLabel = NULL; //显示剩余阳光

signals:
    void playSceneBack();
    void signalToCard();
    void gameLose();
    void gameWin();
    // void gamePause();

private slots:
    void handleCardSelected(Card* card);

};

#endif // PLAYSCENE_H
