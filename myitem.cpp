#include "myitem.h"


MyItem::MyItem(){}

QRectF MyItem::boundingRect() const
{
    return QRectF(0,0,100,100);
}
