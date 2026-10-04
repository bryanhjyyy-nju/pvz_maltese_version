#ifndef GAMESTATE_H
#define GAMESTATE_H

enum class GameState {
    Normal,     // 正常状态
    PrePlace,   // 预放置状态
    Shoveling,  // 激活铲子模式
    Paused,     // 游戏暂停
    GameOver    // 游戏结束
};

#endif // GAMESTATE_H
