#pragma once
#include <QString>
class QPushButton;
class QWidget;
namespace GameUi {
QString fontFamily(const QString& preferred = QString());
QString styleSheet();
void styleButton(QPushButton *button, const QString& color = "green");
void apply(QWidget *widget);
}
