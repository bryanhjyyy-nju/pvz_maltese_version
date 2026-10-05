#ifndef PLAYSCENE_H
#define PLAYSCENE_H

#include "gamepage.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVector>
#include "mygamescene.h"
#include "card.h"
#include <QLabel>
#include "gamepause.h"


class PlayScene : public GamePage
{
    Q_OBJECT
    friend class BattleSnapshot;
public:

    //内部成员记录关卡号
    const int levelIndex;
    const bool endlessMode;

    //构造函数：第几关
    PlayScene(int levelNum,QWidget *parent=nullptr,bool withOpening=true,bool endless=false,int firstWave=1);
    void shutdown();
    bool isPaused() const { return paused; }
    bool isFinished() const { return finished; }
    void showPauseMenu();
    void suspendToMenu();
    int speedMultiplier() const { return myGameScene->gameSpeed()->multiplier(); }
    void setSpeedMultiplier(int multiplier);

    void gamePaused();
    void gameContinued();

protected:
    bool handleGameKey(QKeyEvent *event) override;

private:
    GamePause pausedActivity;
    bool paused = false;
    GameState interactionBeforePause = GameState::Normal;
    bool previewBeforePause = false;
    bool finished = false;
    bool openingActive=false;
    qreal cameraOffset=0;
    class LevelOpening *opening=nullptr;
    class BattleBanner *banner=nullptr;
    class LevelTutorial *tutorial=nullptr;
    class BattleResult *result=nullptr;
    void showResult(bool won);
    void finishOpening();
    void beginGameplay();
    void setBattleHudVisible(bool visible);
    QPushButton *pauseButton = nullptr;
    QPushButton *speedButton = nullptr;
    class PauseDialog *pauseMenu = nullptr;
    class QShortcut *pauseShortcut = nullptr;
    void togglePauseMenu();
    void showAlmanac();
    void showAudioSettings();
    void finishGame();
    void fitBattlefield();
    QVector<Card *> myCards;
    void refreshCardSelection();

    void buildPauseBtn();
    void setLevelText();
    void setCardBar();
    void setCardsInBar();

    MyGameScene* myGameScene;  // 图形场景
    QGraphicsView* myGraphicsView;    // 可视化视图
    QLabel *restHeartLabel = NULL; //显示剩余阳光

    QLabel *preImageLabel = NULL;
    QPointF previewScenePosition;
    void startShow(int num);
    void stopShow();

signals:
    void restartRequested();
    void nextLevelRequested(int level);
    void mainMenuRequested();
    void playSceneBack();
    void signalToCard();
    void gameLose();
    void gameWin();

private slots:
    void handleCardSelected(Card* card);

};

#endif // PLAYSCENE_H
