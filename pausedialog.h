#pragma once
#include <QDialog>
class PauseDialog : public QDialog {
    Q_OBJECT
public:
    explicit PauseDialog(QWidget *parent);
signals:
    void resumeRequested();
    void mainMenuRequested();
    void almanacRequested();
    void audioRequested();
protected:
    void reject() override;
    void keyPressEvent(class QKeyEvent *event) override;
};
