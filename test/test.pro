######################################################################
# Automatically generated by qmake (3.0) Sun Jun 22 10:43:10 2014
######################################################################

TEMPLATE = app
INCLUDEPATH += .
QT -= gui
QMAKE_CXXFLAGS += -std=gnu++11

win32 {
	CONFIG += static
	QMAKE_CXXFLAGS += -static-libgcc -static-libstdc++ -static
}

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
