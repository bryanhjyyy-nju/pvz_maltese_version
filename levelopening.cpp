#include "levelopening.h"
#include "mygamescene.h"
#include "battlebanner.h"
#include "lawn.h"
#include "waveplanner.h"
#include "gamecatalog.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QMovie>
#include <QRandomGenerator>
#include <QPainter>

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
    auto *road=scene->addRect(QRectF(1570,185,490,650),QPen(QColor("#8e704d"),6),QBrush(QColor("#ddd1b4")));
    road->setZValue(2); previewItems.append(road);
    QStringList counts;
    for(int type=0;type<=GameCatalog::level(level).maxEnemyType;++type)
        counts.append(QString("%1 × %2").arg(GameCatalog::enemies()[type].name).arg(types.count(type)));
    auto *label=scene->addText("本关的小金毛\n"+counts.join("  "),QFont("Microsoft YaHei",15,QFont::Bold));
    label->setTextWidth(465); label->setDefaultTextColor(QColor("#65452d")); label->setPos(1582,195); label->setZValue(3); previewItems.append(label);
    for(int i=0;i<types.size();++i) {
        auto *image=scene->addPixmap(QPixmap()); image->setZValue(3);
        image->setData(0,QString("enemyPreview")); image->setData(1,types[i]);
        image->setPos(1610+(i%3)*140,310+(i/3)*125);
        auto *movie=new QMovie(GameCatalog::enemies()[types[i]].image,QByteArray(),this);
        connect(movie,&QMovie::frameChanged,this,[movie,image] {
            image->setPixmap(movie->currentPixmap().scaled(115,115,Qt::KeepAspectRatio,Qt::SmoothTransformation));
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
