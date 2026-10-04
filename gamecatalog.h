#pragma once

#include <QString>
#include <QVector>

// One source of truth for gameplay, the card bar and the almanac.
namespace GameCatalog {
constexpr int LevelCount = 10;
constexpr int BulletDamage = 30;
constexpr int HeartValue = 25;
constexpr int SkyHeartFallDurationMs = 8000;
constexpr int SkyHeartStartY = 140; // Below the card bar, inside the painted canvas.
constexpr int SkyHeartMinIntervalMs = 5000;
constexpr int SkyHeartMaxIntervalMs = 6000;
constexpr int GuitarShotIntervalMs = 2000;
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
