#pragma once
#include <QMainWindow>
#include <QPointer>
#include <QPixmap>
#include <QFont>
#include <QVector>
class QPainter;
class QPushButton;

// All screens use the same logical canvas. Native controls and the battlefield
// share its scale and offset, so input remains aligned in either window mode.
class GameWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit GameWindow(QWidget *parent=nullptr);
    void show();
    void setFullScreenEnabled(bool enabled);
    void toggleFullScreen();
    static bool fullScreenEnabled();
    qreal canvasScale() const;
    QPointF canvasOffset() const;
    QPoint canvasPoint(const QPointF& logical) const;
    static QSize logicalSize() { return QSize(1650,900); }
protected:
    void initializeWindowMode(const QRect& buttonRect);
    void prepareCanvasPaint(QPainter& painter) const;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched,QEvent *event) override;
    virtual bool handleGameKey(class QKeyEvent *event);
signals:
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
    };
    QVector<Overlay> overlays;
    QPushButton *screenButton=nullptr;
    QRect windowedGeometry;
    bool initialized=false;
    void applyWindowMode(bool enabled);
    void layoutOverlays();
};
