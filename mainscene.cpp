#include "mainscene.h"
#include "ui_mainscene.h"
#include <QPainter>

MainScene::MainScene(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainScene)
{
    ui->setupUi(this);

    //配置主场景

    //设置固定大小
    setFixedSize(1100,600);

    //设置窗口图标
    setWindowIcon(QIcon(":/Image/dogIcon.jpg"));

    //设置窗口标题
    setWindowTitle("PvZ_Demo");


}

void MainScene::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pix;

    //背景图片
    pix.load(":/Image/StartPage.jpg");

    painter.drawPixmap(0,0,this->width(),this->height(),pix);

    //游戏标题
    pix.load(":/Image/Title.png");
    pix = pix.scaled(pix.width() * 1.5,pix.height() * 1.5);
    painter.drawPixmap(217,100,pix);
}

MainScene::~MainScene()
{
    delete ui;
}
