#include "message.hpp"

QDataStream& operator<<(QDataStream &out, const QPMessage &msg)
{
	out << msg.id() << msg.size() << msg.ref();
	if (msg.size() != 0)
		out.writeRawData(msg.data(), msg.size());
	return out;
}

QDataStream& operator>>(QDataStream &in, QPMessage &msg)
{
	quint32 size;
	in >> msg.mId >> size >> msg.mRef;
	char *buf = new char[size];
	in.readRawData(buf, size);
	msg.mData = QByteArray(buf, size);
#ifndef QT_NO_DEBUG
	qDebug("eventType = %x\nlength = %u\nrefNum = %d\n", msg.id(), msg.size(), msg.ref());
#endif // QT_NO_DEBUG
	return in;
}
