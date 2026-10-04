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
int endlessEnemyCount(int wave);
double endlessEnemyLikelihood(int type,int wave);
// Wave numbers start at 1. Wave 1 uses the initial preparation time.
QPair<int,int> intervalBeforeWave(int level,int wave);
}
