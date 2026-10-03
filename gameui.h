#pragma once
#include <QString>
class QPushButton;
class QWidget;
namespace GameUi {
QString styleSheet();
void styleButton(QPushButton *button, const QString& color = "green");
void apply(QWidget *widget);
}
