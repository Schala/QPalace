#include "room.h"

QDataStream& RoomDesc::write() {
	QDataStream data ServerMsg::write();
	
	data << flags << faces_id << room_id << room_name_offs;
	if (bg_img_name.isEmpty() == false)
		img_name_offs = room_name_offs + (qint16)room_name.size() + 1;
	data << img_name_offs;
	if (artist_name.isEmpty() == false)
		artist_name_offs = img_name_offs + (qint16)bg_img_name.size() + 1;
	data << artist_name_offs << pw_offs << nspots << spot_offs << nimgs << img_oofs <<
		ndraws << draw1_offs << nusers << nlooseprops << lprop1_offs << (quint16)0 <<
		(quint16)length; // todo: calculate length before writing any data
	
	// var buffer starts here
	data << (quint16)0 << (quint8)room_name.size() << room_name.data();
	if (bg_img_name.isEmpty() == false)
		data << (quint8)bg_img_name.size() << bg_img_name.data();
	if (artist_name.isEmpty() == false)
		data << (quint8)artist_name.size() << artist_name.data();
	
	// ... incomplete...
	return data;
}
