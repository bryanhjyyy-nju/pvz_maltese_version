#include "almanacdialog.h"
#include "gamecatalog.h"
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QMovie>
#include <QScrollArea>
#include <QTabWidget>
#include <QVBoxLayout>

AlmanacDialog::AlmanacDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("草坪图鉴 · 植物与僵尸");
    resize(1000, 720);
    setStyleSheet("QDialog {background:#f2f5e9;} QLabel {color:#26392e;}"
                  "QFrame#entry {background:#fffdf5; border:1px solid #c6d4b6; border-radius:12px;}"
                  "QTabBar::tab {padding:12px 28px;} QScrollArea {border:0;}");
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel("认识你的草坪伙伴", this);
    title->setStyleSheet("font-size:26px; font-weight:bold; padding:10px;");
    layout->addWidget(title);
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
            auto *details = new QLabel(group == 0 ? GameCatalog::plantDetails(i) : GameCatalog::enemyDetails(i),entry);
            details->setWordWrap(true);
            details->setMinimumWidth(260);
            details->setStyleSheet("font-size:15px; padding:5px;");
            row->addWidget(details,1);
            grid->addWidget(entry,i/2,i%2);
        }
        grid->setRowStretch((count+1)/2,1);
        scroll->setWidget(page);
        tabs->addTab(scroll,group == 0 ? "植物卡片 · 小白" : "僵尸卡片 · 金毛");
    }
    auto *hint = new QLabel("关卡逐步开放植物卡片；爱心可点击收集。R 切换铲子，Esc 取消选择。",this);
    hint->setWordWrap(true);
    layout->addWidget(hint);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close,this);
    connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
    layout->addWidget(buttons);
}
