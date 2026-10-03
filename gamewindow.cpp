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

GameWindow::GameWindow(QWidget *parent,const QString& progressPath)
    : QMainWindow(parent), pages(new QStackedWidget(this)),
      home(new MainScene(pages)), levels(new ChooseLevelScene(pages)), progress(progressPath) {
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
    connect(home,&MainScene::quitRequested,this,&QWidget::close);
    connect(levels,&ChooseLevelScene::levelRequested,this,&GameWindow::startLevel);
    connect(levels,&ChooseLevelScene::continueRequested,this,&GameWindow::continueGame);
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
    old->shutdown();
    pages->removeWidget(old);
    old->deleteLater();
}
void GameWindow::showMenu() {
    if(closing) return;
    discardBattle();
    AudioManager::instance().setBattle(false);
    pages->setCurrentWidget(home);
    setWindowTitle("小白大战小金毛");
}
void GameWindow::showLevels() {
    if(closing) return;
    discardBattle();
    AudioManager::instance().setBattle(false);
    progress.load();
    levels->refreshProgress(progress);
    pages->setCurrentWidget(levels);
    setWindowTitle("小白大战小金毛 · 选择关卡");
}
void GameWindow::startLevel(int level) {
    if(closing || battle || level<1 || level>10) return;
    if(!progress.startLevel(level)) QMessageBox::warning(this,"存档未写入",progress.error());
    battle=new PlayScene(level,pages);
    pages->addWidget(battle);
    connectPage(battle);
    connect(battle,&PlayScene::gameWin,this,[this,level] {
        if(!progress.completeLevel(level)) QMessageBox::warning(this,"存档未写入",progress.error());
    });
    connect(battle,&PlayScene::playSceneBack,this,&GameWindow::showLevels);
    connect(battle,&PlayScene::mainMenuRequested,this,&GameWindow::showMenu);
    pages->setCurrentWidget(battle);
    setWindowTitle(QString("小白大战小金毛 · 第 %1 关").arg(level));
}
void GameWindow::continueGame() {
    progress.load();
    if(progress.hasProgress()) startLevel(progress.resumeLevel());
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
