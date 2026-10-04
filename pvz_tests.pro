include(PvZ_demo.pro)
QT += testlib
CONFIG += console testcase
CONFIG -= app_bundle
TARGET = pvz_tests
SOURCES -= src/main.cpp
SOURCES += src/tests/test_game.cpp
