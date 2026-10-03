#pragma once
#include <QFrame>
class MyGameScene;
class QLabel;
class QPushButton;

// Actions advance the lesson; battle timers stay stopped until finished.
class LevelTutorial : public QFrame {
    Q_OBJECT
public:
    enum class Step { Plant,Heart,Controls,Shovel,Done };
    Q_ENUM(Step)
    LevelTutorial(int level,MyGameScene *scene,QWidget *parent);
    void start();
    Step step() const { return current; }
    void notePauseUsed();
    void noteAlmanacViewed();
    void fitCanvas(qreal scale,const QPointF& offset);
signals:
    void stepChanged(LevelTutorial::Step step);
    void finished();
private:
    int level,removed=0;
    MyGameScene *scene;
    Step current=Step::Done;
    bool usedPause=false,viewedAlmanac=false;
    QLabel *title,*instructions;
    QPushButton *next;
    void enter(Step step);
    void updateControlsLesson();
};
