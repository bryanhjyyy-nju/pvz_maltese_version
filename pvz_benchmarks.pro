include(PvZ_demo.pro)
QT += testlib
CONFIG += console testcase
CONFIG -= app_bundle
TARGET = pvz_benchmarks
SOURCES -= src/main.cpp
SOURCES += src/tests/benchmark_game.cpp
win32: LIBS += -lpsapi
