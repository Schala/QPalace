#include "room.h"

void RoomDesc::read(QDataStream &data) {
	quint8 length;
	QDataStream room_data;
	
	ServerMsg::read(data, refnum);
	data >> flags >> faces_id >> room_id >> room_name_offs >> img_name_offs >> artist_name_offs >>
		pw_offs >> nspots >> spot_offs >> nimgs >> ndraws >> draw1_offs >> nusers >>
		nlooseprops >> lprop1_offs;
	data.skipRawData(sizeof(qint16)); // padding
	data >> (quint16)this->length;
	
	data.skipRawData(room_name_offs);
	data >> length;
	data.readRawData(room_name.data(), (int)length);
	
	if (img_name_offs > 0) {
		data.skipRawData(img_name_offs);
		data >> length;
		data.readRawData(bg_img_name.data(), (int)length);
	}
	
	if (artist_name_offs > 0) {
		data.skipRawData(artist_name_offs);
		data >> length;
		data.readRawData(artist_name.data(), (int)length);
	}
}
