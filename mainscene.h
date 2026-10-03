#pragma once
#include "gamepage.h"
class MainScene : public GamePage {
    Q_OBJECT
public:
    explicit MainScene(QWidget *parent=nullptr);
protected:
    void paintEvent(QPaintEvent *) override;
signals:
    void startRequested();
    void continueRequested();
    void quitRequested();
private:
    void buildStartBtn();
    void buildQuitBtn();
    void setGif(int w,int h,int x,int y);
};
