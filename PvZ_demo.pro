QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    allheartwhite.cpp \
    bullet.cpp \
    card.cpp \
    chooselevelscene.cpp \
    dancingwhite.cpp \
    dblsingwhite.cpp \
    heart.cpp \
    heartwhite.cpp \
    linewhite.cpp \
    main.cpp \
    mainscene.cpp \
    map.cpp \
    moneywhite.cpp \
    mygamescene.cpp \
    myitem.cpp \
    mypushbutton.cpp \
    playscene.cpp \
    singingwhite.cpp \
    wallwhite.cpp \
    whitedogs.cpp \
    yellowdogs.cpp

HEADERS += \
    allheartwhite.h \
    bullet.h \
    card.h \
    cardstate.h \
    chooselevelscene.h \
    dancingwhite.h \
    dblsingwhite.h \
    gamestate.h \
    heart.h \
    heartwhite.h \
    linewhite.h \
    mainscene.h \
    map.h \
    moneywhite.h \
    myDirection.h \
    mygamescene.h \
    myitem.h \
    mypushbutton.h \
    playscene.h \
    singingwhite.h \
    wallwhite.h \
    whitedogs.h \
    yellowdogs.h

FORMS += \
    mainscene.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

SOURCES += gamecatalog.cpp
HEADERS += gamecatalog.h

SOURCES += progressstore.cpp
HEADERS += progressstore.h

SOURCES += almanacdialog.cpp
HEADERS += almanacdialog.h

SOURCES += audiomanager.cpp
HEADERS += audiomanager.h

SOURCES += gamepause.cpp
HEADERS += gamepause.h
