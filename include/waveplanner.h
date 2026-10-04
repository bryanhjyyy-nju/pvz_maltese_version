#pragma once
#include <QVector>
#include <QPair>
class QRandomGenerator;
namespace WavePlanner {
using Plan = QVector<QVector<int>>;
// Per-wave rules take priority; otherwise use the gradual opening and late budgets.
Plan create(int level, QRandomGenerator& random);
int enemyCount(const Plan& plan);
double enemyLikelihood(int type,int level=0);
QVector<int> previewTypes(int level,QRandomGenerator& random);
QVector<int> endlessWave(int wave,QRandomGenerator& random);
bool isEndlessBigWave(int wave);
int endlessWaveWeight(int wave);
double endlessEnemyLikelihood(int type,int wave);
// Rest begins after the last enemy is generated; big waves get twice the rest.
int endlessIntervalAfterWave(int wave);
// Delay before each enemy, including zero before the first. All are game ms.
QVector<int> endlessSpawnDelays(int wave,int count,QRandomGenerator& random);
// Wave numbers start at 1. Wave 1 uses the initial preparation time.
QPair<int,int> intervalBeforeWave(int level,int wave);
}
