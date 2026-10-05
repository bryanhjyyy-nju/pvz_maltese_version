#include "audiomanager.h"
#include "gamewindow.h"
#include "gameui.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setFont(QFont(GameUi::fontFamily()));
    QCoreApplication::setOrganizationName("DogGarden");
    QCoreApplication::setApplicationName("PvZ_demo");
    AudioManager::instance();
    GameWindow w;
    w.show();
    return a.exec();
}
