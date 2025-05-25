#include "chooselevelscene.h"
#include <QPainter>
#include "mypushbutton.h"
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include "playscene.h"

ChooseLevelScene::ChooseLevelScene(QWidget *parent)
    : QMainWindow{parent}
{
    //设置固定大小
    setFixedSize(1650,900);

    //设置窗口图标
    setWindowIcon(QIcon(":/Image/dogIcon.jpg"));

    //设置窗口标题
    setWindowTitle("PvZ_Demo");

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
            emit this->chooseSceneBack();
        });
    });

    //创建关卡按钮十个
    for(int i = 0; i < 10; i++){
        MyPushButton *levelBtn = new MyPushButton(":/Image/levelIcon.png");
        levelBtn->setParent(this);
        levelBtn->move(300 * (i % 5) + 120, 200 + i / 5 * 300);


        //监听每个按钮的点击事件
        connect(levelBtn,&MyPushButton::clicked,this,[=](){
            // qDebug() << i + 1;

            //进入游戏场景
            this->hide();
            play = new PlayScene(i + 1);
            play->show();

            //监听游戏界面的返回信号
            connect(play,&PlayScene::playSceneBack,this,[=](){
                //todo 实现存档功能
                delete play;
                play = NULL;
                this->show();
            });
        });

        //显示文字：第 i 关
        QLabel *label = new QLabel;
        label->setParent(this);
        label->setFixedSize(levelBtn->width(),levelBtn->height());
        label->setText(QString::number(i + 1));

        //设置字体颜色和大小
        QFont font;
        font.setFamily("Arial");
        font.setPointSize(45);
        font.setBold(true);
        label->setFont(font);
        label->setStyleSheet("color: pink;");

        label->move(300 * (i % 5) + 120, 200 + i / 5 * 300);
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        //使得鼠标能够穿透label
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
}

void ChooseLevelScene::paintEvent(QPaintEvent *){
    QPainter painter(this);
    QPixmap pix;

    //背景图片
    pix.load(":/Image/StartPage.jpg");

    painter.drawPixmap(0,0,this->width(),this->height(),pix);

    //绘制关卡选择四个字
    pix.load(":Image/chooseTitle.png");
    pix = pix.scaled(pix.width() * 2,pix.height() * 2);
    painter.drawPixmap(this->width() * 0.2 - pix.width() * 0.5,this->height() * 0.1,pix);

}
