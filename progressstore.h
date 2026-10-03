#pragma once
#include <QString>

// Saves level checkpoints, not an in-progress battlefield.
class ProgressStore {
public:
    explicit ProgressStore(QString filePath = {});
    bool load();
    bool startLevel(int level);
    bool completeLevel(int level);
    int resumeLevel() const { return m_resumeLevel; }
    int highestCompleted() const { return m_highestCompleted; }
    bool hasProgress() const { return m_hasProgress; }
    QString error() const { return m_error; }
    QString filePath() const { return m_path; }
private:
    bool save(int resumeLevel, int highestCompleted);
    QString m_path, m_error;
    int m_resumeLevel = 1, m_highestCompleted = 0;
    bool m_hasProgress = false;
};
