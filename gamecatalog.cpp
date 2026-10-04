#include "gamecatalog.h"
#include <QtGlobal>

namespace GameCatalog {
const QVector<Plant>& plants() {
    static const QVector<Plant> values = {
        {"singingWhite", "歌唱小白", ":/white/Image/singingWhite.gif", "同排前方有敌人时，每 1.6 秒发射音符，造成 30 点伤害。", 500,100,7500,SingingShotIntervalMs,110,-13,-3},
        {"heartWhite", "爱心小白", ":/white/Image/heartWhite.gif", "每 12 秒产出一颗爱心；每颗可收集 25 爱心。", 500,50,5000,HeartProductionIntervalMs,85,0,5},
        {"wallWhite", "坚盾小白", ":/white/Image/wallWhite.gif", "以高生命值阻挡敌人，保护后排。", 4000,50,17500,0,80,0,5},
        {"lineWhite", "冲锋小白", ":/white/Image/lineWhite.gif", "种下后沿本排前进（100 像素/秒），开始移动后释放原草格；碰撞造成 2000 伤害，离开草坪后消失。", 10000,125,30000,0,80,0,10},
        {"dancingWhite", "跳舞小白", ":/white/Image/dancingWhite.gif", "受到近战攻击时，将该次伤害的 50% 反弹给攻击者。", 800,75,20000,0,85,5,0},
        {"allHeartWhite", "满心小白", ":/white/Image/allHeartWhite.gif", "每 12 秒同时产出两颗爱心；每颗可收集 25 爱心。", 500,125,12500,HeartProductionIntervalMs,90,-5,0},
        {"dblSingWhite", "双唱小白", ":/white/Image/dblSingWhite.gif", "同排前方有敌人时，每 1.6 秒开始一组两发连射，两发相隔 0.18 秒，每发造成 30 点伤害。", 500,200,15000,SingingShotIntervalMs,80,0,0},
        {"moneyWhite", "招财小白", ":/white/Image/moneyWhite.gif", "受到近战攻击时将敌人向后推 100 像素。", 500,75,10000,0,90,0,0}
    };
    return values;
}
const QVector<Enemy>& enemies() {
    static const QVector<Enemy> values = {
        {"叉子金毛", ":/yellow/Image/forkYellow.gif", "基础敌人，沿本排前进并攻击接触到的小白。",300,50,25,8,0.55,1},
        {"吉他金毛", ":/yellow/Image/guitarYellow.gif", "第 4 关起出现。每 2 秒沿本排向前发射金色双音符，命中小白造成 60 伤害，与啃食伤害相同；无尽模式中两者同步增强。贴身时只啃食。",480,60,15,8,0.4,3},
        {"冲刺金毛", ":/yellow/Image/dashYellow.gif", "移动快、近战伤害高；生命低于一半时，移速提升至初始速度的 1.5 倍。从第 7 关开始出现。",450,80,40,10,0.5,5}
    };
    return values;
}
const Level& level(int number) {
    static const Level values[] = {
        // waves, weight/wave, rows, interval (ms), max type, opening time (ms), hearts
        // Optional: guitar relative likelihood, {last-two weights, guaranteed guitars, final gap (ms)}
        {3,1,2,2,18000,22000,0,15000,50},
        {4,2,1,3,22000,26000,0,18000,50},
        {5,2,0,4,20000,24000,0,18000,50},
        {5,3,0,4,23000,28000,1,20000,75,.45,{8,10,1,35000,42000}}, // Level 4
        {6,3,0,4,22000,27000,1,20000,75,.45,{9,11,1,38000,45000}}, // Level 5
        {6,4,0,4,25000,30000,1,22000,75,.45,{10,12,1,40000,48000}}, // Level 6
        {6,4,0,4,25000,30000,2,22000,100},
        {7,4,0,4,25000,30000,2,25000,100},
        {7,5,0,4,27000,32000,2,25000,100},
        {8,5,0,4,27000,32000,2,28000,100}
    };
    return values[qBound(1,number,LevelCount)-1];
}
QString plantDetails(int index) {
    const auto& p = plants().at(index);
    return QString("%1\n生命：%2  |  花费：%3 爱心\n冷却：%4 秒  |  第 %5 关可用\n%6")
        .arg(p.name).arg(p.health).arg(p.cost).arg(p.cooldownMs/1000.0).arg(index+1).arg(p.description);
}
QString enemyDetails(int index) {
    const auto& e = enemies().at(index);
    return QString("%1\n生命：%2  |  近战：%3 / 0.5 秒\n移速：%4–%5 像素/秒  |  权重：%6\n%7")
        .arg(e.name).arg(e.health).arg(e.attack).arg(e.minSpeed).arg(e.minSpeed+e.speedRange-1).arg(e.weight).arg(e.description);
}
}
