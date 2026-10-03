#include "gamecatalog.h"
#include "wallwhite.h"
#include "whitedogs.h"

WallWhite::WallWhite():WhiteDogs(":/white/Image/wallWhite.gif", 0.5)
{
    const auto& stats = GameCatalog::plants().at(2);
    hp = stats.health;
    heartCost = stats.cost;
}
