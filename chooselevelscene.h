#pragma once
#include "gamepage.h"
class ProgressStore;
class QPushButton;
class QLabel;
class ChooseLevelScene : public GamePage {
    Q_OBJECT
public:
    explicit ChooseLevelScene(QWidget *parent=nullptr);
    void refreshProgress(const ProgressStore& progress);
protected:
    void paintEvent(QPaintEvent *) override;
signals:
    void levelRequested(int level);
    void continueRequested();
    void backRequested();
private:
    QPushButton *continueButton=nullptr;
    QLabel *progressLabel=nullptr;
    void buildBackBtn();
    void buildLevelBtn();
};
