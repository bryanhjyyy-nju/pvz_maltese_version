#include "gamewindow.h"
#include "gameui.h"
#include <QApplication>
#include <QAbstractButton>
#include <QDialog>
#include <QFontInfo>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QVariant>

namespace {
bool preferredFullScreen=false;
QString scaledStyle(const QString& original,qreal scale) {
    static const QRegularExpression pixels("(\\d+(?:\\.\\d+)?)px");
    QString result;
    int offset=0;
    auto matches=pixels.globalMatch(original);
    while(matches.hasNext()) {
        const auto match=matches.next();
        result+=original.mid(offset,match.capturedStart()-offset);
        result+=QString::number(qRound(match.captured(1).toDouble()*scale))+"px";
        offset=match.capturedEnd();
    }
    return result+original.mid(offset);
}
QSize scaledSize(const QSize& size,qreal scale) {
    return QSize(qBound(0,qRound(size.width()*scale),QWIDGETSIZE_MAX),
                 qBound(0,qRound(size.height()*scale),QWIDGETSIZE_MAX));
}
}
GameWindow::GameWindow(QWidget *parent) : QMainWindow(parent) { qApp->installEventFilter(this); }
bool GameWindow::fullScreenEnabled() { return preferredFullScreen; }
qreal GameWindow::canvasScale() const {
    return qMin(width()/1650.0,height()/900.0);
}
QPointF GameWindow::canvasOffset() const {
    const auto size=QSizeF(logicalSize())*canvasScale();
    return QPointF((width()-size.width())/2,(height()-size.height())/2);
}
QPoint GameWindow::canvasPoint(const QPointF& logical) const {
    return (canvasOffset()+logical*canvasScale()).toPoint();
}
void GameWindow::initializeWindowMode(const QRect& buttonRect) {
    screenButton=new QPushButton("全屏 [F11]",this);
    screenButton->setObjectName("fullScreenButton");
    GameUi::styleButton(screenButton,"gold");
    screenButton->setGeometry(buttonRect);
    connect(screenButton,&QPushButton::clicked,this,&GameWindow::toggleFullScreen);
    for(auto *widget : findChildren<QWidget*>(QString(),Qt::FindDirectChildrenOnly)) {
        if(widget==centralWidget() || widget->isWindow() || widget->property("manualScale").toBool()) continue;
        widget->ensurePolished();
        Overlay saved;
        saved.widget=widget; saved.geometry=widget->geometry();
        saved.minimum=widget->minimumSize(); saved.maximum=widget->maximumSize();
        saved.font=widget->font(); saved.style=widget->styleSheet();
        if(auto *button=qobject_cast<QAbstractButton*>(widget)) saved.iconSize=button->iconSize();
        if(auto *label=qobject_cast<QLabel*>(widget))
            if(!label->movie()) saved.pixmap=label->pixmap(Qt::ReturnByValue);
        overlays.append(saved);
    }
    initialized=true;
    layoutOverlays();
}
void GameWindow::prepareCanvasPaint(QPainter& painter) const {
    painter.fillRect(rect(),QColor("#20291c"));
    painter.translate(canvasOffset());
    painter.scale(canvasScale(),canvasScale());
}
void GameWindow::setFullScreenEnabled(bool enabled) {
    preferredFullScreen=enabled;
    applyWindowMode(enabled);
}
void GameWindow::toggleFullScreen() { setFullScreenEnabled(!isFullScreen()); }
void GameWindow::applyWindowMode(bool enabled) {
    if(enabled==isFullScreen()) return;
    emit displayModeChanging();
    if(enabled) {
        windowedGeometry=geometry();
        setMinimumSize(0,0); setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
        QMainWindow::showFullScreen();
    } else {
        QMainWindow::showNormal();
        setFixedSize(logicalSize());
        if(windowedGeometry.isValid()) move(windowedGeometry.topLeft());
    }
    layoutOverlays();
}
void GameWindow::show() {
    applyWindowMode(preferredFullScreen);
    QMainWindow::show();
}
void GameWindow::layoutOverlays() {
    if(!initialized) return;
    const auto scale=canvasScale();
    for(const auto& saved : overlays) {
        auto *widget=saved.widget.data();
        if(!widget) continue;
        widget->setMinimumSize(0,0); widget->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
        QFont font=saved.font;
        font.setPixelSize(qMax(1,qRound(QFontInfo(saved.font).pixelSize()*scale)));
        widget->setFont(scale==1 ? saved.font : font);
        widget->setStyleSheet(scale==1 ? saved.style : scaledStyle(saved.style,scale));
        widget->setMinimumSize(scaledSize(saved.minimum,scale));
        widget->setMaximumSize(scaledSize(saved.maximum,scale));
        widget->setGeometry(QRect(canvasPoint(saved.geometry.topLeft()),scaledSize(saved.geometry.size(),scale)));
        if(auto *button=qobject_cast<QAbstractButton*>(widget)) button->setIconSize(scaledSize(saved.iconSize,scale));
        if(auto *label=qobject_cast<QLabel*>(widget))
            if(!saved.pixmap.isNull()) label->setPixmap(saved.pixmap.scaled(scaledSize(saved.pixmap.size(),scale),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    }
    screenButton->setText(isFullScreen() ? "退出全屏 [F11]" : "全屏 [F11]");
    for(auto *dialog : findChildren<QDialog*>())
        if(dialog->isVisible()) dialog->move(mapToGlobal(rect().center())-dialog->rect().center());
    emit canvasResized();
    update();
}
void GameWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    layoutOverlays();
}
void GameWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    layoutOverlays();
}
bool GameWindow::eventFilter(QObject *watched,QEvent *event) {
    if(event->type()!=QEvent::KeyPress || !initialized) return QMainWindow::eventFilter(watched,event);
    auto *key=static_cast<QKeyEvent*>(event);
    auto *widget=qobject_cast<QWidget*>(watched);
    while(widget && widget!=this) widget=widget->parentWidget();
    if(!widget) return false;
    if(key->key()==Qt::Key_F11) {
        if(!key->isAutoRepeat()) toggleFullScreen();
        return true;
    }
    return handleGameKey(key);
}
bool GameWindow::handleGameKey(QKeyEvent *) { return false; }
