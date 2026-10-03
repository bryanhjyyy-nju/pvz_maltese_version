#include "gamecatalog.h"
#include <QtGlobal>

namespace GameCatalog {
const QVector<Plant>& plants() {
    static const QVector<Plant> values = {
        {"singingWhite", "歌唱小白", ":/white/Image/singingWhite.gif", "同排前方有敌人时，每 1.6 秒发射音符，造成 30 点伤害。", 500,100,7500,1600,110,-13,-3},
        {"heartWhite", "爱心小白", ":/white/Image/heartWhite.gif", "每 12 秒产出一颗爱心；每颗可收集 25 爱心。", 500,50,5000,12000,85,0,5},
        {"wallWhite", "坚盾小白", ":/white/Image/wallWhite.gif", "以高生命值阻挡敌人，保护后排。", 3000,50,17500,0,80,0,5},
        {"lineWhite", "冲锋小白", ":/white/Image/lineWhite.gif", "种下后沿本排前进（100 像素/秒），碰撞造成 2000 伤害；离开草坪后消失。", 10000,0,45000,0,80,0,10},
        {"dancingWhite", "跳舞小白", ":/white/Image/dancingWhite.gif", "受到近战攻击时，将该次伤害的 50% 反弹给攻击者。", 800,75,20000,0,85,5,0},
        {"allHeartWhite", "满心小白", ":/white/Image/allHeartWhite.gif", "每 6 秒产出一颗爱心；每颗可收集 25 爱心。", 500,125,12500,6000,90,-5,0},
        {"dblSingWhite", "双唱小白", ":/white/Image/dblSingWhite.gif", "同排前方有敌人时，每 0.8 秒发射音符，造成 30 点伤害。", 500,200,15000,800,80,0,0},
        {"moneyWhite", "招财小白", ":/white/Image/moneyWhite.gif", "受到近战攻击时将敌人向后推 100 像素。", 500,75,10000,0,90,0,0}
    };
    return values;
}
const QVector<Enemy>& enemies() {
    static const QVector<Enemy> values = {
        {"叉子金毛", ":/yellow/Image/forkYellow.gif", "基础敌人，沿本排前进并攻击接触到的小白。",300,50,25,8,0.55},
        {"吉他金毛", ":/yellow/Image/guitarYellow.gif", "第 4 关起出现。每 4 秒向本排前方 550 像素内的小白发射休止符，造成 15 伤害（近战的 1/4）；贴身时只啃食。",480,60,15,8,0.4},
        {"冲刺金毛", ":/yellow/Image/dashYellow.gif", "移动快、近战伤害高；从第 7 关开始出现。",400,80,40,10,0.5}
    };
    return values;
}
const Level& level(int number) {
    static const Level values[] = {
        {3,2,2,18000,22000,0,0,1,0,0},
        {6,1,3,16000,20000,0,0,1,0,0},
        {10,1,3,14000,18000,0,0,1,.10,0},
        {17,0,4,4000,16000,.15,0,1,0,0},
        {20,0,4,5000,14000,.15,0,1,0,0},
        {25,0,4,7000,13000,.25,0,1,0,0},
        {30,0,4,13000,18000,.35,.05,2,.05,0},
        {35,0,4,5000,15000,.75,0,1,.15,0},
        {40,0,4,10000,16000,.60,.30,1,.40,.05},
        {50,0,4,7500,23500,.65,.35,2,.30,.15}
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
    return QString("%1\n生命：%2  |  攻击：%3 / 0.5 秒\n移速：%4–%5 像素/秒\n%6")
        .arg(e.name).arg(e.health).arg(e.attack).arg(e.minSpeed).arg(e.minSpeed+e.speedRange-1).arg(e.description);
}
}
