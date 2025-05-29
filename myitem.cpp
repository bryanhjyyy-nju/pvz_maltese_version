#include "myitem.h"


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
            setPixmap(movie->currentPixmap().scaled(movie->currentPixmap().size() * scale,
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                    ));
            QPixmap frame = movie->currentPixmap();
            if (!frame.isNull()) {
                QPixmap scaledFrame = frame.scaled(
                    frame.size() * scale,
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                    );
                setPixmap(scaledFrame);
            }
        }
    });

    movie->start();
    setPixmap(movie->currentPixmap().scaled(movie->currentPixmap().size() * scale,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation
                                            ));
}

QPainterPath MyItem::shape() const {
    QPainterPath path;
    path.addEllipse(boundingRect().center(), 40, 65);
    return path;
}

// void MyItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
// {
//     painter->setPen(Qt::red);
//     painter->drawPath(shape());
// }
