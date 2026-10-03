#include "gamecatalog.h"
#include "dancingwhite.h"

DancingWhite::DancingWhite() : WhiteDogs(":/white/Image/dancingWhite.gif",0.45)
{
    const auto& stats = GameCatalog::plants().at(4);
    hp = stats.health;
    heartCost = stats.cost;
}
