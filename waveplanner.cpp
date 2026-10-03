#include "waveplanner.h"
#include "gamecatalog.h"
#include <QRandomGenerator>
WavePlanner::Plan WavePlanner::create(int number, QRandomGenerator& random) {
    const auto& level=GameCatalog::level(number);
    const double likelihood[] = {1.0,.20,.08};
    Plan plan;
    for(int wave=0;wave<level.waves;++wave) {
        QVector<int> enemies;
        int remaining=level.waveWeight;
        while(remaining>0) {
            double total=0;
            for(int type=0;type<=level.maxEnemyType;++type)
                if(GameCatalog::enemies()[type].weight<=remaining) total+=likelihood[type];
            double pick=random.generateDouble()*total;
            int selected=0;
            for(int type=0;type<=level.maxEnemyType;++type) {
                if(GameCatalog::enemies()[type].weight>remaining) continue;
                pick-=likelihood[type];
                if(pick<0) { selected=type; break; }
            }
            enemies.append(selected);
            remaining-=GameCatalog::enemies()[selected].weight;
        }
        plan.append(enemies);
    }
    return plan;
}
int WavePlanner::enemyCount(const Plan& plan) {
    int count=0;
    for(const auto& wave : plan) count+=wave.size();
    return count;
}
