#ifndef CARDSTATE_H
#define CARDSTATE_H

enum class CardState {
    Normal,     // 正常状态
    Cooling,    // 冷却状态
    Unable //冷却完成但是不可选择状态
};

#endif // CARDSTATE_H
