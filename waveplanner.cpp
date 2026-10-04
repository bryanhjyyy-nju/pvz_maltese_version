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
    const bool gradualOpening=number>=2 && number<=8;
    for(int wave=0;wave<level.waves;++wave) {
        QVector<int> enemies;
        if(gradualOpening && wave<2) {
            enemies.fill(0,wave==0 ? 1 : qMax(1,level.waveWeight-1));
            plan.append(enemies);
            continue;
        }
        // Dash dogs cost five points. Allow them in the last two waves of
        // levels 7 and 8, while keeping the existing opening and wave counts.
        int remaining=number>=7 && number<=8 && wave>=level.waves-2
            ? qMax(level.waveWeight,GameCatalog::enemies()[2].weight) : level.waveWeight;
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
        if(gradualOpening && wave>=level.waves-2) {
            // Keep the existing rare-enemy draw, then add ordinary dogs to make
            // the final two waves larger even when a heavy enemy was selected.
            const int count=(number==2 ? level.waveWeight : level.waveWeight+1)
                +(wave==level.waves-1 ? 1 : 0);
            while(enemies.size()<count) enemies.append(0);
            for(int i=enemies.size()-1;i>0;--i) enemies.swapItemsAt(i,random.bounded(i+1));
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
QVector<int> WavePlanner::endlessWave(int wave,QRandomGenerator& random) {
    // Two more threat points each wave; all enemy types are eligible.
    int remaining=4+2*qMax(1,wave);
    QVector<int> result;
    while(remaining>0) {
        double total=0;
        for(int type=0;type<3;++type) if(GameCatalog::enemies()[type].weight<=remaining) total+=enemyLikelihood(type);
        double pick=random.generateDouble()*total;
        int selected=0;
        for(int type=0;type<3;++type) {
            if(GameCatalog::enemies()[type].weight>remaining) continue;
            pick-=enemyLikelihood(type);
            if(pick<0) { selected=type; break; }
        }
        result.append(selected); remaining-=GameCatalog::enemies()[selected].weight;
    }
    return result;
}
