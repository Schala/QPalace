######################################################################
# Automatically generated by qmake (3.0) Thu Jun 26 19:15:34 2014
######################################################################

TEMPLATE = app
TARGET = qpserver
INCLUDEPATH += .
QT -= gui
QT += network sql concurrent
CONFIG += console c++11
DEFINES += SERVER

HEADERS += ../shared.hpp ../message.hpp ../connection.hpp ../room.hpp server.hpp \
    ../blowthru.hpp \
    ../codec.hpp
SOURCES += ../message.cpp ../connection.cpp ../room.cpp server.cpp main.cpp \
    ../blowthru.cpp \
    ../codec.cpp
