#include "gameui.h"
#include <QPushButton>
#include <QWidget>
#include <QVariant>
QString GameUi::styleSheet() {
    return QStringLiteral(R"(
        QWidget { font-family:'Microsoft YaHei'; font-size:16px; color:#563a26; }
        QDialog { background:#f7e7bb; }
        QFrame[panel="true"] { border-image:url(:/ui/Image/ui/panel_brown.png) 14 14 14 14 stretch stretch; border:14px solid transparent; }
        QPushButton[cartoon="true"] { background:#a9cf69; border:3px solid #65452d; border-bottom:6px solid #65452d; border-radius:14px; padding:5px 12px; font-weight:bold; }
        QPushButton[cartoon="true"]:hover { background:#c5e38c; }
        QPushButton[cartoon="true"]:pressed { border-bottom:3px solid #65452d; padding-top:8px; }
        QPushButton[cartoon="true"][color="gold"] { background:#f2c66c; }
        QPushButton[cartoon="true"][color="red"] { background:#efa18a; }
        QPushButton[cartoon="true"]:disabled { background:#c5bd9f; color:#81745e; }
        QTabWidget::pane { border:3px solid #9b7444; border-radius:12px; background:#fff2d4; }
        QTabBar::tab { background:#e4c18b; padding:12px 28px; border:2px solid #9b7444; border-radius:8px; font-weight:bold; }
        QTabBar::tab:selected { background:#b8d879; }
        QSlider::groove:horizontal { height:12px; border-radius:6px; background:#d9bc86; }
        QSlider::sub-page:horizontal { background:#91ba58; border-radius:6px; }
        QSlider::handle:horizontal { background:#f6ce6c; border:3px solid #65452d; width:22px; margin:-7px 0; border-radius:10px; }
    )");
}
void GameUi::styleButton(QPushButton *button, const QString& color) {
    button->setProperty("cartoon",true);
    button->setProperty("color",color);
    button->setFocusPolicy(Qt::NoFocus);
    button->setStyleSheet(styleSheet());
}
void GameUi::apply(QWidget *widget) { widget->setStyleSheet(styleSheet()); }
