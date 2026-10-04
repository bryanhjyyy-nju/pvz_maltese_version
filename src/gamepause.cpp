#include "gamepause.h"
#include <memory>
#include <QVariant>

void GamePause::pause(QObject *root) {
    for(auto *timer : root->findChildren<QTimer*>()) {
        if(!timer->isActive()) continue;
        if(auto *gameTimer=qobject_cast<GameTimer*>(timer)) {
            gameTimers.append(gameTimer);
            gameTimer->pauseGame();
            continue;
        }
        const auto original = timer->property("resumeOriginalInterval");
        timers.append({timer,qMax(1,timer->remainingTime()),original.isValid() ? original.toInt() : timer->interval()});
        timer->stop();
    }
    for(auto *animation : root->findChildren<QAbstractAnimation*>()) {
        if(animation->state() != QAbstractAnimation::Running) continue;
        animations.append(animation);
        animation->pause();
    }
    for(auto *movie : root->findChildren<QMovie*>()) {
        if(movie->state() != QMovie::Running) continue;
        movies.append(movie);
        movie->setPaused(true);
    }
}
void GamePause::resume() {
    for(auto timer : gameTimers) if(timer) timer->resumeGame();
    for(const auto& saved : timers) {
        if(!saved.timer) continue;
        auto *timer = saved.timer.data();
        // Resume the partial interval once, then restore the recurring interval.
        // A gameplay callback may select a new interval (e.g. the next enemy wave).
        timer->setProperty("resumeOriginalInterval",saved.interval);
        const int generation = timer->property("resumeGeneration").toInt() + 1;
        timer->setProperty("resumeGeneration",generation);
        auto connection = std::make_shared<QMetaObject::Connection>();
        *connection = QObject::connect(timer,&QTimer::timeout,timer,[saved,connection,generation] {
            if(saved.timer && saved.timer->property("resumeGeneration").toInt() == generation) {
                if(saved.timer->interval() == saved.remaining)
                    saved.timer->setInterval(saved.interval);
                saved.timer->setProperty("resumeOriginalInterval",QVariant());
            }
            QObject::disconnect(*connection);
        });
        timer->start(saved.remaining);
    }
    for(auto animation : animations) if(animation) animation->resume();
    for(auto movie : movies) if(movie) movie->setPaused(false);
    timers.clear(); gameTimers.clear(); animations.clear(); movies.clear();
}
