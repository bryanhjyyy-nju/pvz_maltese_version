#include "levelopening.h"
#include "mygamescene.h"
#include "battlebanner.h"
#include "lawn.h"
#include "waveplanner.h"
#include "gamecatalog.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include "spriteanimation.h"
#include <QRandomGenerator>
#include <QPainter>
#include <QBitmap>
#include <QRegion>

LevelOpening::LevelOpening(int number,MyGameScene *board,BattleBanner *text,QObject *parent)
    : QObject(parent),level(number),scene(board),banner(text),timeline(this) {
    setObjectName("levelOpening"); timeline.setObjectName("openingTimeline");
    timeline.setStartValue(0.0); timeline.setEndValue(1.0);
    connect(&timeline,&QVariantAnimation::valueChanged,this,[this](const QVariant& value) {
        const qreal t=value.toReal();
        if(current==Stage::PanRight) emit cameraMoved(450*t);
        else if(current==Stage::PanLeft) emit cameraMoved(450*(1-t));
        else if(current==Stage::Reveal) scene->lawn()->setRevealProgress(t);
    });
    connect(&timeline,&QVariantAnimation::finished,this,[this] {
        if(current==Stage::PanRight) enter(Stage::Preview);
        else if(current==Stage::Preview) enter(Stage::PanLeft);
        else if(current==Stage::PanLeft) { clearPreview(); enter(Stage::Reveal); }
        else if(current==Stage::Reveal) enter(Stage::Ready);
    });
    connect(banner,&BattleBanner::finished,this,[this] {
        if(current==Stage::Ready) { current=Stage::Complete; emit finished(); }
    });
}
void LevelOpening::start() {
    scene->lawn()->setRevealProgress(0); buildPreview(); enter(Stage::PanRight);
}
void LevelOpening::enter(Stage next) {
    current=next;
    if(next==Stage::Ready) { banner->announce("准备迎接小金毛的进击！","readyImpact"); return; }
    timeline.setDuration(next==Stage::Preview ? 2000 : next==Stage::Reveal ? (level<=3 ? 1700 : 200) : 1000);
    timeline.setEasingCurve(next==Stage::PanRight || next==Stage::PanLeft ? QEasingCurve::InOutCubic : QEasingCurve::Linear);
    timeline.start();
}
void LevelOpening::buildPreview() {
    const auto types=WavePlanner::previewTypes(level,*QRandomGenerator::global());
    for(int i=0;i<types.size();++i) {
        const auto& enemy=GameCatalog::enemies()[types[i]];
        const qreal scale=enemy.scale;
        auto *movie=new SpriteAnimation(enemy.image,scale,this);
        const QPixmap firstFrame=movie->currentPixmap();
        auto *image=scene->addPixmap(firstFrame); image->setZValue(3);
        image->setAcceptedMouseButtons(Qt::NoButton);
        image->setData(0,QString("enemyPreview")); image->setData(1,types[i]);
        // Align visible feet on the road; GIF canvases have different padding.
        QRect visible=firstFrame.hasAlphaChannel() ? QRegion(firstFrame.mask()).boundingRect() : firstFrame.rect();
        if(visible.isEmpty()) visible=firstFrame.rect();
        const qreal ground=360+(i/3)*145+QRandomGenerator::global()->bounded(-14,15);
        image->setPos(1595+(i%3)*150+QRandomGenerator::global()->bounded(-10,11),ground-visible.bottom());
        connect(movie,&SpriteAnimation::frameChanged,this,[movie,image] {
            image->setPixmap(movie->currentPixmap());
        });
        previewItems.append(image); previewMovies.append(movie); movie->start();
    }
}
void LevelOpening::clearPreview() {
    for(auto *movie : previewMovies) { movie->stop(); delete movie; }
    previewMovies.clear();
    for(auto *item : previewItems) { scene->removeItem(item); delete item; }
    previewItems.clear();
}
void LevelOpening::stop() { current=Stage::Stopped; timeline.stop(); clearPreview(); banner->stop(); }
