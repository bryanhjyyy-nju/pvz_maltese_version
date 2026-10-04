#pragma once
#include <QGraphicsObject>
// A short lived, pause-aware particle burst. It never participates in combat.
class CombatEffect : public QGraphicsObject {
    Q_OBJECT
    friend class BattleSnapshot;
    Q_PROPERTY(qreal progress READ progress WRITE setProgress)
public:
    enum Kind { Bite, Hit, Plant, Uproot };
    CombatEffect(class MyGameScene *scene, const QPointF& center, Kind kind);
    QRectF boundingRect() const override { return QRectF(-65,-65,130,130); }
    QPainterPath shape() const override { return {}; }
    void paint(QPainter *,const QStyleOptionGraphicsItem *,QWidget *) override;
    qreal progress() const { return m_progress; }
    void setProgress(qreal value) { m_progress = value; update(); }
private:
    qreal m_progress = 0;
    Kind m_kind;
};
