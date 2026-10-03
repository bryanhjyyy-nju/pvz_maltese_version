#pragma once
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QAbstractAnimation>
#include <QMovie>
#include <QVector>

// Captures only running activity, so idle shooters and stopped spawn timers stay idle.
class GamePause {
public:
    void pause(QObject *root);
    void resume();
private:
    struct TimerState { QPointer<QTimer> timer; int remaining, interval; };
    QVector<TimerState> timers;
    QVector<QPointer<QAbstractAnimation>> animations;
    QVector<QPointer<QMovie>> movies;
};
