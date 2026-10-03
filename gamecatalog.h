#pragma once

#include <QString>
#include <QVector>

// One source of truth for gameplay, the card bar and the almanac.
namespace GameCatalog {
constexpr int LevelCount = 10;
constexpr int BulletDamage = 30;
constexpr int HeartValue = 25;
struct Plant {
    QString id, name, image, description;
    int health, cost, cooldownMs, actionIntervalMs;
    int iconSize, iconX, iconY;
};
struct Enemy {
    QString name, image, description;
    int health, attack, minSpeed, speedRange;
    double scale;
};
struct Level {
    int enemies, minRow, maxRow, minInterval, maxInterval;
    double toughChance, quickChance;
    int perWave;
    double extraChance, extraTwoChance;
};
const QVector<Plant>& plants();
const QVector<Enemy>& enemies();
const Level& level(int number);
QString plantDetails(int index);
QString enemyDetails(int index);
}
