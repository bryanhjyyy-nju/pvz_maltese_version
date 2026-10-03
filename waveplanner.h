#pragma once
#include <QVector>
class QRandomGenerator;
namespace WavePlanner {
using Plan = QVector<QVector<int>>;
// Integer budgets are filled exactly; ineligible enemies are excluded before sampling.
Plan create(int level, QRandomGenerator& random);
int enemyCount(const Plan& plan);
}
