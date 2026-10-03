#ifndef MAINSCENE_H
#define MAINSCENE_H

#include <QMainWindow>
#include "gamewindow.h"
#include "chooselevelscene.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainScene;
}
QT_END_NAMESPACE

class MainScene : public GameWindow
{
    Q_OBJECT

public:
    explicit MainScene(QWidget *parent = nullptr, const QString& progressPath = {});
    ~MainScene();

    //重写paintEvent事件 画背景图
    void paintEvent(QPaintEvent *) override;

    //
    ChooseLevelScene *chooseScene = nullptr;
private:
    //ui界面（未使用）
    Ui::MainScene *ui;

    void buildStartBtn(const QString& progressPath); //创建开始按钮
    void buildQuitBtn(); //创建退出按钮
    void setGif(int w,int h,int x,int y); //显示我的动图
};
#endif // MAINSCENE_H
