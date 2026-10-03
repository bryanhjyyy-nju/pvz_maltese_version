#pragma once
#include <QWidget>
#include <QPointer>
#include <QPixmap>
#include <QFont>
#include <QVector>
class QPainter;
class QPushButton;

// All screens use the same logical canvas. Native controls and the battlefield
// share its scale and offset, so input remains aligned in either window mode.
class GamePage : public QWidget {
    Q_OBJECT
public:
    explicit GamePage(QWidget *parent=nullptr);
    void setDisplayState(bool fullScreen, bool maximized);
    bool processGameKey(class QKeyEvent *event) { return handleGameKey(event); }
    qreal canvasScale() const;
    QPointF canvasOffset() const;
    QPoint canvasPoint(const QPointF& logical) const;
    static QSize logicalSize() { return QSize(1650,900); }
protected:
    void initializePage(const QRect& buttonRect);
    void prepareCanvasPaint(QPainter& painter) const;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched,QEvent *event) override;
    virtual bool handleGameKey(class QKeyEvent *event);
signals:
    void fullScreenRequested();
    void canvasResized();
    void displayModeChanging();
private:
    struct Overlay {
        QPointer<QWidget> widget;
        QRect geometry;
        QSize minimum,maximum,iconSize;
        QFont font;
        QString style;
        QPixmap pixmap;
        QPixmap icon;
    };
    QVector<Overlay> overlays;
    QPushButton *screenButton=nullptr;
    bool initialized=false;
    void layoutOverlays();
};
