#include "audiomanager.h"
#include "mainscene.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("DogGarden");
    QCoreApplication::setApplicationName("PvZ_demo");
    AudioManager::instance();
    MainScene w;
    w.show();
    return a.exec();
}
