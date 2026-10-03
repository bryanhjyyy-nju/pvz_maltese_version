#include "progressstore.h"
#include "gamecatalog.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

ProgressStore::ProgressStore(QString filePath) : m_path(filePath) {
    if(m_path.isEmpty())
        m_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/progress.json";
    load();
}
bool ProgressStore::load() {
    m_error.clear();
    m_resumeLevel = 1; m_highestCompleted = 0; m_hasProgress = false;
    unfinished=false; endlessActive=false; bestWave=0; checkpoint=1;
    QFile file(m_path);
    if(!file.exists()) return true;
    if(!file.open(QIODevice::ReadOnly)) { m_error = file.errorString(); return false; }
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(file.readAll(), &parseError);
    const auto object = document.object();
    const int resume = object.value("resumeLevel").toInt(-1);
    const int completed = object.value("highestCompleted").toInt(-1);
    if(parseError.error != QJsonParseError::NoError || !document.isObject()
        || (object.value("version").toInt(-1)!=1 && object.value("version").toInt(-1)!=2) || resume < 1
        || resume > GameCatalog::LevelCount || completed < 0 || completed > GameCatalog::LevelCount) {
        m_error = "存档格式无效；可重新选择关卡开始。";
        return false;
    }
    m_resumeLevel = resume; m_highestCompleted = completed; m_hasProgress = true;
    if(object.value("version").toInt()==2) {
        if(!object.value("unfinished").isBool() || !object.value("endlessActive").isBool()
            || !object.value("endlessBest").isDouble() || !object.value("endlessCheckpoint").isDouble()
            || object.value("endlessBest").toInt(-1)<0 || object.value("endlessCheckpoint").toInt(0)<1) {
            m_hasProgress=false; m_highestCompleted=0; m_resumeLevel=1;
            m_error="存档状态无效。"; return false;
        }
        unfinished=object.value("unfinished").toBool(); endlessActive=object.value("endlessActive").toBool();
        bestWave=object.value("endlessBest").toInt(); checkpoint=object.value("endlessCheckpoint").toInt();
        if((unfinished && !isUnlocked(resume)) || (endlessActive && (!endlessUnlocked() || unfinished))) {
            m_hasProgress=false; m_resumeLevel=1; m_highestCompleted=0;
            unfinished=false; endlessActive=false; bestWave=0; checkpoint=1;
            m_error="存档解锁状态无效。"; return false;
        }
    }
    return true;
}
bool ProgressStore::writeState(int resume,int completed,bool active,bool endless,int best,int wave) {
    m_error.clear();
    if(!QDir().mkpath(QFileInfo(m_path).absolutePath())) {
        m_error = "无法创建存档目录。"; return false;
    }
    QSaveFile file(m_path);
    if(!file.open(QIODevice::WriteOnly)) { m_error = file.errorString(); return false; }
    const QJsonObject object{{"version",2},{"resumeLevel",resume},{"highestCompleted",completed},
        {"unfinished",active},{"endlessActive",endless},{"endlessBest",best},{"endlessCheckpoint",wave}};
    const auto bytes = QJsonDocument(object).toJson();
    if(file.write(bytes) != bytes.size() || !file.commit()) { m_error = file.errorString(); return false; }
    m_resumeLevel = resume; m_highestCompleted = completed; m_hasProgress = true;
    unfinished=active; endlessActive=endless; bestWave=best; checkpoint=wave;
    return true;
}
bool ProgressStore::startLevel(int level) {
    if(!isUnlocked(level)) { m_error = "关卡尚未解锁。"; return false; }
    return writeState(level,m_highestCompleted,true,false,bestWave,checkpoint);
}
bool ProgressStore::completeLevel(int level) {
    if(!isUnlocked(level)) { m_error = "关卡尚未解锁。"; return false; }
    return writeState(qMin(level+1,GameCatalog::LevelCount),qMax(level,m_highestCompleted),false,false,bestWave,checkpoint);
}
int ProgressStore::unlockedLevel() const { return qMin(10,m_highestCompleted+1); }
bool ProgressStore::isUnlocked(int level) const { return level>=1 && level<=unlockedLevel(); }
bool ProgressStore::finishAttempt() { return writeState(m_resumeLevel,m_highestCompleted,false,false,bestWave,checkpoint); }
bool ProgressStore::reset() { return writeState(1,0,false,false,0,1); }
bool ProgressStore::unlockAll() { return writeState(m_resumeLevel,10,unfinished,endlessActive,bestWave,checkpoint); }
bool ProgressStore::startEndless() {
    if(!endlessUnlocked()) { m_error="通过第十关后解锁无尽模式。"; return false; }
    return writeState(m_resumeLevel,m_highestCompleted,false,true,bestWave,1);
}
bool ProgressStore::recordEndlessWave(int wave) {
    if(!endlessActive || wave<1) return false;
    return writeState(m_resumeLevel,m_highestCompleted,false,true,qMax(bestWave,wave),wave);
}
bool ProgressStore::finishEndless() { return writeState(m_resumeLevel,m_highestCompleted,false,false,bestWave,1); }
