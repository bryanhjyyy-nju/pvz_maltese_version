#include "gamewindow.h"
#include "gamepage.h"
#include "mainscene.h"
#include "chooselevelscene.h"
#include "playscene.h"
#include "audiomanager.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "gameui.h"

GameWindow::GameWindow(QWidget *parent,const QString& progressPath,bool openingEnabled)
    : QMainWindow(parent), pages(new QStackedWidget(this)),
      home(new MainScene(pages)), levels(new ChooseLevelScene(pages)), progress(progressPath),playOpening(openingEnabled) {
    setCentralWidget(pages);
    resize(logicalSize());
    setMinimumSize(825,450);
    setWindowIcon(QIcon(":/white/Image/dogIcon.jpg"));
    pages->addWidget(home);
    pages->addWidget(levels);
    connectPage(home);
    connectPage(levels);
    connect(home,&MainScene::startRequested,this,&GameWindow::showLevels);
    connect(home,&MainScene::continueRequested,this,&GameWindow::continueGame);
    connect(home,&MainScene::endlessRequested,this,&GameWindow::startEndless);
    connect(home,&MainScene::saveSettingsRequested,this,&GameWindow::showSaveSettings);
    connect(home,&MainScene::quitRequested,this,&QWidget::close);
    connect(levels,&ChooseLevelScene::levelRequested,this,&GameWindow::startLevel);
    connect(levels,&ChooseLevelScene::backRequested,this,&GameWindow::showMenu);
    qApp->installEventFilter(this);
    showMenu();
}
GamePage *GameWindow::currentPage() const { return qobject_cast<GamePage*>(pages->currentWidget()); }
void GameWindow::connectPage(GamePage *page) {
    connect(page,&GamePage::fullScreenRequested,this,&GameWindow::toggleFullScreen);
    page->setDisplayState(isFullScreen(),isMaximized());
}
void GameWindow::discardBattle() {
    if(!battle) return;
    auto *old=battle;
    battle=nullptr;
    old->disconnect(this);
    old->findChild<MyGameScene*>()->disconnect(this);
    old->shutdown();
    pages->removeWidget(old);
    old->deleteLater();
}
void GameWindow::showMenu() {
    if(closing) return;
    if(battle && !battle->isFinished() && !battle->isPaused()) return;
    if(battle && battle->isFinished()) discardBattle();
    if(battle) battle->suspendToMenu();
    refreshHome();
    AudioManager::instance().setBattle(false);
    pages->setCurrentWidget(home);
    setWindowTitle("小白大战小金毛");
}
void GameWindow::showLevels() {
    if(closing) return;
    if(battle && !battle->isFinished() && !battle->isPaused()) return;
    if(battle && battle->isFinished()) discardBattle();
    if(battle) battle->suspendToMenu();
    AudioManager::instance().setBattle(false);
    progress.load();
    levels->refreshProgress(progress);
    pages->setCurrentWidget(levels);
    setWindowTitle("小白大战小金毛 · 选择关卡");
}
void GameWindow::startLevel(int level) {
    if(closing || !progress.isUnlocked(level)) return;
    if(battle && !battle->isPaused() && !battle->isFinished()) return;
    discardBattle();
    if(!progress.startLevel(level)) QMessageBox::warning(this,"存档未写入",progress.error());
    battle=new PlayScene(level,pages,playOpening);
    connectBattle(false);
    pages->setCurrentWidget(battle);
    setWindowTitle(QString("小白大战小金毛 · 第 %1 关").arg(level));
}
void GameWindow::continueGame() {
    if(!progress.hasUnfinishedLevel()) return;
    if(!battle) startLevel(progress.resumeLevel());
    if(battle) {
        AudioManager::instance().setBattle(true);
        pages->setCurrentWidget(battle);
        battle->showPauseMenu();
        setWindowTitle(QString("小白大战小金毛 · 第 %1 关").arg(battle->levelIndex));
    }
}
void GameWindow::refreshHome() {
    home->refreshState(progress.hasUnfinishedLevel(),progress.endlessUnlocked(),progress.hasEndlessRun(),progress.endlessBest());
}
void GameWindow::connectBattle(bool endless) {
    pages->addWidget(battle);
    connectPage(battle);
    connect(battle,&PlayScene::gameWin,this,[this] {
        if(!progress.completeLevel(battle->levelIndex)) QMessageBox::warning(this,"存档未写入",progress.error());
        refreshHome();
    });
    connect(battle,&PlayScene::gameLose,this,[this,endless] {
        const bool saved=endless ? progress.finishEndless() : progress.finishAttempt();
        if(!saved) QMessageBox::warning(this,"存档未写入",progress.error());
        refreshHome();
    });
    if(endless) connect(battle->findChild<MyGameScene*>(),&MyGameScene::waveStarted,this,[this](int wave,int) {
        if(!progress.recordEndlessWave(wave)) QMessageBox::warning(this,"存档未写入",progress.error());
        refreshHome();
    });
    connect(battle,&PlayScene::playSceneBack,this,&GameWindow::showLevels);
    connect(battle,&PlayScene::mainMenuRequested,this,&GameWindow::showMenu);
}
void GameWindow::startEndless() {
    if(closing || !progress.endlessUnlocked()) return;
    if(battle && !battle->isPaused() && !battle->isFinished()) return;
    const bool continuing=progress.hasEndlessRun();
    if(!battle || !battle->endlessMode) {
        discardBattle();
        if(!continuing && !progress.startEndless()) { QMessageBox::warning(this,"存档未写入",progress.error()); return; }
        battle=new PlayScene(10,pages,playOpening,true,progress.endlessCheckpoint());
        connectBattle(true);
    }
    AudioManager::instance().setBattle(true);
    pages->setCurrentWidget(battle);
    setWindowTitle("小白大战小金毛 · 无尽模式");
    if(continuing) battle->showPauseMenu();
}
void GameWindow::showSaveSettings() {
    QDialog dialog(this); dialog.setWindowTitle("存档管理"); dialog.setObjectName("saveSettingsDialog");
    GameUi::apply(&dialog); dialog.setMinimumWidth(520);
    auto *layout=new QVBoxLayout(&dialog);
    auto *description=new QLabel("新存档会清空关卡进度、当前游戏和无尽纪录。\n全解锁会开放全部关卡及无尽模式。",&dialog);
    layout->addWidget(description);
    auto *fresh=new QPushButton("清空存档，重新开始",&dialog); fresh->setObjectName("resetSave");
    auto *unlock=new QPushButton("一键全解锁",&dialog); unlock->setObjectName("unlockSave");
    auto *close=new QPushButton("返回主菜单",&dialog);
    for(auto *button : {fresh,unlock,close}) { GameUi::styleButton(button,"gold"); layout->addWidget(button); }
    connect(fresh,&QPushButton::clicked,&dialog,[this,&dialog] {
        if(!progress.reset()) { QMessageBox::warning(&dialog,"存档未写入",progress.error()); return; }
        discardBattle(); refreshHome(); dialog.accept();
    });
    connect(unlock,&QPushButton::clicked,&dialog,[this,&dialog] {
        if(!progress.unlockAll()) { QMessageBox::warning(&dialog,"存档未写入",progress.error()); return; }
        refreshHome(); dialog.accept();
    });
    connect(close,&QPushButton::clicked,&dialog,&QDialog::accept);
    dialog.exec();
}
void GameWindow::setFullScreenEnabled(bool enabled) {
    if(enabled==isFullScreen() && !(isMaximized() && !enabled)) return;
    if(auto *page=currentPage()) emit page->displayModeChanging();
    if(enabled) {
        windowedGeometry=isMaximized() ? normalGeometry() : geometry();
        showFullScreen();
    } else {
        const auto restore=isFullScreen() && windowedGeometry.isValid() ? windowedGeometry : normalGeometry();
        showNormal();
        if(restore.isValid()) setGeometry(restore);
    }
    updateDisplayState();
}
void GameWindow::toggleFullScreen() { setFullScreenEnabled(!(isFullScreen() || isMaximized())); }
void GameWindow::updateDisplayState() {
    for(int i=0;i<pages->count();++i)
        static_cast<GamePage*>(pages->widget(i))->setDisplayState(isFullScreen(),isMaximized());
}
void GameWindow::changeEvent(QEvent *event) {
    QMainWindow::changeEvent(event);
    if(event->type()==QEvent::WindowStateChange) {
        if(auto *page=currentPage()) emit page->displayModeChanging();
        updateDisplayState();
    }
}
bool GameWindow::eventFilter(QObject *watched,QEvent *event) {
    if(event->type()!=QEvent::KeyPress) return QMainWindow::eventFilter(watched,event);
    auto *widget=qobject_cast<QWidget*>(watched);
    while(widget && widget!=this) widget=widget->parentWidget();
    if(!widget) return false;
    auto *key=static_cast<QKeyEvent*>(event);
    if(key->key()==Qt::Key_Escape) {
        if(!key->isAutoRepeat() && isFullScreen()) setFullScreenEnabled(false);
        return true;
    }
    if(key->key()==Qt::Key_F11) {
        if(!key->isAutoRepeat()) toggleFullScreen();
        return true;
    }
    return currentPage() && currentPage()->processGameKey(key);
}
void GameWindow::closeEvent(QCloseEvent *event) {
    closing=true;
    if(battle) battle->shutdown();
    for(auto *dialog : findChildren<QDialog*>()) dialog->hide();
    AudioManager::instance().stopAll();
    event->accept();
}
