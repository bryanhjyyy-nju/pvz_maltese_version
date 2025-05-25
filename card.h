#ifndef CARD_H
#define CARD_H

#include <QPushButton>

class Card : public QPushButton
{
    Q_OBJECT
public:
    QString whiteType;  // 卡牌对应的植物类型
    int coolTime;       // 冷却时间（毫秒）
    int heartCost;      // 阳光消耗
    const int cardIndex;    // 记录第几章卡牌
    //实现构造函数
    Card(int cardNum);


signals:
};

#endif // CARD_H
