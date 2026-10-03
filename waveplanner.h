#pragma once
#include <QVector>
class QRandomGenerator;
namespace WavePlanner {
using Plan = QVector<QVector<int>>;
// Integer budgets are filled exactly; ineligible enemies are excluded before sampling.
Plan create(int level, QRandomGenerator& random);
int enemyCount(const Plan& plan);
double enemyLikelihood(int type);
QVector<int> previewTypes(int level,QRandomGenerator& random);
QVector<int> endlessWave(int wave,QRandomGenerator& random);
}
