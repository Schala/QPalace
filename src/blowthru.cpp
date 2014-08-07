#include <QDataStream>

#include "blowthru.hpp"

QPBlowThru::QPBlowThru(qint32 relay, qint32 userCount, quint32 tag): mRelay(relay), mUserCount(userCount), mIdCap(0), mTag(tag)
{
	if (userCount > 0)
		mUserIds = new qint32[userCount];
}

QPBlowThru::~QPBlowThru()
{
	if (mUserIds)
		delete[] mUserIds;
}

QPMessage& operator<<(QPMessage &msg, const QPBlowThru *blow)
{
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::WriteOnly);
	ds.device()->reset();
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

#ifndef SERVER
	ds << (quint32)0 << blow->userCount();

	if (blow->userCount() > 0)
		for (quint32 i = 0; i < blow->userCount(); i++)
			ds << blow->user(i);
#endif // SERVER
	ds << blow->tag();
	ds.writeRawData(blow->data(), blow->size());

	msg = ba;
	msg.setRef(blow->relay());
	return msg;
}

QPMessage& operator>>(QPMessage &msg, QPBlowThru *blow)
{
	QByteArray ba(msg.data(), msg.size());
	QDataStream ds(ba);
	ds.device()->reset();
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

#ifdef SERVER
	ds.skipRawData(4); // unused
	ds >> blow->mUserCount;

	if (blow->userCount() > 0)
		for (qint32 i = 0; i < blow->userCount(); i++)
			ds >> blow->mUserIds[i];
#endif // SERVER
	ds >> blow->mTag >> blow->mData;
	blow->mRelay = msg.ref();
	return msg;
}
