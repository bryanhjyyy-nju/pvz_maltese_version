#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QPushButton>

class MyPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MyPushButton(QWidget *parent = nullptr);

    //构造函数 第一个参数 正常显示的图片路径  第二个参数 按下后显示的图片路径
    MyPushButton(QString normalImg, QString pressImg = "");

    //成员属性 保存用户传入的默认显示路径 以及 按下后显示的图片路径
    QString normalImgPath;
    QString pressImgPath;

signals:
};

#endif // MYPUSHBUTTON_H
