#include <QDataStream>
#include "room.hpp"

QPRoom::~QPRoom()
{
	if (!mConnections.empty())
		mConnections.clear();
	if (!mHotspots.empty())
	{
		for (auto hs: mHotspots)
			delete hs;
		mHotspots.clear();
	}
	if (!mLProps.empty())
	{
		for (auto lp: mLProps)
			delete lp;
		mLProps.clear();
	}
}

QPMessage& description() const
{
	QByteArray ba;
	QDataStream ds(ba);
	qint16 imgNamePos = 3 + mName.size();
	qint16 artistNamePos = imgNamePos + 1 + mImgName.size();
	qint16 pwPos, nSpots, spotPos, nImgs, imgPos nDraws, draw1Pos, nUsers, nLProps, lProp1Pos;
	
	ds.device()->reset();
	ds << mFlags << mFacesId << mId << (qint16)2 << imgNamePos << artistNamePos << pwPos <<
		(qint16)mHotspots.size(); << spotPos << nImgs << draw1Pos << (qint16)mConnections.size() <<
		(qint16)mLProps.size() << lProp1Pos;
	
	QPMessage msg(QPMessage::Id::room);
	msg.setData(ba.data());
	return msg;
}

void QPRoom::setDescription(QPMessage &msg)
{
	QDataStream buf(QByteArray(msg.data());
	qint16 namePos, imgNamePos, artistNamePos, pwPos, nSpots, spotPos, nImgs, imgPos,
		nDraws, draw1Pos, nUsers, nLProps, lProp1Pos;
	quint16 len;
	
	buf.device()->reset();
	buf >> mFlags >> mFacesId >> mId >> namePos >> imgNamePos >> artistNamePos >> pwPos >>
		nSpots >> spotPos >> nImgs >> draw1Pos >> nUsers >> nLProps >> lProp1Pos;
	buf.skipRawData(2); // padding
	buf >> len;
	
	QByteArray ba(len, 0);
	buf.readRawData(ba.data(), len);
	QDataStream ds(ba);
	quint8 sLen;
	ds.device()->reset();
	ds.skipRawData(namePos);
	
	if (namePos)
	{
		ds >> sLen;
		ds.readRawData(mName.data(), sLen);
	}
	
	if (imgNamePos)
	{
		ds >> sLen;
		ds.readRawData(mImgName.data(), sLen);
	}
	
	if (artistNamePos)
	{
		ds >> sLen;
		ds.readRawData(mArtistName.data(), sLen);
	}
}
