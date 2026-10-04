#pragma once

#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QSignalBlocker>
#include <utility>

// One clock per battlefield. Durations in gameplay code stay in game milliseconds.
class GameSpeed : public QObject {
    Q_OBJECT
public:
    explicit GameSpeed(QObject *parent=nullptr) : QObject(parent) {}
    int multiplier() const { return rate; }
    void setMultiplier(int multiplier);
    static GameSpeed *forObject(QObject *object);
signals:
    void speedChanged(int previous,int current);
private:
    int rate=1;
};

class GameTimer : public QTimer {
    Q_OBJECT
public:
    GameTimer(QObject *parent,GameSpeed *clock);
    void setInterval(int gameMilliseconds);
    void setIntervalPreservingProgress(int gameMilliseconds);
    int gameInterval() const { return nominalInterval; }
    void start();
    void start(int gameMilliseconds);
    void stop();
    void pauseGame();
    void resumeGame();
private:
    QPointer<GameSpeed> clock;
    int nominalInterval=0;
    bool partial=false,paused=false;
    double pausedRemaining=0;
    int multiplier() const;
    int wallTime(double gameMilliseconds) const;
    void scheduleRemaining(double gameMilliseconds);
    void changeSpeed(int previous,int current);
};

// Qt animates in wall time. Rescale duration and elapsed time together so a
// speed switch changes the remaining time without moving the actor or effect.
template<class Base>
class GameAnimation : public Base {
public:
    template<class... Args>
    explicit GameAnimation(GameSpeed *clock,Args&&... args)
        : Base(std::forward<Args>(args)...),clock(clock),nominalDuration(Base::duration()) {
        applySpeed();
        if(clock) QObject::connect(clock,&GameSpeed::speedChanged,this,[this](int,int) { applySpeed(); });
    }
    void setDuration(int gameMilliseconds) {
        nominalDuration=gameMilliseconds;
        Base::setDuration(wallDuration());
    }
    void setDurationPreservingProgress(int gameMilliseconds) {
        nominalDuration=gameMilliseconds;
        applySpeed();
    }
private:
    QPointer<GameSpeed> clock;
    int nominalDuration;
    int wallDuration() const { return qMax(1,qRound(double(nominalDuration)/(clock ? clock->multiplier() : 1))); }
    void applySpeed() {
        const int oldDuration=qMax(1,Base::duration());
        const int elapsed=Base::currentTime(),duration=wallDuration();
        const QSignalBlocker blocker(this);
        // Shortening a live animation below its elapsed time must not finish it.
        Base::setCurrentTime(0);
        Base::setDuration(duration);
        Base::setCurrentTime(qRound(double(elapsed)*duration/oldDuration));
    }
};
using GamePropertyAnimation=GameAnimation<QPropertyAnimation>;
using GameVariantAnimation=GameAnimation<QVariantAnimation>;
