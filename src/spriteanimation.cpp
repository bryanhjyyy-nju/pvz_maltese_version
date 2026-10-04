#include "spriteanimation.h"
#include <QCache>
#include <QImageReader>
#include <QVector>
#include <algorithm>

struct SpriteClip {
    QVector<QPixmap> frames;
    QVector<int> frameEnds;
};

namespace {
QSharedPointer<const SpriteClip> loadClip(const QString& path,qreal scale) {
    static QCache<QString,QSharedPointer<const SpriteClip>> cache(32*1024);
    const QString key=path+QString(":%1").arg(scale,0,'g',12);
    if(auto *saved=cache.object(key)) return *saved;
    auto decoded=QSharedPointer<SpriteClip>::create();
    QImageReader reader(path);
    int elapsed=0,cost=0;
    while(reader.canRead()) {
        const QImage image=reader.read();
        if(image.isNull()) break;
        const QPixmap pixels=QPixmap::fromImage(image).scaled(image.size()*scale,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        decoded->frames.append(pixels);
        elapsed+=qMax(10,reader.nextImageDelay());
        decoded->frameEnds.append(elapsed);
        cost+=qMax(1,(pixels.width()*pixels.height()*4+1023)/1024);
    }
    QSharedPointer<const SpriteClip> result=decoded;
    cache.insert(key,new QSharedPointer<const SpriteClip>(result),qMax(1,cost));
    return result;
}
}

SpriteAnimation::SpriteAnimation(const QString& path,qreal scale,QObject *parent)
    : QAbstractAnimation(parent),clip(loadClip(path,scale)) {
    setLoopCount(-1);
}
int SpriteAnimation::duration() const { return clip->frameEnds.isEmpty() ? 1 : qMax(1,(clip->frameEnds.back()+multiplier-1)/multiplier); }
int SpriteAnimation::frameCount() const { return clip->frames.size(); }
QPixmap SpriteAnimation::currentPixmap() const { return clip->frames.isEmpty() ? QPixmap() : clip->frames[qMax(0,frame)]; }
bool SpriteAnimation::jumpToFrame(int index) {
    if(index<0 || index>=frameCount()) return false;
    setCurrentTime(index==0 ? 0 : (clip->frameEnds[index-1]+multiplier-1)/multiplier);
    return true;
}
void SpriteAnimation::updateCurrentTime(int time) {
    if(clip->frames.isEmpty()) return;
    time*=multiplier;
    const int next=qMin(int(std::upper_bound(clip->frameEnds.cbegin(),clip->frameEnds.cend(),time)-clip->frameEnds.cbegin()),frameCount()-1);
    if(next==frame) return;
    frame=next;
    emit frameChanged(frame);
}
void SpriteAnimation::setGameSpeed(GameSpeed *clock) {
    QObject::disconnect(speedConnection);
    changeSpeed(multiplier,clock ? clock->multiplier() : 1);
    if(clock) speedConnection=connect(clock,&GameSpeed::speedChanged,this,&SpriteAnimation::changeSpeed);
}
void SpriteAnimation::changeSpeed(int previous,int current) {
    if(multiplier==current) return;
    const int elapsed=currentTime();
    multiplier=current;
    setCurrentTime(qRound(double(elapsed)*previous/current));
}
