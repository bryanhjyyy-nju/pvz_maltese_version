include(PvZ_demo.pro)
QT += testlib
CONFIG += console testcase
CONFIG -= app_bundle
TARGET = pvz_benchmarks
SOURCES -= main.cpp
SOURCES += tests/benchmark_game.cpp
INCLUDEPATH += $$PWD
win32: LIBS += -lpsapi
