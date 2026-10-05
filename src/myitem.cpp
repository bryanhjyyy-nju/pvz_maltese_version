#include "myitem.h"
#include <QDebug>
#include <QGraphicsSimpleTextItem>
#include <QBitmap>
#include <QRegion>
#include <QCache>
#include <QGraphicsScene>

namespace {
QRect visibleSpriteBounds(const QPixmap& sprite) {
    static QCache<qint64,QRect> bounds(1024);
    const qint64 key=sprite.cacheKey();
    if(auto *cached=bounds.object(key)) return *cached;
    QRect visible=sprite.hasAlphaChannel() ? QRegion(sprite.mask()).boundingRect() : sprite.rect();
    if(visible.isEmpty()) visible=sprite.rect();
    bounds.insert(key,new QRect(visible));
    return visible;
}
}

// Keep QObject ownership with the unit, but paint independently above sprites.
class HealthLabel : public QObject, public QGraphicsSimpleTextItem {
public:
    explicit HealthLabel(QObject *owner) : QObject(owner) {
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

MyItem::MyItem(){
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

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
    movie = new SpriteAnimation(gifPath,scale,this);
    setPixmap(movie->currentPixmap());

    connect(movie, &SpriteAnimation::frameChanged,this, [this](int frameNumber){
        if(frameNumber<0) return;
        const QPixmap frame=movie->currentPixmap();
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
    healthVisible=visible;
    if(visible && !healthLabel) healthLabel=new HealthLabel(this);
    updateHealthLabel();
}
bool MyItem::isHealthVisible() const { return healthLabel && healthLabel->isVisible(); }
QString MyItem::healthText() const { return healthLabel ? healthLabel->text() : QString(); }
void MyItem::updateHealthLabel() {
    if(!healthLabel) return;
    if(healthLabel->scene()!=scene()) {
        if(healthLabel->scene()) healthLabel->scene()->removeItem(healthLabel);
        if(scene()) scene()->addItem(healthLabel);
    }
    healthLabel->setText(QString::number(qMax(0,hp)));
    healthLabel->setZValue(100+qBound(0,itRow,4));
    healthLabel->setVisible(healthVisible && hp>0 && scene() && isVisible());
    if(!healthLabel->isVisible()) return;
    // GIF canvases contain transparent padding; anchor above the visible sprite.
    const QRect visible=visibleSpriteBounds(pixmap());
    const qreal y=qMax(visible.top()-healthLabel->boundingRect().height()-6,132.0-pos().y());
    healthLabel->setPos(mapToScene(QPointF(visible.center().x()-healthLabel->boundingRect().center().x(),y)));
}

QVariant MyItem::itemChange(GraphicsItemChange change,const QVariant& value) {
    const auto result=QGraphicsPixmapItem::itemChange(change,value);
    switch(change) {
    case ItemPositionHasChanged:
    case ItemScenePositionHasChanged:
    case ItemSceneHasChanged:
    case ItemTransformHasChanged:
    case ItemRotationHasChanged:
    case ItemScaleHasChanged:
    case ItemTransformOriginPointHasChanged:
    case ItemVisibleHasChanged:
        updateHealthLabel();
        break;
    default: break;
    }
    return result;
}

QPainterPath MyItem::shape() const {
    QPainterPath path;
    path.addEllipse(boundingRect().center(), 40, 60);
    return path;
}

void MyItem::setItPos(int r, int c){
    itRow = r;
    itCol = c;
    updateHealthLabel();
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
