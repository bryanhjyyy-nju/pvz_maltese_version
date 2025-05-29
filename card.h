#ifndef CARD_H
#define CARD_H

#include <QPushButton>
#include "gamestate.h"

class Card : public QPushButton
{
    Q_OBJECT
public:
    QString whiteType;  // 卡牌对应的植物类型
    int coolTime;       // 冷却时间（毫秒）
    int heartCost;      // 阳光消耗
    //实现构造函数
    Card(int cardNum);

    // 设置/获取预放置状态
    static void setGameState(GameState state);
    static GameState currentState();

    // 设置当前选择的植物类型
    static void setSelectedWhite(const QString& sWhite);
    static QString selectedWhite();

private:
    const int cardIndex;    // 记录第几章卡牌
    static GameState cardGameState; // 共享的游戏状态
    static QString cardSelectedWhite; // 当前选择小白类型

    void mousePressEvent(QMouseEvent *e) override;
signals:
    void cardSelected(Card* card); // 卡牌被选中的信号

};

#endif // CARD_H
