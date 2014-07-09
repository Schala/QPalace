#include <QByteArray>
#include "message.hpp"

void QPMessage::setData(const char *buf)
{
	if (!buf)
	{
		if (mData)
			delete[] mData;
		mData = nullptr;
		mSize = 0;
	}
	else
	{
		QByteArray ba(buf);
		if (mData)
			delete[] mData;
		mData = ba.data();
		mSize = ba.size();
	}
}

QDataStream& operator<<(QDataStream &out, const QPMessage &msg)
{
	out << msg.id() << msg.size() << msg.ref();
	if (msg.size() != 0)
		out.writeRawData(msg.data(), msg.size());
	return out;
}

QDataStream& operator>>(QDataStream &in, QPMessage &msg)
{
	in >> msg.mId >> msg.mSize >> msg.mRef;
	if (msg.data())
		delete[] msg.mData;
	msg.mData = nullptr;
	if (msg.size() != 0)
	{
		msg.mData = new char[msg.size()];
		in.readRawData(msg.mData, msg.size());
	}
	return in;
}
