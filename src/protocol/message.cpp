#include "message.h"

void ServerMsg::read(QDataStream &data, qint32 refnum) {
	data.skipRawData(sizeof(quint32)); // ID should be set by deriving class, not by stream
	data >> length;
	data >> refnum;
}

QDataStream& ServerMsg::write() {
	QDataStream data;
	data << id;
	data << length;
	data << refnum;
	
	return data;
}
