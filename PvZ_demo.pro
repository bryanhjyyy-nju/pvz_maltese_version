QT += core gui multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

INCLUDEPATH += $$PWD/include

SOURCES += \
    src/allheartwhite.cpp \
    src/almanacdialog.cpp \
    src/audiomanager.cpp \
    src/battlebanner.cpp \
    src/battleresult.cpp \
    src/battlesnapshot.cpp \
    src/bullet.cpp \
    src/card.cpp \
    src/chooselevelscene.cpp \
    src/combateffect.cpp \
    src/dancingwhite.cpp \
    src/dblsingwhite.cpp \
    src/enemyprojectile.cpp \
    src/gameartwork.cpp \
    src/gamecatalog.cpp \
    src/gamepage.cpp \
    src/gamepause.cpp \
    src/gamespeed.cpp \
    src/gameui.cpp \
    src/gamewindow.cpp \
    src/heart.cpp \
    src/heartwhite.cpp \
    src/lawn.cpp \
    src/levelopening.cpp \
    src/leveltutorial.cpp \
    src/linewhite.cpp \
    src/main.cpp \
    src/mainscene.cpp \
    src/map.cpp \
    src/moneywhite.cpp \
    src/mygamescene.cpp \
    src/myitem.cpp \
    src/mypushbutton.cpp \
    src/pausedialog.cpp \
    src/playscene.cpp \
    src/progressstore.cpp \
    src/singingwhite.cpp \
    src/spriteanimation.cpp \
    src/wallwhite.cpp \
    src/waveplanner.cpp \
    src/whitedogs.cpp \
    src/yellowdogs.cpp

HEADERS += \
    include/allheartwhite.h \
    include/almanacdialog.h \
    include/audiomanager.h \
    include/battlebanner.h \
    include/battleresult.h \
    include/battlesnapshot.h \
    include/bullet.h \
    include/card.h \
    include/cardstate.h \
    include/chooselevelscene.h \
    include/combateffect.h \
    include/dancingwhite.h \
    include/dblsingwhite.h \
    include/enemyprojectile.h \
    include/gameartwork.h \
    include/gamecatalog.h \
    include/gamepage.h \
    include/gamepause.h \
    include/gamespeed.h \
    include/gamestate.h \
    include/gameui.h \
    include/gamewindow.h \
    include/heart.h \
    include/heartwhite.h \
    include/lawn.h \
    include/levelopening.h \
    include/leveltutorial.h \
    include/linewhite.h \
    include/mainscene.h \
    include/map.h \
    include/moneywhite.h \
    include/myDirection.h \
    include/mygamescene.h \
    include/myitem.h \
    include/mypushbutton.h \
    include/pausedialog.h \
    include/playscene.h \
    include/progressstore.h \
    include/singingwhite.h \
    include/spriteanimation.h \
    include/wallwhite.h \
    include/waveplanner.h \
    include/whitedogs.h \
    include/yellowdogs.h

RESOURCES += res.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
