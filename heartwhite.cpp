#include "heartwhite.h"

HeartWhite::HeartWhite():WhiteDogs(":/white/Image/heartWhite.gif")
{
    hp = 300;
    heartCost = 100;
    myScale = 1.3;
    setScale(myScale);
}
