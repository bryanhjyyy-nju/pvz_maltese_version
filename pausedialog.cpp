#include "pausedialog.h"
#include "gameui.h"
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QVariant>
PauseDialog::PauseDialog(QWidget *parent) : QDialog(parent) {
    setObjectName("pauseMenu");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);
    setFixedSize(400,440);
    GameUi::apply(this);
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0,0,0,0);
    auto *panel = new QFrame(this);
    panel->setProperty("panel",true);
    outer->addWidget(panel);
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(30,20,30,20);
    auto *title = new QLabel("休息一下！",panel);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:30px; font-weight:bold; color:#805024;");
    layout->addWidget(title);
    auto add = [&](const QString& text, const char *name, const QString& color, void(PauseDialog::*signal)()) {
        auto *button = new QPushButton(text,panel);
        button->setObjectName(name);
        button->setMinimumHeight(52);
        GameUi::styleButton(button,color);
        layout->addWidget(button);
        connect(button,&QPushButton::clicked,this,signal);
    };
    add("继续游戏  [空格]","resume","green",&PauseDialog::resumeRequested);
    add("植物 / 僵尸图鉴","almanac","gold",&PauseDialog::almanacRequested);
    add("声音设置","audio","gold",&PauseDialog::audioRequested);
    add("返回主菜单","mainMenu","red",&PauseDialog::mainMenuRequested);
    auto *hint = new QLabel("已自动记录本关 · 下次可继续",panel);
    hint->setAlignment(Qt::AlignCenter);
    layout->addWidget(hint);
}
void PauseDialog::reject() { emit resumeRequested(); }
void PauseDialog::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Space) {
        if(!event->isAutoRepeat()) emit resumeRequested();
        event->accept();
    }
    else QDialog::keyPressEvent(event);
}
