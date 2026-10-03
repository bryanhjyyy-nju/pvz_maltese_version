#pragma once
#include <QWidget>
#include <QVariantAnimation>

class BattleBanner : public QWidget {
    Q_OBJECT
public:
    explicit BattleBanner(QWidget *parent);
    void announce(const QString& text,const QString& sound);
    void stop();
signals:
    void finished();
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QString message;
    QVariantAnimation animation;
    qreal progress=0;
};
