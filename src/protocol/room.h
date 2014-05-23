#ifndef _ROOM_H
#define _ROOM_H

#include <QtGlobal>

#define RF_AUTHORLOCKED 0x0001
#define RF_PRIVATE 0x0002
#define RF_NOPAINTING 0x0004
#define RF_CLOSED 0x0008
#define RF_NOCYBORGS 0x0010
#define RF_HIDDEN 0x0020
#define RF_NOGUESTS 0x0040
#define RF_WIZARDSONLY 0x0080
#define RF_DROPZONE 0x0100

#define GATE_ID 86

struct Point {
	qint16 v, h;
};

#endif _ROOM_H
