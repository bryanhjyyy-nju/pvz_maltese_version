#ifndef PLAYSCENE_H
#define PLAYSCENE_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVector>
#include "mygamescene.h"
#include "card.h"
#include <QLabel>
#include "gamepause.h"


class PlayScene : public QMainWindow
{
    Q_OBJECT
public:

    //内部成员记录关卡号
    const int levelIndex;

    //构造函数：第几关
    PlayScene(int levelNum);

    void gamePaused();
    void gameContinued();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    GamePause pausedActivity;
    bool paused = false;
    bool finished = false;
    QPushButton *pauseButton = nullptr;
    class PauseDialog *pauseMenu = nullptr;
    class QShortcut *pauseShortcut = nullptr;
    void togglePauseMenu();
    void showAlmanac();
    void showAudioSettings();
    void finishGame();
    QVector<Card *> myCards;

    void buildBackBtn();
    void buildPauseBtn();
    void setLevelText();
    void setCardBar();
    void setCardsInBar();

    MyGameScene* myGameScene;  // 图形场景
    QGraphicsView* myGraphicsView;    // 可视化视图
    QLabel *restHeartLabel = NULL; //显示剩余阳光

    QLabel *preImageLabel = NULL;
    void startShow(int num);
    void stopShow();

signals:
    void mainMenuRequested();
    void playSceneBack();
    void signalToCard();
    void gameLose();
    void gameWin();
    // void gamePause();

private slots:
    void handleCardSelected(Card* card);

};

#endif // PLAYSCENE_H
