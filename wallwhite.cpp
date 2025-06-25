#include "wallwhite.h"
#include "whitedogs.h"

WallWhite::WallWhite():WhiteDogs(":/white/Image/wallWhite.gif", 0.5)
{
    hp = 3000;
    heartCost = 50;
}
