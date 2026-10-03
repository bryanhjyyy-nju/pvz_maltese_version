#include "gamecatalog.h"
#include "moneywhite.h"

MoneyWhite::MoneyWhite() : WhiteDogs(":/white/Image/moneyWhite.gif",0.35)
{
    const auto& stats = GameCatalog::plants().at(7);
    hp = stats.health;
    heartCost = stats.cost;
}
