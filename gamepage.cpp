#include "gamepage.h"
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
GamePage::GamePage(QWidget *parent) : QWidget(parent) {
    resize(logicalSize());
    qApp->installEventFilter(this);
}
qreal GamePage::canvasScale() const {
    return qMin(width()/1650.0,height()/900.0);
}
QPointF GamePage::canvasOffset() const {
    const auto size=QSizeF(logicalSize())*canvasScale();
    return QPointF((width()-size.width())/2,(height()-size.height())/2);
}
QPoint GamePage::canvasPoint(const QPointF& logical) const {
    return (canvasOffset()+logical*canvasScale()).toPoint();
}
void GamePage::initializePage(const QRect& buttonRect) {
    screenButton=new QPushButton("全屏 [F11]",this);
    screenButton->setObjectName("fullScreenButton");
    GameUi::styleButton(screenButton,"gold");
    screenButton->setGeometry(buttonRect);
    connect(screenButton,&QPushButton::clicked,this,&GamePage::fullScreenRequested);
    for(auto *widget : findChildren<QWidget*>(QString(),Qt::FindDirectChildrenOnly)) {
        if(widget->isWindow() || widget->property("manualScale").toBool()) continue;
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
void GamePage::prepareCanvasPaint(QPainter& painter) const {
    painter.fillRect(rect(),QColor("#20291c"));
    painter.translate(canvasOffset());
    painter.scale(canvasScale(),canvasScale());
}
void GamePage::setDisplayState(bool fullScreen,bool maximized) {
    screenButton->setText(fullScreen ? "退出全屏 [F11]" : maximized ? "还原窗口 [F11]" : "全屏 [F11]");
}
void GamePage::layoutOverlays() {
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
    for(auto *dialog : findChildren<QDialog*>())
        if(dialog->isVisible()) dialog->move(mapToGlobal(rect().center())-dialog->rect().center());
    emit canvasResized();
    update();
}
void GamePage::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    layoutOverlays();
}
void GamePage::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    layoutOverlays();
}
bool GamePage::eventFilter(QObject *watched,QEvent *event) {
    if(event->type()!=QEvent::KeyPress || !initialized || window()!=this) return QWidget::eventFilter(watched,event);
    auto *key=static_cast<QKeyEvent*>(event);
    auto *widget=qobject_cast<QWidget*>(watched);
    while(widget && widget!=this) widget=widget->parentWidget();
    if(!widget) return false;
    if(key->key()==Qt::Key_Escape) return true;
    if(key->key()==Qt::Key_F11) {
        if(!key->isAutoRepeat()) emit fullScreenRequested();
        return true;
    }
    return handleGameKey(key);
}
bool GamePage::handleGameKey(QKeyEvent *) { return false; }
