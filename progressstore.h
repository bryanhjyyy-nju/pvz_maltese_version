#pragma once
#include <QString>
#include <QJsonObject>

// Saves unlocked progress and an optional complete battlefield atomically.
class ProgressStore {
public:
    explicit ProgressStore(QString filePath = {});
    bool load();
    bool startLevel(int level);
    bool completeLevel(int level);
    bool finishAttempt();
    bool reset();
    bool unlockAll();
    bool startEndless();
    bool recordEndlessWave(int wave);
    bool finishEndless();
    bool saveBattle(const QJsonObject& snapshot);
    QJsonObject battleSnapshot() const { return battlefield; }
    int unlockedLevel() const;
    bool isUnlocked(int level) const;
    bool hasUnfinishedLevel() const { return unfinished; }
    bool endlessUnlocked() const { return m_highestCompleted==10; }
    bool hasEndlessRun() const { return endlessActive; }
    int endlessBest() const { return bestWave; }
    int endlessCheckpoint() const { return checkpoint; }
    int resumeLevel() const { return m_resumeLevel; }
    int highestCompleted() const { return m_highestCompleted; }
    bool hasProgress() const { return m_hasProgress; }
    QString error() const { return m_error; }
    QString filePath() const { return m_path; }
private:
    QString m_path, m_error;
    int m_resumeLevel = 1, m_highestCompleted = 0;
    bool m_hasProgress = false;
    bool unfinished=false,endlessActive=false;
    int bestWave=0,checkpoint=1;
    QJsonObject battlefield;
    bool writeState(int resume,int completed,bool active,bool endless,int best,int wave,const QJsonObject& snapshot={});
};
