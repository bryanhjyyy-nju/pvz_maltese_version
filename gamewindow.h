#pragma once
#include <QMainWindow>
#include "progressstore.h"
class QStackedWidget;
class GamePage;
class MainScene;
class ChooseLevelScene;
class PlayScene;

// Owns the only native game window, page navigation and saved progress.
class GameWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit GameWindow(QWidget *parent=nullptr,const QString& progressPath={},bool playOpening=true);
    void showMenu();
    void showLevels();
    void startLevel(int level);
    void continueGame();
    void startEndless();
    void showSaveSettings();
    void setFullScreenEnabled(bool enabled);
    void toggleFullScreen();
    static QSize logicalSize() { return QSize(1650,900); }
    MainScene *homePage() const { return home; }
    ChooseLevelScene *levelPage() const { return levels; }
    PlayScene *playPage() const { return battle; }
    GamePage *currentPage() const;
protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched,QEvent *event) override;
private:
    QStackedWidget *pages;
    MainScene *home;
    ChooseLevelScene *levels;
    PlayScene *battle=nullptr;
    ProgressStore progress;
    QRect windowedGeometry;
    bool closing=false;
    bool playOpening=true;
    void connectPage(GamePage *page);
    void discardBattle();
    void updateDisplayState();
    void refreshHome();
    void connectBattle(bool endless);
};
