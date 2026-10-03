#include "waveplanner.h"
#include "gamecatalog.h"
#include <QRandomGenerator>
double WavePlanner::enemyLikelihood(int type) {
    static const double values[]={1.0,.20,.08};
    return type>=0 && type<3 ? values[type] : 0;
}
WavePlanner::Plan WavePlanner::create(int number, QRandomGenerator& random) {
    const auto& level=GameCatalog::level(number);
    Plan plan;
    for(int wave=0;wave<level.waves;++wave) {
        QVector<int> enemies;
        int remaining=level.waveWeight;
        while(remaining>0) {
            double total=0;
            for(int type=0;type<=level.maxEnemyType;++type)
                if(GameCatalog::enemies()[type].weight<=remaining) total+=enemyLikelihood(type);
            double pick=random.generateDouble()*total;
            int selected=0;
            for(int type=0;type<=level.maxEnemyType;++type) {
                if(GameCatalog::enemies()[type].weight>remaining) continue;
                pick-=enemyLikelihood(type);
                if(pick<0) { selected=type; break; }
            }
            enemies.append(selected);
            remaining-=GameCatalog::enemies()[selected].weight;
        }
        plan.append(enemies);
    }
    return plan;
}
QVector<int> WavePlanner::previewTypes(int number,QRandomGenerator& random) {
    const int maxType=GameCatalog::level(number).maxEnemyType;
    const int count=maxType==0 ? 5 : 12;
    double total=0; for(int type=0;type<=maxType;++type) total+=enemyLikelihood(type);
    QVector<int> counts(3,0); QVector<double> remainder(3,0);
    int assigned=0;
    for(int type=0;type<=maxType;++type) {
        const double expected=count*enemyLikelihood(type)/total;
        counts[type]=int(expected); remainder[type]=expected-counts[type]; assigned+=counts[type];
    }
    while(assigned<count) {
        int best=0; for(int type=1;type<=maxType;++type) if(remainder[type]>remainder[best]) best=type;
        ++counts[best]; remainder[best]=-1; ++assigned;
    }
    QVector<int> result;
    for(int type=0;type<=maxType;++type) for(int i=0;i<counts[type];++i) result.append(type);
    for(int i=result.size()-1;i>0;--i) result.swapItemsAt(i,random.bounded(i+1));
    return result;
}
int WavePlanner::enemyCount(const Plan& plan) {
    int count=0;
    for(const auto& wave : plan) count+=wave.size();
    return count;
}
