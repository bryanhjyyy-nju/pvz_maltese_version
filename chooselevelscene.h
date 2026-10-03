#ifndef CHOOSELEVELSCENE_H
#define CHOOSELEVELSCENE_H

#include <QMainWindow>
#include "playscene.h"
#include "progressstore.h"
#include <QPushButton>

class ChooseLevelScene : public QMainWindow
{
    Q_OBJECT
public:
    explicit ChooseLevelScene(QWidget *parent = nullptr);

    //重写绘图事件
    void paintEvent(QPaintEvent *) override;

    PlayScene *play = nullptr;
    void startLevel(int level);
    void refreshProgress();
    void continueGame();

private:
    ProgressStore progress;
    QPushButton *continueButton = nullptr;
    QLabel *progressLabel = nullptr;
    void buildBackBtn();
    void buildLevelBtn();

signals:
    //自定义信号告诉主场景点击了返回
    void chooseSceneBack();
};

#endif // CHOOSELEVELSCENE_H
