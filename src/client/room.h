#ifndef _ROOM_CLIENT_H
#define _ROOM_CLIENT_H

#include "../protocol/message.h"
#include <QByteArray>

class RoomDesc final: public ServerMsg {
public:
	RoomDesc():
		id(MSG_ROOMDESC),
		refnum(0),
		bg_img_name(""),
		artist_name("") {};
	
	void read(QDataStream &data);
private:
	qint32 flags, faces_id;
	qint16 room_id, room_name_offs, img_name_offs, artist_name_offs, pw_offs,
		nspots, spot_offs, nimgs, ndraws, draw1_offs, nusers, nlooseprops, lprop1_offs;
	QByteArray room_name, bg_img_name, artist_name;
};

#endif // _ROOM_CLIENT_H
