#ifndef QT_NO_DEBUG
#include <QFile>
#endif // QT_NO_DEBUG

#include "message.hpp"

QDataStream& operator<<(QDataStream &out, const QPMessage &msg)
{
	out << msg.id() << msg.size() << msg.ref();
	if (msg.size() != 0)
		out.writeRawData(msg.data(), msg.size());
#ifndef QT_NO_DEBUG
	qDebug("eventType = %x\nlength = %u\nrefNum = %d", msg.id(), msg.size(), msg.ref());
#endif // QT_NO_DEBUG
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
	qDebug("eventType = %x\nlength = %u\nrefNum = %d", msg.id(), msg.size(), msg.ref());
#endif // QT_NO_DEBUG
	return in;
}

#ifndef QT_NO_DEBUG
void QPMessage::dump() const
{
	QFile file(QString("/tmp/").append(QString().setNum(mId, 16)).append(".dat"));
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		qWarning("QPServer is too constipated and unable to take a packet dump!");
	QDataStream ds(&file);
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << *this;
}
#endif // QT_NO_DEBUG
