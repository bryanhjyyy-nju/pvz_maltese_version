#pragma once
#include "gamepage.h"
class MainScene : public GamePage {
    Q_OBJECT
public:
    explicit MainScene(QWidget *parent=nullptr);
    void refreshState(bool campaignActive,bool endlessUnlocked,bool endlessActive,int bestWave);
protected:
    void paintEvent(QPaintEvent *) override;
signals:
    void startRequested();
    void continueRequested();
    void quitRequested();
    void endlessRequested();
    void saveSettingsRequested();
private:
    void setGif(int w,int h,int x,int y);
    class QPushButton *resumeButton=nullptr,*endlessButton=nullptr;
    class QLabel *recordLabel=nullptr;
};
