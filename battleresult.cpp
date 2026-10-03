#include "battleresult.h"
#include "audiomanager.h"
#include "gamecatalog.h"
#include "gameui.h"
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QtMath>

namespace {
void cartoonText(QPainter& painter,const QString& text,qreal y,int size,const QColor& fill) {
    QFont font("华文琥珀"); font.setPixelSize(size); font.setBold(true);
    QFontMetricsF metrics(font);
    QPainterPath path; path.addText(QPointF(825-metrics.horizontalAdvance(text)/2,y),font,text);
    painter.setPen(QPen(QColor("#281919"),7,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    painter.setBrush(fill); painter.drawPath(path);
    painter.setPen(Qt::NoPen); painter.drawPath(path);
}
}
BattleResult::BattleResult(bool victory,int number,const QPointF& losingEnemy,QWidget *parent)
    : QWidget(parent),won(victory),level(number),enemy(losingEnemy),timeline(this) {
    setObjectName("battleResult"); setProperty("manualScale",true);
    timeline.setObjectName("resultAnimation"); timeline.setStartValue(0.0); timeline.setEndValue(1.0);
    timeline.setDuration(won ? 3600 : 5400);
    connect(&timeline,&QVariantAnimation::valueChanged,this,[this](const QVariant& value) {
        progress=value.toReal();
        if(!won && progress>=.48 && !sounded) { sounded=true; AudioManager::instance().play("lose"); }
        update();
    });
    connect(&timeline,&QVariantAnimation::finished,this,[this] {
        if(won) back->show(); else emit returnRequested();
    });
    AudioManager::instance().stopMusic();
    if(won) {
        AudioManager::instance().playVictory();
        back=new QPushButton("返回选关",this); back->setObjectName("resultBack");
        GameUi::styleButton(back,"sunshine"); back->hide();
        connect(back,&QPushButton::clicked,this,&BattleResult::returnRequested);
    }
    setGeometry(parent->rect()); show(); raise(); timeline.start();
}
void BattleResult::stop() { timeline.stop(); hide(); }
void BattleResult::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    const qreal scale=qMin(width()/1650.0,height()/900.0);
    const QPointF offset((width()-1650*scale)/2,(height()-900*scale)/2);
    if(back) { back->setGeometry(QRect(qRound(offset.x()+610*scale),qRound(offset.y()+805*scale),qRound(430*scale),qRound(65*scale)));
        QFont font("Microsoft YaHei"); font.setPixelSize(qMax(12,qRound(24*scale))); font.setBold(true); back->setFont(font); }
}
void BattleResult::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing); painter.setRenderHint(QPainter::SmoothPixmapTransform);
    const qreal scale=qMin(width()/1650.0,height()/900.0);
    painter.translate((width()-1650*scale)/2,(height()-900*scale)/2); painter.scale(scale,scale);
    // Include letterboxed areas in the fade as well.
    const QRectF screen(-width()/scale,-height()/scale,3*width()/scale,3*height()/scale);
    if(!won) {
        const qreal fade=qMin(1.0,progress/.36);
        const qreal radius=1900*(1-fade)+105;
        QPainterPath mask; mask.addRect(screen); mask.addEllipse(enemy,radius,radius); mask.setFillRule(Qt::OddEvenFill);
        painter.fillPath(mask,QColor(0,0,0,qRound(255*fade)));
        if(progress>.40) painter.fillRect(screen,QColor(0,0,0,qRound(255*qMin(1.0,(progress-.40)/.14))));
        if(progress>=.48) {
            painter.setOpacity(qMin(1.0,(progress-.48)/.12));
            cartoonText(painter,"小白输给了小金毛！",470,76,QColor("#f04436"));
        }
        return;
    }
    painter.fillRect(screen,QColor(22,38,29,qRound(215*qMin(1.0,progress*5))));
    const qreal reveal=qMin(1.0,progress/.60);
    painter.setOpacity(reveal);
    painter.save(); painter.translate(825,425); painter.rotate(progress*24);
    painter.setPen(Qt::NoPen);
    for(int i=0;i<18;++i) {
        painter.rotate(20);
        QLinearGradient glow(0,0,0,-610); glow.setColorAt(0,QColor(255,238,147,210)); glow.setColorAt(1,QColor(255,205,60,0));
        QPolygonF ray; ray << QPointF(0,0) << QPointF(-65,-610) << QPointF(65,-610);
        painter.setBrush(glow); painter.drawPolygon(ray);
    }
    painter.restore();
    QRadialGradient halo(QPointF(825,425),340); halo.setColorAt(0,QColor(255,249,201,230)); halo.setColorAt(1,QColor(255,217,70,0));
    painter.fillRect(QRectF(465,65,720,720),halo);
    cartoonText(painter,"草坪保卫成功！",145,64,QColor("#ffe895"));
    painter.save(); painter.translate(825,425);
    const qreal grow=.62+.38*QEasingCurve(QEasingCurve::OutBack).valueForProgress(reveal);
    painter.scale(grow,grow);
    const int reward=rewardPlant();
    if(reward>=0) {
        painter.setPen(QPen(QColor("#5d7750"),9)); painter.setBrush(QColor("#f5f5e6"));
        painter.drawRoundedRect(QRectF(-135,-200,270,380),14,14);
        painter.setPen(QPen(QColor("#a7c185"),6)); painter.setBrush(Qt::white);
        painter.drawRoundedRect(QRectF(-120,-173,240,266),8,8);
        const QPixmap unit(GameCatalog::plants()[reward].image);
        painter.drawPixmap(QRectF(-112,-145,224,224),unit,unit.rect());
        QFont cost("Arial"); cost.setPixelSize(34); cost.setBold(true); painter.setFont(cost); painter.setPen(QColor("#24362a"));
        painter.drawText(QRectF(-110,98,145,70),Qt::AlignCenter,QString::number(GameCatalog::plants()[reward].cost));
        QPainterPath heart; heart.moveTo(83,160); heart.cubicTo(10,120,45,87,83,109); heart.cubicTo(121,87,155,120,83,160);
        painter.setPen(Qt::NoPen); painter.setBrush(QColor("#f42b39")); painter.drawPath(heart);
    } else {
        painter.setPen(QPen(QColor("#9c6526"),8));
        QLinearGradient gold(-100,0,120,0); gold.setColorAt(0,QColor("#d99425")); gold.setColorAt(.5,QColor("#fff295")); gold.setColorAt(1,QColor("#e4aa35")); painter.setBrush(gold);
        painter.drawEllipse(QRectF(-180,-135,360,200));
        QPainterPath cup; cup.moveTo(-120,-165); cup.lineTo(120,-165); cup.cubicTo(130,25,55,95,0,95); cup.cubicTo(-55,95,-130,25,-120,-165);
        painter.drawPath(cup); painter.drawRoundedRect(QRectF(-22,80,44,75),10,10); painter.drawRoundedRect(QRectF(-110,145,220,35),10,10);
        painter.setPen(QColor("#a76b24")); QFont star("Arial"); star.setPixelSize(100); painter.setFont(star); painter.drawText(QRectF(-100,-140,200,170),Qt::AlignCenter,"★");
    }
    painter.restore();
    cartoonText(painter,reward>=0 ? "新伙伴："+GameCatalog::plants()[reward].name : "挑战关胜利 · 荣耀奖杯",665,45,QColor("#ffefb8"));
    painter.setPen(QColor("#fff7df")); QFont details("Microsoft YaHei"); details.setPixelSize(24); painter.setFont(details);
    painter.drawText(QRectF(355,690,940,100),Qt::AlignHCenter|Qt::TextWordWrap,
        reward>=0 ? GameCatalog::plants()[reward].description : level==10 ? "全部十关通关！主菜单已解锁无尽模式。" : "下一关已解锁，迎接更强的小金毛吧！");
}
