#pragma once
#include <QObject>
#include <QVariantAnimation>
#include <QVector>
class MyGameScene;
class BattleBanner;
class QGraphicsItem;
class QMovie;

// Presentation runs before battle timers start. Preview actors never enter combat.
class LevelOpening : public QObject {
    Q_OBJECT
public:
    enum class Stage { PanRight,Preview,PanLeft,Reveal,Ready,Complete,Stopped };
    Q_ENUM(Stage)
    LevelOpening(int level,MyGameScene *scene,BattleBanner *banner,QObject *parent);
    void start();
    void stop();
    Stage stage() const { return current; }
signals:
    void cameraMoved(qreal offset);
    void finished();
private:
    int level;
    MyGameScene *scene;
    BattleBanner *banner;
    QVariantAnimation timeline;
    Stage current=Stage::Stopped;
    QVector<QGraphicsItem*> previewItems;
    QVector<QMovie*> previewMovies;
    void enter(Stage stage);
    void buildPreview();
    void clearPreview();
};
