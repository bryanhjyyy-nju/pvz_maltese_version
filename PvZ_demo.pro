QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    card.cpp \
    chooselevelscene.cpp \
    heart.cpp \
    heartwhite.cpp \
    main.cpp \
    mainscene.cpp \
    map.cpp \
    mygamescene.cpp \
    myitem.cpp \
    mypushbutton.cpp \
    playscene.cpp \
    singingwhite.cpp \
    wallwhite.cpp \
    whitedogs.cpp

HEADERS += \
    card.h \
    cardstate.h \
    chooselevelscene.h \
    gamestate.h \
    heart.h \
    heartwhite.h \
    mainscene.h \
    map.h \
    mygamescene.h \
    myitem.h \
    mypushbutton.h \
    playscene.h \
    singingwhite.h \
    wallwhite.h \
    whitedogs.h

FORMS += \
    mainscene.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
