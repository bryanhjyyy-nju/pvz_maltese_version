#pragma once
#include <QPixmap>
#include <QRectF>
class QPainter;
namespace GameArtwork {
void drawTitle(QPainter& painter, const QRectF& area, const QString& text);
QPixmap cuteShovel();
QPixmap shovelSlot();
inline QRectF shovelSlotRect() { return QRectF(1200,0,104,110); }
inline QPointF shovelHome() { return QPointF(1210,9); }
}
