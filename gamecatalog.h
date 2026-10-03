#pragma once

#include <QString>
#include <QVector>

// One source of truth for gameplay, the card bar and the almanac.
namespace GameCatalog {
constexpr int LevelCount = 10;
constexpr int BulletDamage = 30;
constexpr int HeartValue = 25;
constexpr int GuitarShotIntervalMs = 4000;
struct Plant {
    QString id, name, image, description;
    int health, cost, cooldownMs, actionIntervalMs;
    int iconSize, iconX, iconY;
};
struct Enemy {
    QString name, image, description;
    int health, attack, minSpeed, speedRange;
    double scale;
    int weight;
};
struct Level {
    int waves, waveWeight, minRow, maxRow, minInterval, maxInterval;
    int maxEnemyType, initialDelayMs, startingHearts;
};
const QVector<Plant>& plants();
const QVector<Enemy>& enemies();
const Level& level(int number);
QString plantDetails(int index);
QString enemyDetails(int index);
}
