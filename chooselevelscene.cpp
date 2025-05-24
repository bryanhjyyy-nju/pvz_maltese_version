#include "chooselevelscene.h"

ChooseLevelScene::ChooseLevelScene(QWidget *parent)
    : QMainWindow{parent}
{
    //设置固定大小
    setFixedSize(1650,900);

    //设置窗口图标
    setWindowIcon(QIcon(":/Image/dogIcon.jpg"));

    //设置窗口标题
    setWindowTitle("PvZ_Demo");
}
