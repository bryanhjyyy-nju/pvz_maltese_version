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
    QFile file(m_path);
    if(!file.exists()) return true;
    if(!file.open(QIODevice::ReadOnly)) { m_error = file.errorString(); return false; }
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(file.readAll(), &parseError);
    const auto object = document.object();
    const int resume = object.value("resumeLevel").toInt(-1);
    const int completed = object.value("highestCompleted").toInt(-1);
    if(parseError.error != QJsonParseError::NoError || !document.isObject()
        || object.value("version").toInt(-1) != 1 || resume < 1
        || resume > GameCatalog::LevelCount || completed < 0 || completed > GameCatalog::LevelCount) {
        m_error = "存档格式无效；可重新选择关卡开始。";
        return false;
    }
    m_resumeLevel = resume; m_highestCompleted = completed; m_hasProgress = true;
    return true;
}
bool ProgressStore::save(int resume, int completed) {
    m_error.clear();
    if(!QDir().mkpath(QFileInfo(m_path).absolutePath())) {
        m_error = "无法创建存档目录。"; return false;
    }
    QSaveFile file(m_path);
    if(!file.open(QIODevice::WriteOnly)) { m_error = file.errorString(); return false; }
    const QJsonObject object{{"version",1},{"resumeLevel",resume},{"highestCompleted",completed}};
    const auto bytes = QJsonDocument(object).toJson();
    if(file.write(bytes) != bytes.size() || !file.commit()) { m_error = file.errorString(); return false; }
    m_resumeLevel = resume; m_highestCompleted = completed; m_hasProgress = true;
    return true;
}
bool ProgressStore::startLevel(int level) {
    if(level < 1 || level > GameCatalog::LevelCount) { m_error = "无效关卡。"; return false; }
    return save(level,m_highestCompleted);
}
bool ProgressStore::completeLevel(int level) {
    if(level < 1 || level > GameCatalog::LevelCount) { m_error = "无效关卡。"; return false; }
    return save(qMin(level+1,GameCatalog::LevelCount),qMax(level,m_highestCompleted));
}
