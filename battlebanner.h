#pragma once
#include <QWidget>
#include "gamespeed.h"

class BattleBanner : public QWidget {
    Q_OBJECT
public:
    explicit BattleBanner(QWidget *parent,GameSpeed *clock=nullptr);
    void announce(const QString& text,const QString& sound,bool withFlash=true);
    void stop();
signals:
    void finished();
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QString message;
    GameVariantAnimation animation;
    qreal progress=0;
    bool flash=true;
};
