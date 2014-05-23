#ifndef _ROOM_SERVER_H
#define _ROOM_SERVER_H

#include "../protocol/message.h"
#include <QByteArray>

class RoomDesc final: public ServerMsg {
public:
	RoomDesc():
		id(MSG_ROOMDESC), refnum(0), flags(0), faces_id(0), room_id(0), room_name_offs(2), // room name offset is always 2 (?)
		img_name_offs(0), artist_name_offs(0), pw_offs(0), nspots(0), spot_offs(0),
		nimgs(0), img_offs(0), ndraws(0), draw1_offs(0), nusers(0), nlooseprops(0),
		lprop1_offs(0), room_name(""), bg_img_name(""), artist_name("") {};
	
	QDataStream& write();
private:
	qint32 flags, faces_id;
	qint16 room_id, room_name_offs, img_name_offs, artist_name_offs, pw_offs,
		nspots, spot_offs, nimgs, img_offs, ndraws, draw1_offs, nusers, nlooseprops, lprop1_offs;
	QByteArray room_name, bg_img_name, artist_name;
};

#endif // _ROOM_SERVER_H
