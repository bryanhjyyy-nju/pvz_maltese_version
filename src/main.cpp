#include "audiomanager.h"
#include "gamewindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("DogGarden");
    QCoreApplication::setApplicationName("PvZ_demo");
    AudioManager::instance();
    GameWindow w;
    w.show();
    return a.exec();
}
