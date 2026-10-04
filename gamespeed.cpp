#include "gamespeed.h"

void GameSpeed::setMultiplier(int multiplier) {
    multiplier=multiplier==2 ? 2 : 1;
    if(rate==multiplier) return;
    const int previous=rate;
    rate=multiplier;
    emit speedChanged(previous,rate);
}
GameSpeed *GameSpeed::forObject(QObject *object) {
    for(auto *owner=object;owner;owner=owner->parent()) {
        if(auto *clock=qobject_cast<GameSpeed*>(owner)) return clock;
        if(auto *clock=owner->findChild<GameSpeed*>(QString(),Qt::FindDirectChildrenOnly)) return clock;
    }
    return nullptr;
}
GameTimer::GameTimer(QObject *parent,GameSpeed *clock) : QTimer(parent),clock(clock) {
    setTimerType(Qt::PreciseTimer);
    // Restore the recurring interval before gameplay chooses the next interval.
    connect(this,&QTimer::timeout,this,[this] {
        if(!partial) return;
        partial=false;
        QTimer::setInterval(wallTime(nominalInterval));
    });
    if(clock) connect(clock,&GameSpeed::speedChanged,this,&GameTimer::changeSpeed);
}
int GameTimer::multiplier() const { return clock ? clock->multiplier() : 1; }
int GameTimer::wallTime(double gameMilliseconds) const { return qMax(1,qRound(gameMilliseconds/multiplier())); }
void GameTimer::setInterval(int gameMilliseconds) {
    nominalInterval=qMax(0,gameMilliseconds);
    partial=false;
    QTimer::setInterval(wallTime(nominalInterval));
}
void GameTimer::start() {
    paused=false; partial=false;
    QTimer::start(wallTime(nominalInterval));
}
void GameTimer::setIntervalPreservingProgress(int gameMilliseconds) {
    const bool running=isActive();
    const double remaining=paused ? pausedRemaining : qMax(0,remainingTime())*multiplier();
    const double fraction=remaining/qMax(1,nominalInterval);
    QTimer::stop();
    setInterval(gameMilliseconds);
    if(paused) pausedRemaining=fraction*nominalInterval;
    else if(running) scheduleRemaining(fraction*nominalInterval);
}
void GameTimer::start(int gameMilliseconds) { setInterval(gameMilliseconds); start(); }
void GameTimer::stop() { paused=false; partial=false; QTimer::stop(); }
void GameTimer::scheduleRemaining(double gameMilliseconds) {
    partial=true;
    QTimer::start(wallTime(gameMilliseconds));
}
void GameTimer::changeSpeed(int previous,int) {
    const bool running=isActive();
    const double remaining=qMax(0,remainingTime())*previous;
    QTimer::stop();
    QTimer::setInterval(wallTime(nominalInterval));
    if(running) scheduleRemaining(remaining);
}
void GameTimer::pauseGame() {
    if(!isActive() || paused) return;
    pausedRemaining=qMax(0,remainingTime())*multiplier();
    paused=true;
    QTimer::stop();
}
void GameTimer::resumeGame() {
    if(!paused) return;
    paused=false;
    scheduleRemaining(pausedRemaining);
}
