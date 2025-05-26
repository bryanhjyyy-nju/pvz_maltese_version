#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QPushButton>
#include <QMainWindow>

class MyPushButton : public QPushButton
{
    Q_OBJECT
public:
    //构造函数 第一个参数 正常显示的图片路径  第二个参数 按下后显示的图片路径
    MyPushButton(QString firstImg, QString secondImg = "", float times = 1.5);

    //成员属性 保存用户传入的默认显示路径 以及 按下后显示的图片路径
    QString firstImgPath;
    QString secondImgPath;
    void buildBackBtn(QMainWindow *parent);

    //特效
    //todo 更改特效
    void zoom1();//往下跳
    void zoom2();//往上跳

    //点击一个卡牌，卡牌变灰色
private:



signals:
};

#endif // MYPUSHBUTTON_H
