#include "gameui.h"
#include <QPushButton>
#include <QWidget>
#include <QVariant>
#include <QFontDatabase>
QString GameUi::fontFamily(const QString& preferred) {
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    const auto families=QFontDatabase().families();
#else
    const auto families=QFontDatabase::families();
#endif
    for(const auto& family : {preferred,QStringLiteral("Microsoft YaHei"),
                             QStringLiteral("PingFang SC"),QStringLiteral("Heiti SC"),
                             QStringLiteral("Noto Sans CJK SC")})
        if(!family.isEmpty() && families.contains(family)) return family;
    return QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();
}
QString GameUi::styleSheet() {
    return QStringLiteral(R"(
        QWidget { font-family:'Microsoft YaHei','PingFang SC','Heiti SC'; font-size:16px; color:#563a26; }
        QDialog { background:#f7e7bb; }
        QFrame[panel="true"] { border-image:url(:/ui/Image/ui/panel_brown.png) 14 14 14 14 stretch stretch; border:14px solid transparent; }
        QPushButton[cartoon="true"] { background:#a9cf69; border:3px solid #65452d; border-bottom:6px solid #65452d; border-radius:14px; padding:5px 12px; font-weight:bold; }
        QPushButton[cartoon="true"]:hover { background:#c5e38c; }
        QPushButton[cartoon="true"]:pressed { border-bottom:3px solid #65452d; padding-top:8px; }
        QPushButton[cartoon="true"][color="gold"] { background:#f2c66c; }
        QPushButton[cartoon="true"][color="red"] { background:#efa18a; }
        QPushButton[cartoon="true"][color="sunshine"] { background:#ffeb32; color:#171711; font-family:'华文琥珀','Microsoft YaHei','PingFang SC','Heiti SC'; font-size:34px; border-radius:25px; border-width:4px; border-bottom-width:8px; padding:8px 24px; }
        QPushButton[cartoon="true"][color="sunshine"]:hover { background:#fff478; }
        QPushButton[cartoon="true"][color="sunshine"]:pressed { background:#f6ce2e; border-bottom-width:4px; padding-top:12px; }
        QPushButton[cartoon="true"][color="level"] { background:#ffe492; color:#563a26; font-size:40px; border-width:5px; border-bottom-width:9px; border-radius:30px; padding:8px; }
        QPushButton[cartoon="true"][color="level"]:hover { background:#fff1bb; }
        QPushButton[cartoon="true"][color="level"]:pressed { background:#f7cf6c; border-bottom-width:5px; padding-top:12px; }
        QPushButton[cartoon="true"]:disabled { background:#c5bd9f; color:#81745e; }
        QTabWidget::pane { border:3px solid #9b7444; border-radius:12px; background:#fff2d4; }
        QTabBar::tab { background:#e4c18b; padding:12px 28px; border:2px solid #9b7444; border-radius:8px; font-weight:bold; }
        QTabBar::tab:selected { background:#b8d879; }
        QSlider::groove:horizontal { height:12px; border-radius:6px; background:#d9bc86; }
        QSlider::sub-page:horizontal { background:#91ba58; border-radius:6px; }
        QSlider::handle:horizontal { background:#f6ce6c; border:3px solid #65452d; width:22px; margin:-7px 0; border-radius:10px; }
        QScrollBar:vertical { background:#e9cf9b; width:14px; margin:0; border-radius:7px; }
        QScrollBar::handle:vertical { background:#ae8144; border:2px solid #805c34; border-radius:6px; min-height:30px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }
    )");
}
void GameUi::styleButton(QPushButton *button, const QString& color) {
    button->setProperty("cartoon",true);
    button->setProperty("color",color);
    button->setFocusPolicy(Qt::NoFocus);
    button->setStyleSheet(styleSheet());
}
void GameUi::apply(QWidget *widget) { widget->setStyleSheet(styleSheet()); }
