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
    return QRectF(0,0,100,100);
}

void MyItem::setupGifAnimation(const QString& gifPath){
    movie = new QMovie(this);
    movie->setFileName(gifPath);
    movie->setCacheMode(QMovie::CacheAll);

    // 连接帧更新信号
    connect(movie, &QMovie::frameChanged,this, [=](int frameNumber){
        if(frameNumber >= 0) {
            setPixmap(movie->currentPixmap());
        }
    });

    movie->start();
    setPixmap(movie->currentPixmap());
}

QPainterPath MyItem::shape() const {
    QPainterPath path;
    path.addEllipse(boundingRect().center(), 20, 30);
    return path;
}

