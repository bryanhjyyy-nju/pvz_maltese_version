#include "singingwhite.h"
#include "whitedogs.h"

SingingWhite::SingingWhite():WhiteDogs(":/white/Image/singingWhite.gif")
{
    hp = 300;
    heartCost = 100;
    myScale = 1.0;
    setScale(myScale);
}
