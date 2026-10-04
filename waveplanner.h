#pragma once
#include <QVector>
class QRandomGenerator;
namespace WavePlanner {
using Plan = QVector<QVector<int>>;
// Middle waves fill the threat budget. Levels 2–8 ease the opening; custom
// late-wave budgets guarantee guitars and retain growing enemy counts.
Plan create(int level, QRandomGenerator& random);
int enemyCount(const Plan& plan);
double enemyLikelihood(int type,int level=0);
QVector<int> previewTypes(int level,QRandomGenerator& random);
QVector<int> endlessWave(int wave,QRandomGenerator& random);
}
