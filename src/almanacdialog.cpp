#include "almanacdialog.h"
#include "gamecatalog.h"
#include "gameui.h"
#include <QVariant>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QMovie>
#include <QScrollArea>
#include <QTabWidget>
#include <QVBoxLayout>

AlmanacDialog::AlmanacDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("草坪图鉴 · 小白与金毛");
    resize(1000, 720);
    setStyleSheet(GameUi::styleSheet()+"QScrollArea {border:0; background:#fff2d4;} QScrollArea > QWidget > QWidget {background:#fff2d4;}");
    auto *layout = new QVBoxLayout(this);
    auto *titleRow = new QHBoxLayout;
    auto *star = new QLabel(this);
    star->setPixmap(QPixmap(":/ui/Image/ui/minimap_icon_star_yellow.png").scaled(42,42,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    titleRow->addWidget(star);
    auto *title = new QLabel("认识你的草坪伙伴", this);
    title->setStyleSheet("font-size:26px; font-weight:bold; padding:10px;");
    titleRow->addWidget(title,1);
    layout->addLayout(titleRow);
    auto *tabs = new QTabWidget(this);
    layout->addWidget(tabs);
    for(int group=0;group<2;++group) {
        auto *scroll = new QScrollArea(tabs);
        scroll->setWidgetResizable(true);
        auto *page = new QWidget(scroll);
        auto *grid = new QGridLayout(page);
        grid->setSpacing(14);
        const int count = group == 0 ? GameCatalog::plants().size() : GameCatalog::enemies().size();
        for(int i=0;i<count;++i) {
            auto *entry = new QFrame(page);
            entry->setObjectName("entry");
            entry->setProperty("panel",true);
            entry->setMinimumHeight(220);
            auto *row = new QHBoxLayout(entry);
            auto *icon = new QLabel(entry);
            icon->setFixedSize(115,150);
            icon->setAlignment(Qt::AlignCenter);
            const QString path = group == 0 ? GameCatalog::plants()[i].image : GameCatalog::enemies()[i].image;
            auto *movie = new QMovie(path,QByteArray(),icon);
            movie->setScaledSize(QSize(110,110));
            icon->setMovie(movie);
            movie->start();
            row->addWidget(icon);
            auto *text = new QVBoxLayout;
            const auto lines=(group == 0 ? GameCatalog::plantDetails(i) : GameCatalog::enemyDetails(i)).split('\n');
            auto *name = new QLabel(lines[0],entry);
            name->setStyleSheet(group == 0 ? "font-size:21px; font-weight:bold; color:#547733;" : "font-size:21px; font-weight:bold; color:#a45c32;");
            text->addWidget(name);
            auto *stats = new QLabel(lines[1]+"\n"+lines[2],entry);
            stats->setWordWrap(true);
            stats->setStyleSheet("background:#f1d79f; border-radius:8px; padding:7px; font-size:14px; color:#65452d;");
            text->addWidget(stats);
            auto *details = new QLabel(lines.mid(3).join("\n"),entry);
            details->setWordWrap(true);
            details->setMinimumWidth(240);
            details->setStyleSheet("font-size:15px; padding:5px;");
            text->addWidget(details,1);
            row->addLayout(text,1);
            grid->addWidget(entry,i/2,i%2);
        }
        grid->setRowStretch((count+1)/2,1);
        scroll->setWidget(page);
        tabs->addTab(scroll,group == 0 ? "小白卡片" : "金毛卡片");
    }
    auto *hint = new QLabel("关卡逐步开放小白卡片；点击爱心收集。点击铲子或按 R 切换，无尽模式点击手套或按 S 搬动小白；冲锋小白不可移动或铲除。右键取消选择，空格暂停，A / D 显示小白 / 金毛血量，F 切换原速 / 二倍速，Esc 退出全屏。",this);
    hint->setWordWrap(true);
    layout->addWidget(hint);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close,this);
    buttons->button(QDialogButtonBox::Close)->setText("关闭");
    GameUi::styleButton(buttons->button(QDialogButtonBox::Close),"gold");
    connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
    layout->addWidget(buttons);
}
