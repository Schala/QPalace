######################################################################
# Automatically generated by qmake (3.0) Sun Jun 22 10:43:10 2014
######################################################################

TEMPLATE = app
INCLUDEPATH += .
QT -= gui
CONFIG += c++11 enctest

regtest {
	TARGET = regtest
	HEADERS += ../src/client/registration.hpp
	SOURCES += regtest.cpp \
						../src/client/registration.cpp
}

enctest {
	TARGET = enctest
	HEADERS += ../src/crypt.hpp
	SOURCES += enctest.cpp \
						../src/crypt.cpp
}

dectest {
	TARGET = dectest
	HEADERS += ../src/crypt.hpp
	SOURCES += dectest.cpp \
						../src/crypt.cpp
}
