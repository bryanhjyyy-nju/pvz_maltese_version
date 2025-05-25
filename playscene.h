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

signals:
};

#endif // PLAYSCENE_H
