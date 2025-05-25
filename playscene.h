#ifndef PLAYSCENE_H
#define PLAYSCENE_H

#include <QMainWindow>

class PlayScene : public QMainWindow
{
    Q_OBJECT
public:
    explicit PlayScene(QWidget *parent = nullptr);

    //构造函数：第几关
    PlayScene(int levelNum);

    //内部成员记录关卡好
    int levelIndex;

    //重写画背景图事件
    void paintEvent(QPaintEvent *);

    //枚举白色小狗类型，冷却时间，花费爱心数量
    QVector<QString> whiteTypes = {"singingWhite", //相当于豌豆射手
                                   "heartWhite", //相当于向日葵
                                   "wallWhite", //相当于坚果
                                   "lineWhite", //相当于火爆辣椒
                                   "doubleWhite", //相当于大蒜
                                   "allHeartWhite", //樱桃炸弹并释放爱心
                                   "dblSingWhite", //相当于双发射手
                                   "moneyWhite"}; //相当于寒冰菇
    QVector<int> heartCosts = {100, 50,1,1,1,1,1,1};
    QVector<int> coolTimes = {7500,5000,1,1,1,1,1,1};

    //卡片上小狗的大小
    QVector<int> sizes = {110,85,80,80,85,90,80,90};

    //卡片上小狗位置微调
    QVector<int> Xs = {-13,0,0,0,5,-5,0,0};
    QVector<int> Ys = {-3,5,5,10,0,0,0,0};

    QVector<QString> whiteImages = {":/Image/singingWhite.gif",
                                    ":/Image/heartWhite.gif",
                                    ":/Image/wallWhite.gif",
                                    ":/Image/lineWhite.gif",
                                    ":/Image/doubleWhite.gif",
                                    ":/Image/allHeartWhite.gif",
                                    ":/Image/dblSingWhite.gif",
                                    ":/Image/moneyWhite.gif"};
signals:
    void playSceneBack();
};

#endif // PLAYSCENE_H
