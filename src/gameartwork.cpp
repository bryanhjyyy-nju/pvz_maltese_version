#include "gameartwork.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QtMath>
namespace GameArtwork {
QPixmap cuteHeart() {
    QPixmap result(108,100); result.fill(Qt::transparent);
    QPainter p(&result); p.setRenderHint(QPainter::Antialiasing);
    QPainterPath heart;
    heart.moveTo(54,24);
    heart.cubicTo(25,-4,3,12,7,35);
    heart.cubicTo(10,59,36,78,54,94);
    heart.cubicTo(72,78,98,59,101,35);
    heart.cubicTo(105,12,83,-4,54,24);
    QLinearGradient pink(25,12,75,94);
    pink.setColorAt(0,QColor("#ffb1cc")); pink.setColorAt(.5,QColor("#ff629c"));
    pink.setColorAt(1,QColor("#e93571"));
    p.setPen(QPen(QColor("#713449"),5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.setBrush(pink); p.drawPath(heart);
    p.setPen(QPen(QColor("#fff4fa"),5,Qt::SolidLine,Qt::RoundCap));
    QPainterPath shine; shine.moveTo(18,33); shine.cubicTo(16,23,24,16,33,19); p.drawPath(shine);
    p.setPen(Qt::NoPen); p.setBrush(QColor("#713449"));
    p.drawEllipse(QPointF(40,45),3,4); p.drawEllipse(QPointF(68,45),3,4);
    p.setBrush(QColor("#ffc2d6"));
    p.drawEllipse(QPointF(31,53),6,3); p.drawEllipse(QPointF(77,53),6,3);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor("#713449"),3,Qt::SolidLine,Qt::RoundCap));
    QPainterPath smile; smile.moveTo(47,53); smile.quadTo(54,62,61,53); p.drawPath(smile);
    return result;
}
QPixmap speakerIcon() {
    QPixmap result(64,64); result.fill(Qt::transparent);
    QPainter p(&result); p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor("#65452d"),4,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.setBrush(QColor("#fff6cc"));
    QPainterPath shape; shape.moveTo(9,25); shape.lineTo(22,25); shape.lineTo(36,13);
    shape.lineTo(36,51); shape.lineTo(22,39); shape.lineTo(9,39); shape.closeSubpath(); p.drawPath(shape);
    p.setBrush(Qt::NoBrush); p.drawArc(QRectF(32,16,20,32),-65*16,130*16);
    p.drawArc(QRectF(29,7,32,50),-65*16,130*16); return result;
}
void drawTitle(QPainter& p, const QRectF& area, const QString& text) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    const auto families=QFontDatabase().families();
#else
    const auto families=QFontDatabase::families();
#endif
    QFont font(families.contains("华文琥珀") ? "华文琥珀" : "Microsoft YaHei");
    font.setPixelSize(100); font.setWeight(QFont::Black);
    const QFontMetricsF metrics(font);
    const qreal spacing=8;
    const qreal total=metrics.horizontalAdvance(text)+(text.size()-1)*spacing;
    const qreal scale=qMin(1.0,(area.width()-40)/total);
    p.translate(area.center().x()-total*scale/2,area.center().y()+32);
    p.scale(scale,scale);
    qreal offset=0;
    const int tilt[]={-4,3,-2,4,-3,2,-3};
    for(int i=0;i<text.size();++i) {
        p.save();
        p.translate(offset,qSin(i*1.5)*7);
        p.rotate(tilt[i%7]);
        QPainterPath glyph;
        glyph.addText(0,0,font,QString(text[i]));
        p.translate(3,9);
        p.setPen(QPen(QColor("#ba8432"),18,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        p.setBrush(QColor("#ba8432")); p.drawPath(glyph);
        p.translate(-3,-9);
        p.setPen(QPen(QColor("#fff8d7"),12,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        p.setBrush(QColor("#242018")); p.drawPath(glyph);
        p.setPen(Qt::NoPen); p.drawPath(glyph);
        p.restore();
        offset+=metrics.horizontalAdvance(text[i])+spacing;
    }
    p.restore();
    p.save();
    p.setPen(QPen(QColor("#fff6b0"),5,Qt::SolidLine,Qt::RoundCap));
    for(const auto& point : {QPointF(area.left()+45,area.center().y()-20),QPointF(area.right()-45,area.center().y()+10)}) {
        p.drawLine(point-QPointF(0,12),point+QPointF(0,12));
        p.drawLine(point-QPointF(12,0),point+QPointF(12,0));
    }
    p.restore();
}
QPixmap cuteShovel() {
    QPixmap pixmap(84,84); pixmap.fill(Qt::transparent);
    QPainter p(&pixmap); p.setRenderHint(QPainter::Antialiasing);
    p.translate(42,42); p.rotate(27); p.translate(-42,-42);
    p.setPen(QPen(QColor("#6c492e"),3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.setBrush(QColor("#f4be6a"));
    p.drawRoundedRect(QRectF(38,20,8,36),4,4);
    p.setBrush(QColor("#ffd575"));
    p.drawRoundedRect(QRectF(29,3,26,23),8,8);
    p.setBrush(QColor("#fff1cb"));
    p.drawRoundedRect(QRectF(36,9,12,10),4,4);
    QPainterPath blade;
    blade.moveTo(23,43); blade.quadTo(42,38,61,43);
    blade.lineTo(59,61); blade.quadTo(55,73,42,80);
    blade.quadTo(29,73,25,61); blade.closeSubpath();
    QLinearGradient mint(25,43,58,78);
    mint.setColorAt(0,QColor("#ddfae8")); mint.setColorAt(1,QColor("#87cdb0"));
    p.setBrush(mint); p.drawPath(blade);
    p.setPen(QPen(QColor("#fffef1"),3,Qt::SolidLine,Qt::RoundCap));
    p.drawLine(28,49,27,57);
    p.setPen(Qt::NoPen); p.setBrush(QColor("#563a26"));
    p.drawEllipse(QPointF(35,55),2,3); p.drawEllipse(QPointF(49,55),2,3);
    p.setBrush(QColor("#f1a6a1"));
    p.drawEllipse(QPointF(30,60),4,2); p.drawEllipse(QPointF(54,60),4,2);
    p.setPen(QPen(QColor("#563a26"),2,Qt::SolidLine,Qt::RoundCap));
    p.drawArc(QRectF(36,57,12,9),190*16,160*16);
    return pixmap;
}
QPixmap cuteGlove() {
    QPixmap pixmap(84,84); pixmap.fill(Qt::transparent);
    QPainter p(&pixmap); p.setRenderHint(QPainter::Antialiasing);
    QPainterPath hand;
    hand.moveTo(25,67); hand.lineTo(13,46); hand.quadTo(8,37,15,35);
    hand.quadTo(20,33,27,45); hand.lineTo(24,18); hand.quadTo(23,7,31,9);
    hand.quadTo(35,10,35,18); hand.lineTo(37,35); hand.lineTo(37,10);
    hand.quadTo(38,0,46,4); hand.quadTo(49,6,48,15); hand.lineTo(48,35);
    hand.lineTo(52,15); hand.quadTo(54,5,61,11); hand.quadTo(64,14,61,23);
    hand.lineTo(57,39); hand.lineTo(63,26); hand.quadTo(68,18,73,24);
    hand.quadTo(76,28,71,38); hand.lineTo(64,57); hand.quadTo(61,65,57,69);
    hand.closeSubpath();
    QLinearGradient cotton(22,10,62,70);
    cotton.setColorAt(0,QColor("#fffdf2")); cotton.setColorAt(1,QColor("#f2d997"));
    p.setPen(QPen(QColor("#6c492e"),3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.setBrush(cotton); p.drawPath(hand);
    p.setBrush(QColor("#a7ddc5")); p.drawRoundedRect(QRectF(23,64,37,15),5,5);
    p.setPen(Qt::NoPen); p.setBrush(QColor("#563a26"));
    p.drawEllipse(QPointF(35,48),2,3); p.drawEllipse(QPointF(50,48),2,3);
    p.setBrush(QColor("#f1a6a1"));
    p.drawEllipse(QPointF(29,54),4,2); p.drawEllipse(QPointF(56,54),4,2);
    p.setPen(QPen(QColor("#563a26"),2,Qt::SolidLine,Qt::RoundCap));
    p.drawArc(QRectF(36,50,13,9),190*16,160*16);
    return pixmap;
}
static QPixmap toolSlot(const QString& shortcut) {
    QPixmap pixmap(104,110); pixmap.fill(Qt::transparent);
    QPainter p(&pixmap); p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor("#6c492e"),4)); p.setBrush(QColor("#c09655"));
    p.drawRoundedRect(QRectF(3,7,98,100),18,18);
    p.setBrush(QColor("#ffedb6")); p.drawRoundedRect(QRectF(3,3,98,99),18,18);
    p.setPen(Qt::NoPen); p.setBrush(QColor("#b7d58b"));
    p.drawRoundedRect(QRectF(74,84,23,19),6,6);
    p.setPen(QColor("#563a26")); p.setFont(QFont("Arial",10,QFont::Bold));
    p.drawText(QRectF(74,84,23,19),Qt::AlignCenter,shortcut);
    return pixmap;
}
QPixmap shovelSlot() { return toolSlot("R"); }
QPixmap gloveSlot() { return toolSlot("S"); }
}
