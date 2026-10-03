include(PvZ_demo.pro)
QT += testlib
CONFIG += console testcase
CONFIG -= app_bundle
TARGET = pvz_tests
SOURCES -= main.cpp
SOURCES += tests/test_game.cpp
INCLUDEPATH += $$PWD
