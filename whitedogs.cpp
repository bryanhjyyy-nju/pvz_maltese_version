#include "whitedogs.h"

WhiteDogs::WhiteDogs(const QString& gifPath)
{
    setTransformOriginPoint(boundingRect().center());
    // setOffset(-pixmap().width() / 2.0, -pixmap().height() / 2.0);
    setupGifAnimation(gifPath);
}
