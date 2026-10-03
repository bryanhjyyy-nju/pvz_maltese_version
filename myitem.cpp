#include "myitem.h"
#include <QDebug>
#include <QGraphicsSimpleTextItem>
#include <QBitmap>
#include <QRegion>

namespace {
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

    // 连接帧更新信号
    connect(movie, &QMovie::frameChanged,this, [=](int frameNumber){
        if(frameNumber >= 0) {
            // setPixmap(movie->currentPixmap().scaled(movie->currentPixmap().size() * scale,
            //         Qt::KeepAspectRatio,
            //         Qt::SmoothTransformation
            //         ));
            QPixmap frame = movie->currentPixmap();
            if (!frame.isNull()) {
                QPixmap scaledFrame = frame.scaled(
                    frame.size() * scale,
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                    );
                setPixmap(scaledFrame);
                updateHealthLabel();
            }
        }
    });

    movie->start();
    setPixmap(movie->currentPixmap().scaled(movie->currentPixmap().size() * scale,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation
                                            ));
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
    QRect visible=pixmap().hasAlphaChannel() ? QRegion(pixmap().mask()).boundingRect() : pixmap().rect();
    if(visible.isEmpty()) visible=pixmap().rect();
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
