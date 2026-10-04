#pragma once
#include <QVector>
class QRandomGenerator;
namespace WavePlanner {
using Plan = QVector<QVector<int>>;
// Middle waves fill the threat budget. Levels 2–8 ease the opening and pad
// the last two waves with ordinary dogs so the final wave has the most enemies.
Plan create(int level, QRandomGenerator& random);
int enemyCount(const Plan& plan);
double enemyLikelihood(int type);
QVector<int> previewTypes(int level,QRandomGenerator& random);
QVector<int> endlessWave(int wave,QRandomGenerator& random);
}
