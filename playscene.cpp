#include "playscene.h"
#include <QPainter>
#include "mypushbutton.h"
#include <QTimer>
#include <QLabel>

PlayScene::PlayScene(QWidget *parent)
    : QMainWindow{parent}
{}

PlayScene::PlayScene(int levelNum){
    //维护传进来的关卡号
    this->levelIndex = levelNum;
    //设置标题
    QString str = QString(" 第 %1 关").arg(levelNum);

    //初始化游戏场景
    //设置固定大小
    setFixedSize(1650,900);

    //设置窗口图标
    setWindowIcon(QIcon(":/Image/dogIcon.jpg"));

    //设置窗口标题
    setWindowTitle("PvZ_Demo" + str);

    //返回按钮
    MyPushButton *backBtn = new MyPushButton(":/Image/backBtn.png");
    backBtn->setParent(this);
    backBtn->move(this->width() - backBtn->width() * 1.2,backBtn->width() * 0.2);

    connect(backBtn,&MyPushButton::clicked,this,[=](){
        //点击动画
        backBtn->zoom1();
        backBtn->zoom2();

        //延时返回
        QTimer::singleShot(300,this,[=](){
            emit this->playSceneBack();
        });
    });

    //显示当前关卡数
}

void PlayScene::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pix;

    //背景图片
    pix.load(":/Image/grass.jpg");

    painter.drawPixmap(0,0,pix.width() * this->height() / pix.height(),this->height(),pix);

}
