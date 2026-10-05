#include "card.h"
#include "audiomanager.h"
#include "gamecatalog.h"
#include <QPainter>
#include <QPainterPath>
#include <QTimer>

GameState Card::cardGameState=GameState::Normal;
QString Card::cardSelectedWhite;
int Card::curRestHeart=50;

Card::Card(int number,GameSpeed *clock) : coolTime(0),heartCost(0),cardIndex(number) {
    artwork.load(":/others/Image/card.png");
    setFixedSize(artwork.size()*1.21);
    setFocusPolicy(Qt::NoFocus);
    memCoolTimer=new GameTimer(this,clock);
    memCoolTimer->setInterval(100);
    connect(memCoolTimer,&QTimer::timeout,this,[this] {
        setCoolProgress(memCoolProgress+100.0f/qMax(1,coolTime));
        if(memCoolProgress>=1) emit cooldownFinished();
    });
    connect(this,&Card::checkHeartEnough,this,&Card::refreshAvailability);
    connect(this,&Card::cooldownFinished,this,[this] {
        memCoolTimer->stop(); coolingState=false; memCoolProgress=1;
        refreshAvailability();
    });
    connect(this,&QPushButton::clicked,this,[this] {
        AudioManager::instance().play("click"); emit cardSelected(this);
    });
}
void Card::refreshAvailability() {
    memHeartIsEnough=curRestHeart>=heartCost;
    cardState=coolingState ? CardState::Cooling : memHeartIsEnough ? CardState::Normal : CardState::Unable;
    setEnabled(cardState==CardState::Normal); update();
}
void Card::cacheFaces() {
    if(cachedHeartCost==heartCost && !readyFace.isNull()) return;
    cachedHeartCost=heartCost;
    // Static art and the gray heart are built once; only the curtain animates.
    QPixmap face(artwork.size()); face.fill(Qt::transparent);
    QPainter content(&face);
    content.setRenderHint(QPainter::Antialiasing);
    content.setRenderHint(QPainter::SmoothPixmapTransform);
    content.drawPixmap(0,0,artwork);
    const auto& plant=GameCatalog::plants().at(cardIndex);
    const QPixmap unit(plant.image);
    const QRectF portrait(5,13,face.width()-10,face.height()*.55);
    QSizeF unitSize=unit.size(); unitSize.scale(portrait.size(),Qt::KeepAspectRatio);
    content.drawPixmap(QRectF(portrait.center()-QPointF(unitSize.width()/2,unitSize.height()/2),unitSize),unit,unit.rect());
    QFont font("Arial"); font.setPixelSize(11); font.setBold(true); content.setFont(font);
    content.setPen(QColor("#26372a"));
    content.drawText(QRectF(3,face.height()*.76,face.width()*.56,face.height()*.22),Qt::AlignCenter,QString::number(heartCost));
    content.end();
    readyFace=face;
    QImage pixels=face.toImage().convertToFormat(QImage::Format_ARGB32);
    for(int y=pixels.height()*2/3;y<pixels.height();++y) for(int x=pixels.width()/2;x<pixels.width();++x) {
        const QColor color=pixels.pixelColor(x,y);
        if(color.red()>color.green()*1.4 && color.red()>color.blue()*1.4)
            pixels.setPixelColor(x,y,QColor(145,145,145,color.alpha()));
    }
    unavailableFace=QPixmap::fromImage(pixels);
}
void Card::paintEvent(QPaintEvent*) {
    cacheFaces();
    QPainter painter(this); painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(rect(),coolingState || !memHeartIsEnough ? unavailableFace : readyFace);
    if(coolingState) {
        painter.fillRect(rect(),QColor(0,0,0,125));
        const qreal edge=height()*(1-memCoolProgress);
        painter.fillRect(QRectF(0,0,width(),edge),QColor(0,0,0,130));
        painter.setPen(QPen(QColor(255,237,173,180),2));
        painter.drawLine(QPointF(0,edge),QPointF(width(),edge));
    } else if(!memHeartIsEnough || selected) painter.fillRect(rect(),QColor(0,0,0,85));
}
void Card::setSelected(bool value) {
    if(selected==value) return;
    selected=value;
    update();
}
void Card::mousePressEvent(QMouseEvent *event) {
    if(cardGameState==GameState::Normal) QPushButton::mousePressEvent(event);
}
void Card::setCoolProgress(float progress) { memCoolProgress=qBound(0.0f,progress,1.0f); update(); }
void Card::startCooldown() {
    if(coolingState) return;
    coolingState=true; memCoolProgress=0; refreshAvailability(); memCoolTimer->start();
}
void Card::gamePaused() { memCoolTimer->stop(); }
void Card::gameContinued() { if(coolingState && !memCoolTimer->isActive()) memCoolTimer->start(); }
