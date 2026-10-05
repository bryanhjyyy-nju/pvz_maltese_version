#pragma once
#include <QPixmap>
#include <QRectF>
class QPainter;
namespace GameArtwork {
void drawTitle(QPainter& painter, const QRectF& area, const QString& text);
QPixmap cuteShovel();
QPixmap cuteGlove();
QPixmap cuteHeart();
QPixmap shovelSlot();
QPixmap gloveSlot();
QPixmap speakerIcon();
inline QRectF shovelSlotRect() { return QRectF(1200,0,104,110); }
inline QPointF shovelHome() { return QPointF(1210,9); }
inline QRectF gloveSlotRect() { return QRectF(1314,0,104,110); }
inline QPointF gloveHome() { return QPointF(1324,9); }
}
