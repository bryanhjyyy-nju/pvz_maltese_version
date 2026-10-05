#include "waveplanner.h"
#include "gamecatalog.h"
#include <QRandomGenerator>
namespace {
void fillBudget(QVector<int>& enemies,int remaining,int maxType,int level,
                QRandomGenerator& random,int minimumCount=0) {
    const auto eligible=[&](int type) {
        const int cost=GameCatalog::enemies()[type].weight;
        return cost<=remaining && enemies.size()+1+remaining-cost>=minimumCount;
    };
    while(remaining>0) {
        double total=0;
        for(int type=0;type<=maxType;++type)
            if(eligible(type)) total+=WavePlanner::enemyLikelihood(type,level);
        double pick=random.generateDouble()*total;
        int selected=0;
        for(int type=0;type<=maxType;++type) {
            if(!eligible(type)) continue;
            pick-=WavePlanner::enemyLikelihood(type,level);
            if(pick<0) { selected=type; break; }
        }
        enemies.append(selected);
        remaining-=GameCatalog::enemies()[selected].weight;
    }
}
void shuffle(QVector<int>& enemies,QRandomGenerator& random) {
    for(int i=enemies.size()-1;i>0;--i) enemies.swapItemsAt(i,random.bounded(i+1));
}
QVector<int> configuredWave(const GameCatalog::Wave& rule,int number,QRandomGenerator& random) {
    const auto& level=GameCatalog::level(number);
    const int maxType=rule.maxEnemyType<0 ? level.maxEnemyType : qMin(rule.maxEnemyType,level.maxEnemyType);
    QVector<int> enemies;
    int remaining=rule.weight;
    // Guaranteed units consume the same budget as random picks.
    for(int type=1;type<=maxType;++type) {
        const int count=qBound(0,type==1 ? rule.minGuitars : rule.minDashes,
                               remaining/GameCatalog::enemies()[type].weight);
        for(int i=0;i<count;++i) enemies.append(type);
        remaining-=count*GameCatalog::enemies()[type].weight;
    }
    fillBudget(enemies,remaining,maxType,number,random);
    shuffle(enemies,random);
    return enemies;
}
}

double WavePlanner::enemyLikelihood(int type,int level) {
    if(type==1 && level>0) return GameCatalog::level(level).guitarLikelihood;
    if(type==2 && level>0) return GameCatalog::level(level).dashLikelihood;
    static const double values[]={1.0,.20,.08};
    return type>=0 && type<3 ? values[type] : 0;
}
WavePlanner::Plan WavePlanner::create(int number, QRandomGenerator& random) {
    const auto& level=GameCatalog::level(number);
    Plan plan;
    const bool gradualOpening=number>=2 && number<=8;
    for(int wave=0;wave<level.waves;++wave) {
        if(wave<level.waveRules.size() && level.waveRules[wave].weight>0) {
            plan.append(configuredWave(level.waveRules[wave],number,random));
            continue;
        }
        QVector<int> enemies;
        if(gradualOpening && wave<2) {
            enemies.fill(0,wave==0 ? 1 : qMax(1,level.waveWeight-1));
            plan.append(enemies);
            continue;
        }
        int remaining=level.waveWeight;
        const bool finalTwo=wave>=level.waves-2;
        const int configuredWeight=wave==level.waves-1 ? level.late.finalWeight : level.late.penultimateWeight;
        const bool configuredLate=finalTwo && configuredWeight>0;
        int guitars=0,minimumCount=0;
        if(configuredLate) {
            remaining=configuredWeight;
            const int guitarWeight=GameCatalog::enemies()[1].weight;
            guitars=level.maxEnemyType>=1 ? qBound(0,level.late.minGuitars,remaining/guitarWeight) : 0;
            int largestEarlier=0;
            for(const auto& earlier : plan) largestEarlier=qMax(largestEarlier,int(earlier.size()));
            // Reserve enough ordinary dogs for increasing wave counts. This
            // applies to every expensive type, including level 7's dash dogs.
            minimumCount=largestEarlier+1;
            enemies.fill(1,guitars);
            remaining-=guitars*guitarWeight;
        }
        fillBudget(enemies,remaining,level.maxEnemyType,number,random,minimumCount);
        if(gradualOpening && finalTwo) {
            // The default plan pads with ordinary dogs. Explicit late budgets
            // already reserve enough enemies and must keep their exact cost.
            const int count=(number==2 ? level.waveWeight : level.waveWeight+1)
                +(wave==level.waves-1 ? 1 : 0);
            if(!configuredLate) while(enemies.size()<count) enemies.append(0);
            shuffle(enemies,random);
        }
        plan.append(enemies);
    }
    return plan;
}
QPair<int,int> WavePlanner::intervalBeforeWave(int number,int wave) {
    const auto& level=GameCatalog::level(number);
    if(wave<=1) return {level.initialDelayMs,level.initialDelayMs};
    if(wave<=level.waveRules.size()) {
        const auto& rule=level.waveRules[wave-1];
        if(rule.minIntervalMs>0) return {rule.minIntervalMs,qMax(rule.minIntervalMs,rule.maxIntervalMs)};
    }
    if(wave==level.waves && level.late.minIntervalMs>0)
        return {level.late.minIntervalMs,qMax(level.late.minIntervalMs,level.late.maxIntervalMs)};
    return {level.minInterval,qMax(level.minInterval,level.maxInterval)};
}
QVector<int> WavePlanner::previewTypes(int number,QRandomGenerator& random) {
    const int maxType=GameCatalog::level(number).maxEnemyType;
    const int count=maxType==0 ? 5 : 12;
    double total=0; for(int type=0;type<=maxType;++type) total+=enemyLikelihood(type,number);
    QVector<int> counts(3,0); QVector<double> remainder(3,0);
    int assigned=0;
    for(int type=0;type<=maxType;++type) {
        const double expected=count*enemyLikelihood(type,number)/total;
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
bool WavePlanner::isEndlessBigWave(int wave) {
    return wave>0 && wave%GameCatalog::EndlessWavesPerCycle==0;
}
int WavePlanner::endlessWaveWeight(int wave) {
    wave=qMax(1,wave);
    const int cycle=(wave-1)/GameCatalog::EndlessWavesPerCycle;
    if(isEndlessBigWave(wave)) return (cycle+1)*GameCatalog::EndlessBigWaveWeightStep;
    static const int offsets[]={0,0,2,3};
    return 1+cycle*GameCatalog::EndlessSmallWaveWeightStep
        +offsets[(wave-1)%GameCatalog::EndlessWavesPerCycle];
}
double WavePlanner::endlessEnemyLikelihood(int type,int wave) {
    if(type==0) return 4.0;
    // Both advanced types start at wave five and settle at the 4:3:3 ratio.
    const int progress=qBound(0,wave-4,GameCatalog::EndlessDifficultyCapWave-4);
    if(type==1 || type==2) return 3.0*progress/(GameCatalog::EndlessDifficultyCapWave-4);
    return 0;
}
QVector<int> WavePlanner::endlessWave(int wave,QRandomGenerator& random) {
    wave=qMax(1,wave);
    int remaining=endlessWaveWeight(wave);
    QVector<int> result;
    // Preserve wave ten's guaranteed guitarist within its total weight.
    if(wave==10) { result.append(1); remaining-=GameCatalog::enemies()[1].weight; }
    while(remaining>0) {
        double total=0;
        for(int type=0;type<3;++type)
            if(GameCatalog::enemies()[type].weight<=remaining) total+=endlessEnemyLikelihood(type,wave);
        double pick=random.generateDouble()*total;
        int selected=0;
        for(int type=0;type<3;++type) {
            if(GameCatalog::enemies()[type].weight>remaining) continue;
            pick-=endlessEnemyLikelihood(type,wave);
            if(pick<0) { selected=type; break; }
        }
        result.append(selected);
        remaining-=GameCatalog::enemies()[selected].weight;
    }
    shuffle(result,random);
    return result;
}

int WavePlanner::endlessIntervalAfterWave(int wave) {
    const int progress=qBound(0,wave-5,GameCatalog::EndlessIntervalCapWave-5);
    const int gap=GameCatalog::EndlessOpeningGapMs
        -(GameCatalog::EndlessOpeningGapMs-GameCatalog::EndlessSettledGapMs)
            *progress/(GameCatalog::EndlessIntervalCapWave-5);
    return gap*(isEndlessBigWave(wave) ? 2 : 1);
}

QVector<int> WavePlanner::endlessSpawnDelays(int wave,int count,QRandomGenerator& random) {
    if(count<=0) return {};
    QVector<int> delays(count,0);
    if(count==1) return delays;
    const bool big=isEndlessBigWave(wave);
    const int minimum=big ? GameCatalog::EndlessBigSpawnMinMs : GameCatalog::EndlessSmallSpawnMinMs;
    const int maximum=big ? GameCatalog::EndlessBigSpawnMaxMs : GameCatalog::EndlessSmallSpawnMaxMs;
    const int duration=qMax(count-1,random.bounded(minimum,maximum+1));
    const int gaps=count-1;
    int previous=0;
    for(int i=1;i<count;++i) {
        // Each slot is separate, so random jitter cannot reorder or merge spawns.
        const int lower=int((qint64(2)*i-1)*duration/(qint64(2)*gaps))+1;
        const int upper=int(qint64(i)*duration/gaps);
        const int time=i==gaps ? duration : random.bounded(lower,upper+1);
        delays[i]=time-previous;
        previous=time;
    }
    return delays;
}
