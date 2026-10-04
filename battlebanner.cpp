#include "battlebanner.h"
#include "audiomanager.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

BattleBanner::BattleBanner(QWidget *parent) : QWidget(parent),animation(this) {
    setObjectName("battleBanner"); setProperty("manualScale",true);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    animation.setObjectName("bannerAnimation");
    animation.setDuration(1800); animation.setStartValue(0.0); animation.setEndValue(1.0);
    connect(&animation,&QVariantAnimation::valueChanged,this,[this](const QVariant& value) { progress=value.toReal(); update(); });
    connect(&animation,&QVariantAnimation::finished,this,[this] { hide(); emit finished(); });
    hide();
}
void BattleBanner::announce(const QString& text,const QString& sound,bool withFlash) {
    animation.stop(); message=text; progress=0; flash=withFlash;
    setAccessibleName(text);
    setGeometry(parentWidget()->rect()); show(); raise();
    AudioManager::instance().play(sound); animation.start();
}
void BattleBanner::stop() { animation.stop(); hide(); }
void BattleBanner::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    const qreal scale=qMin(width()/1650.0,height()/900.0);
    p.translate(rect().center()); p.scale(scale,scale);
    const qreal alpha=progress>.78 ? (1-progress)/.22 : 1;
    p.setOpacity(alpha);
    if(flash) {
        QRadialGradient glow(0,0,680); glow.setColorAt(0,QColor(255,229,128,190)); glow.setColorAt(1,Qt::transparent);
        p.fillRect(QRectF(-825,-450,1650,900),glow);
    }
    const qreal impact=progress<.2 ? 1+.35*qPow(1-progress/.2,2) : 1;
    p.translate(progress<.2 ? qSin(progress*180)*6 : 0,0); p.scale(impact,impact);
    QFont font("华文琥珀"); font.setPixelSize(68); font.setWeight(QFont::Black);
    QPainterPath letters; letters.addText(0,0,font,message);
    const QRectF bounds=letters.boundingRect();
    const qreal fit=qMin(1.0,1420.0/(bounds.width()+20)); p.scale(fit,fit);
    p.translate(-bounds.center());
    p.setPen(QPen(QColor("#100d0a"),10,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.setBrush(QColor("#f33328")); p.drawPath(letters);
    p.setPen(Qt::NoPen); p.drawPath(letters);
}
