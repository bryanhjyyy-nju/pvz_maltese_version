#pragma once
#include <QJsonObject>
class PlayScene;

// Portable battle state. Qt objects and signal connections are rebuilt on load.
class BattleSnapshot {
public:
    static QJsonObject capture(PlayScene& play);
    static bool isValid(const QJsonObject& state);
    static bool restore(PlayScene& play,const QJsonObject& state);
};
