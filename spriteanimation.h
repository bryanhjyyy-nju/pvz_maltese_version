#pragma once

#include <QAbstractAnimation>
#include <QPixmap>
#include <QSharedPointer>

struct SpriteClip;

// Decoded pixels are shared. Each actor keeps its own progress and pause state.
class SpriteAnimation : public QAbstractAnimation {
    Q_OBJECT
public:
    SpriteAnimation(const QString& path,qreal scale,QObject *parent=nullptr);
    int duration() const override;
    int frameCount() const;
    int currentFrameNumber() const { return frame; }
    QPixmap currentPixmap() const;
    bool jumpToFrame(int index);
signals:
    void frameChanged(int index);
protected:
    void updateCurrentTime(int time) override;
private:
    QSharedPointer<const SpriteClip> clip;
    int frame=-1;
};
