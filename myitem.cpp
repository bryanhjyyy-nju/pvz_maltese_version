#include "myitem.h"
#include <QDebug>
#include <QGraphicsSimpleTextItem>
#include <QBitmap>
#include <QRegion>
#include <QCache>

namespace {
QPixmap scaledMovieFrame(QMovie *movie,qreal scale) {
    // Every actor keeps its own movie clock, while identical scaled frames share pixels.
    static QCache<QString,QPixmap> frames(32*1024); // 32 MiB, charged in KiB.
    const QPixmap original=movie->currentPixmap();
    if(original.isNull()) return {};
    const QSize size=original.size()*scale;
    const QString key=movie->fileName()+QString(":%1:%2x%3").arg(movie->currentFrameNumber()).arg(size.width()).arg(size.height());
    if(auto *cached=frames.object(key)) return *cached;
    const QPixmap scaled=original.scaled(size,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    frames.insert(key,new QPixmap(scaled),qMax(1,(scaled.width()*scaled.height()*4+1023)/1024));
    return scaled;
}
QRect visibleSpriteBounds(const QPixmap& sprite) {
    static QCache<qint64,QRect> bounds(1024);
    const qint64 key=sprite.cacheKey();
    if(auto *cached=bounds.object(key)) return *cached;
    QRect visible=sprite.hasAlphaChannel() ? QRegion(sprite.mask()).boundingRect() : sprite.rect();
    if(visible.isEmpty()) visible=sprite.rect();
    bounds.insert(key,new QRect(visible));
    return visible;
}
class HealthLabel : public QGraphicsSimpleTextItem {
public:
    explicit HealthLabel(QGraphicsItem *parent) : QGraphicsSimpleTextItem(parent) {
        setFont(QFont("Microsoft YaHei",11,QFont::Bold));
        setBrush(QColor("#443326"));
        setAcceptedMouseButtons(Qt::NoButton);
        setZValue(100);
    }
    QRectF boundingRect() const override {
        return QGraphicsSimpleTextItem::boundingRect().adjusted(-6,-3,6,3);
    }
    QPainterPath shape() const override { return {}; }
    void paint(QPainter *p,const QStyleOptionGraphicsItem *option,QWidget *widget) override {
        p->save(); p->setRenderHint(QPainter::Antialiasing);
        p->setPen(QPen(QColor("#84643a"),1.5)); p->setBrush(QColor("#fff4cb"));
        p->drawRoundedRect(boundingRect(),7,7);
        p->restore();
        QGraphicsSimpleTextItem::paint(p,option,widget);
    }
};
}

MyItem::MyItem(){}

MyItem::~MyItem(){
    if(movie) {
        movie->stop();
        delete movie;
    }
}

QRectF MyItem::boundingRect() const
{
    return movie ? pixmap().rect() : QRectF(0,0,100,100);
}

void MyItem::setupGifAnimation(const QString& gifPath, qreal scale){
    movie = new QMovie(this);
    movie->setFileName(gifPath);
    movie->setCacheMode(QMovie::CacheAll);

    connect(movie, &QMovie::frameChanged,this, [this,scale](int frameNumber){
        if(frameNumber<0) return;
        const QPixmap frame=scaledMovieFrame(movie,scale);
        if(frame.isNull()) return;
        setPixmap(frame);
        updateHealthLabel();
    });

    movie->start();
}

void MyItem::applyDamage(int damage) {
    if(damage<=0 || hp<=0) return;
    hp=qMax(0,hp-damage);
    updateHealthLabel();
    emit healthChanged(hp);
}
void MyItem::setHealthVisible(bool visible) {
    if(visible && !healthLabel) healthLabel=new HealthLabel(this);
    if(healthLabel) healthLabel->setVisible(visible && hp>0);
    updateHealthLabel();
}
bool MyItem::isHealthVisible() const { return healthLabel && healthLabel->isVisible(); }
QString MyItem::healthText() const { return healthLabel ? healthLabel->text() : QString(); }
void MyItem::updateHealthLabel() {
    if(!healthLabel) return;
    healthLabel->setText(QString::number(qMax(0,hp)));
    if(hp<=0) healthLabel->hide();
    if(!healthLabel->isVisible()) return;
    // GIF canvases contain transparent padding; anchor above the visible sprite.
    const QRect visible=visibleSpriteBounds(pixmap());
    const qreal y=qMax(visible.top()-healthLabel->boundingRect().height()-6,132.0-pos().y());
    healthLabel->setPos(visible.center().x()-healthLabel->boundingRect().center().x(),y);
}

QPainterPath MyItem::shape() const {
    QPainterPath path;
    path.addEllipse(boundingRect().center(), 40, 60);
    return path;
}

void MyItem::setItPos(int r, int c){
    itRow = r;
    itCol = c;
}

void MyItem::startMoving(){
    if (movingAnim && !memIsMoving){
        if(movingAnim->state() == movingAnim->Stopped){
            memIsMoving = true;
            movingAnim->start();
        }
        else if(movingAnim->state() == movingAnim->Paused){
            memIsMoving = true;
            movingAnim->setPaused(false);
        }
    }
}

void MyItem::stopMoving(){
    if(memIsMoving){
        if(movingAnim){
            movingAnim->setPaused(true);
            memIsMoving = false;
        }
    }
}

// void MyItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
// {
//     painter->setPen(Qt::red);
//     painter->drawPath(shape());
// }

void MyItem::gamePaused(){
    if(movie){
        movie->setPaused(true);
    }
}

void MyItem::gameContinued(){
    if(movie){
        movie->setPaused(false);
    }
}
