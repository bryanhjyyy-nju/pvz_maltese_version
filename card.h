#ifndef CARD_H
#define CARD_H

#include <QPushButton>
#include <QPixmap>
#include "gamestate.h"
#include "cardstate.h"
class Card : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(float coolProgress READ coolProgress WRITE setCoolProgress)

public:
    QString whiteType;  // 卡牌对应的植物类型
    int coolTime;       // 冷却时间（毫秒）
    int heartCost;      // 阳光消耗
    //实现构造函数
    Card(int cardNum);

    //返回冷却状态
    bool isCooling(){ return coolingState; }
    float coolProgress() const { return memCoolProgress; }
    void setCoolProgress(float progress); //防止编译报错

    //返回和设置爱心是否足够
    bool getHeartIsEnough(){ return memHeartIsEnough; }
    void setHeartIsEnough(bool isEnough){ memHeartIsEnough = isEnough; }

    //设置卡牌状态
    void setCardState(CardState state){ cardState = state; }
    CardState getCardState(){ return cardState; }

    // 设置/获取预放置状态
    static void setGameState(GameState state){ cardGameState = state; }
    static GameState currentState(){ return cardGameState; }

    // 设置当前选择的植物类型
    static void setSelectedWhite(const QString& sWhite){ cardSelectedWhite = sWhite; }
    static QString selectedWhite(){ return cardSelectedWhite; }

    // 设置当前剩余爱心
    static void setCurRestHeart(int r){ curRestHeart = r; }

    void startCooldown(); //开始冷却

    void gamePaused();
    void gameContinued();

private:
    QPixmap artwork;
    void refreshAvailability();
    void paintEvent(QPaintEvent *event) override;

    const int cardIndex;    // 记录第几章卡牌
    bool coolingState = false; //记录是否正在冷却
    bool memHeartIsEnough = true; //判断爱心是否足够
    float memCoolProgress = 0.0f; // 冷却进度 (0.0-1.0)
    QTimer *memCoolTimer = nullptr; //冷却计时器
    CardState cardState = CardState::Normal;

    void mousePressEvent(QMouseEvent *e) override; //重写鼠标点击事件

    static GameState cardGameState; // 共享的游戏状态
    static QString cardSelectedWhite; // 当前选择小白类型
    static int curRestHeart; //统计现在剩余的阳光
signals:
    void cardSelected(Card* card); // 卡牌被选中的信号
    void cooldownFinished(); //冷却结束的信号
    void checkHeartEnough(); //发送信号检查爱心是否足够

};

#endif // CARD_H
