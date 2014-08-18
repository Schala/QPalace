#include <QDateTime>

#ifndef SERVER
#include <QException>
#include <QtMath>
#endif // SERVER

#include "room.hpp"

#ifdef SERVER
#include <QJsonArray>
#include <QSqlQuery>
#include <QSqlRecord>

QPRoom::QPRoom(QObject *parent, qint16 id): QObject(parent)
{
	mId = id--;

	QSqlQuery q;
	q.exec("SELECT * FROM room");
	if (!q.seek(id))
		qWarning("Malformed SQL query for room %d record retrieval!", id);
	else
	{
		QSqlRecord rec = q.record();
		mName = rec.value("name").toString().toLatin1();
		mFlags = (qint32)rec.value("flags").toInt();
		mFaces = (qint32)rec.value("faces").toInt();
		mImgName = rec.value("img_name").toString().toLatin1();
		mArtistName = rec.value("artist_name").toString().toLatin1();
		mPwd = rec.value("password").toByteArray();
		
		QJsonArray hotspots = rec.value("hotspots").toJsonArray();
		for (qint16 i = 0; i < hotspots.size(); i++)
		{
			mHotspots.append(QPHotspot()); // compiler problem workaround
			mHotspots[i] = QPHotspot(hotspots[i].toObject());
		}
		
		QJsonArray lprops = rec.value("loose_props").toJsonArray();
		for (qint16 i = 0; i < lprops.size(); i++)
		{
			QJsonObject lprop = lprops[i].toObject();
			mLProps.append(QPLooseProp());
			mLProps[i].prop.id = (qint32)lprop["propId"].toInt();
			mLProps[i].prop.crc = (qint32)lprop["propCrc"].toInt();
			mLProps[i].flags = (qint32)lprop["flags"].toInt();
			mLProps[i].ref = (qint32)lprop["ref"].toInt();
			mLProps[i].location.x = (qint16)lprop["x"].toInt();
			mLProps[i].location.y = (qint16)lprop["y"].toInt();
		}
		
		QJsonArray imgs = rec.value("images").toJsonArray();
		for (qint16 i = 0; i < imgs.size(); i++)
		{
			QJsonObject img = imgs[i].toObject();
			mImages.append(QPImage());
			mImages[i].id = (qint16)img["id"].toInt();
			mImages[i].alpha = (qint16)img["alpha"].toInt();
		}
		
		QJsonArray scripts = rec.value("scripts").toJsonArray();
		for (qint16 i = 0; i < scripts.size(); i++)
			mScripts.append(scripts[i].toString().toLatin1());
		
		QJsonArray spotNames = rec.value("spot_names").toJsonArray();
		for (qint16 i = 0; i < spotNames.size(); i++)
			mSpotNames.append(spotNames[i].toString().toLatin1());
		
		QJsonArray points = rec.value("points").toJsonArray();
		for (qint16 i = 0; i < points.size(); i++)
		{
			QJsonObject ploc = points[i].toObject();
			mPoints.append(QPPoint());
			mPoints[i].x = (qint16)ploc["x"].toInt();
			mPoints[i].y = (qint16)ploc["y"].toInt();
		}
		
		QJsonArray states = rec.value("states").toJsonArray();
		for (qint16 i = 0; i < states.size(); i++)
		{
			QJsonObject srec = states[i].toObject();
			mStates.append(QPSpotState());
			mStates[i].imgId = (qint16)srec["imgId"].toInt();
			QJsonObject sloc = srec["location"].toObject();
			mStates[i].location.x = (qint16)sloc["x"].toInt();
			mStates[i].location.y = (qint16)sloc["y"].toInt();
		}
		
		QSqlQuery q2;
		q2.exec("SELECT * FROM image");
		for (auto i: mImages)
		{
			if (!q2.seek(i.id))
				qWarning("Image ID %d for room %d was not found!", i.id, id);
			else
				mImgNames.append(q2.record().value("filename").toString().toLatin1());
		}
	}
}

QPHotspot::QPHotspot(const QJsonObject &data)
{
	mScriptEventMask = (qint32)data["scriptEventMask"].toInt();
	mFlags = (qint32)data["flags"].toInt();
	mType = (qint16)data["type"].toInt();
	mDest = (qint16)data["destination"].toInt();
	mState = (qint16)data["state"].toInt();
	mNamePtr = (qint16)data["nameIndex"].toInt();
	QJsonObject loc = data["location"].toObject();
	mLoc.x = (qint16)loc["x"].toInt();
	mLoc.y = (qint16)loc["y"].toInt();
	
	QJsonArray pointIndices = data["pointIndices"].toArray();
	for (qint16 i = 0; i < pointIndices.size(); i++)
		mPointPtrs.append((qint16)pointIndices[i].toInt());
	
	QJsonArray scriptIndices = data["scriptIndices"].toArray();
	for (qint16 i = 0; i < scriptIndices.size(); i++)
		mScriptPtrs.append((qint16)scriptIndices[i].toInt());
	
	QJsonArray stateIndices = data["stateIndices"].toArray();
	for (qint16 i = 0; i < stateIndices.size(); i++)
		mStatePtrs.append((qint16)stateIndices[i].toInt());
}

void QPRoom::description(QPConnection *c, bool revised) const
{
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	//qint16 lpropArraySize = mLProps.size()*24;
	qint16 imgArraySize = mImages.size()*12;
	qint16 spotArraySize = mHotspots.size()*48;
	qint16 pointArraySize = mPoints.size()*4;
	qint16 stateArraySize = mStates.size()*6;
	
	qint16 scriptArraySize = 0;
	for (auto script: mScripts)
		scriptArraySize += script.size()+1;
	
	qint16 imgNamesArraySize = 0;
	for (auto iname: mImgNames)
		imgNamesArraySize += iname.size()+1;
	
	qint16 spotNamesArraySize = 0;
	for (auto sname: mSpotNames)
		spotNamesArraySize += sname.size()+1;

	qint16 drawArraySize = 0;
	for (auto d: mDraws)
		drawArraySize += d.size();
	
	qint16 pos = 2;
	
	qint16 namePos = (bool)mName.size() ? pos : 0; // isEmpty() gives 'const bool' error *bool*shit
	pos += namePos ? mName.size()+1 : 0;
	qint16 imgNamePos = (bool)mImgName.size() ? pos : 0;
	pos += imgNamePos ? mImgName.size()+1 : 0;
	qint16 artistNamePos = (bool)mArtistName.size() ? pos : 0;
	pos += artistNamePos ? mArtistName.size()+1 : 0;
	qint16 pwPos = (bool)mPwd.size() ? pos : 0;
	pos += pwPos ? mPwd.size()+1 : 0;
	qint16 imgNamesPos = (bool)mImgNames.size() ? pos : 0;
	pos += imgNamesPos ? imgNamesArraySize : 0;
	qint16 spotNamePos = (bool)mSpotNames.size() ? pos : 0;
	pos += spotNamePos ? spotNamesArraySize : 0;
	qint16 statePos = (bool)mStates.size() ? pos : 0;
	pos += statePos ? stateArraySize : 0;
	qint16 pointPos = (bool)mPoints.size() ? pos : 0;
	pos += pointPos ? pointArraySize : 0;
	qint16 scriptPos = (bool)mScripts.size() ? pos : 0;
	pos += scriptPos ? scriptArraySize : 0;
	qint16 imgPos = (bool)mImages.size() ? pos : 0;
	pos += imgPos ? imgArraySize : 0;
	qint16 spotPos = (bool)mHotspots.size() ? pos : 0;
	pos += spotPos ? spotArraySize : 0;
	qint16 draw1Pos = (bool)mDraws.size() ? pos : 0;
	pos += draw1Pos ? drawArraySize : 0;
	qint16 lProp1Pos = (bool)mLProps.size() ? pos : 0;
	
	ds1 << mFlags << mFaces << mId << namePos << imgNamePos << artistNamePos << pwPos <<
		(qint16)mHotspots.size() << spotPos << (qint16)mImages.size() << imgPos << (qint16)mDraws.size() << draw1Pos <<
		(qint16)mConnections.size() << (qint16)mLProps.size() << lProp1Pos << (quint16)0;
	
	QByteArray buf;
	QDataStream ds2(&buf, QIODevice::WriteOnly);
	ds2.device()->reset();
	ds2.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds2 << (quint16)0;
	
	qint16 *imgNamePtrs = new qint16[mImgNames.size()];
	qint16 *spotNamePtrs = new qint16[mSpotNames.size()];
	qint16 *statePtrs = new qint16[mStates.size()];
	qint16 *pointPtrs = new qint16[mPoints.size()];
	qint16 *scriptTextPtrs = new qint16[mScripts.size()];
	
	if (namePos)
	{
		ds2 << (quint8)mName.size();
		ds2.writeRawData(mName.data(), mName.size());
	}
	
	if (imgNamePos)
	{
		ds2 << (quint8)mImgName.size();
		ds2.writeRawData(mImgName.data(), mImgName.size());
	}
	
	if (artistNamePos)
	{
		ds2 << (quint8)mArtistName.size();
		ds2.writeRawData(mArtistName.data(), mArtistName.size());
	}
	
	if (pwPos)
	{
		ds2 << (quint8)mPwd.size();
		ds2.writeRawData(mPwd.data(), mPwd.size());
	}
	
	if (imgNamesPos)
	{
		for (qint16 i = 0; i < mImgNames.size(); i++)
		{
			imgNamePtrs[i] = (qint16)ds2.device()->pos();
			ds2 << (quint8)mImgNames[i].size();
			ds2.writeRawData(mImgNames[i].data(), mImgNames[i].size());
		}
	}
	
	if (spotNamePos)
	{
		for (qint16 i = 0; i < mSpotNames.size(); i++)
		{
			spotNamePtrs[i] = (qint16)ds2.device()->pos();
			ds2 << (quint8)mSpotNames[i].size();
			ds2.writeRawData(mSpotNames[i].data(), mSpotNames[i].size());
		}
	}
	
	if (statePos)
	{
		for (qint16 i = 0; i < mStates.size(); i++)
		{
			statePtrs[i] = (qint16)ds2.device()->pos();
			ds2 << mStates[i].imgId << mStates[i].location.x << mStates[i].location.y;
		}
	}
	
	if (pointPos)
	{
		for (qint16 i = 0; i < mPoints.size(); i++)
		{
			pointPtrs[i] = (qint16)ds2.device()->pos();
			ds2 << mPoints[i].x << mPoints[i].y;
		}
	}
	
	if (scriptPos)
	{
		for (qint16 i = 0; i < mScripts.size(); i++)
		{
			scriptTextPtrs[i] = (qint16)ds2.device()->pos();
			ds2.writeRawData(mScripts[i].data(), mScripts[i].size());
			ds2 << (quint8)0;
		}
	}
	
	if (imgPos)
		for (qint16 i = 0; i < mImages.size(); i++)
			ds2 << (qint32)0 << mImages[i].id << imgNamePtrs[i] << mImages[i].alpha << (qint16)0;
	
	if (spotPos)
	{
		qint16 j = pointPtrs[0], k = statePtrs[0], l = scriptTextPtrs[0];
		for (qint16 i = 0; i < mHotspots.size(); i++)
		{
			if (i > pointPtrs[0]) j += (qint16)mHotspots[i].mPointPtrs.size();
			if (i > statePtrs[0]) k += (qint16)mHotspots[i].mStatePtrs.size();
			if (i > scriptTextPtrs[0]) l += (qint16)mHotspots[i].mScriptPtrs.size();
			
			ds2 << mHotspots[i].mScriptEventMask << mHotspots[i].mFlags << (qint64)0 << mHotspots[i].mLoc.x <<
				mHotspots[i].mLoc.y << i << mHotspots[i].mDest << (qint16)mHotspots[i].mPointPtrs.size() << j <<
				mHotspots[i].mType << (qint16)0 << (qint16)mHotspots[i].mScriptPtrs.size() << (qint16)0 <<
				mHotspots[i].mState << (qint16)mHotspots[i].mStatePtrs.size() << k << spotNamePtrs[mHotspots[i].mNamePtr] <<
				l << (qint16)0;
		}
	}

	if (draw1Pos)
		for (auto d: mDraws)
			ds2.writeRawData(d.constData(), d.size());

	if (lProp1Pos)
		for (auto lp: mLProps)
			ds2 << (qint32)0 << lp.prop.id << lp.prop.crc << lp.flags << lp.ref << lp.location.x << lp.location.y;
	
	ds1 << (quint16)buf.size();
	ba += buf;
	QPMessage msg = revised ? QPMessage(QPMessage::sRom) : QPMessage(QPMessage::room);
	msg = ba;
	QDataStream ds(c->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << msg;
	delete[] imgNamePtrs;
	delete[] statePtrs;
	delete[] pointPtrs;
	delete[] scriptTextPtrs;
	delete[] spotNamePtrs;
}

void QPRoom::users(QPConnection *c) const
{
	QPMessage rprs(QPMessage::rprs, population());

	for (auto p: mConnections)
		rprs << p;

	QPMessage endr(QPMessage::endr);
	QDataStream ds(c->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << rprs << endr;
}

#else

void QPRoom::description(QPMessage &msg)
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

QDataStream& operator<<(QDataStream &ds, const QPDraw *draw)
{
	QByteArray dbuf;
	QDataStream ds1(&dbuf, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

	if ((draw->mCmd != QPDraw::Delete) || (draw->mCmd != QPDraw::Detonate))
	{
		ds1 << draw->mPenSize << (qint16)draw->mPolygon.size()-1;

		// duplicate values for compatibility
		ds1 << (qint8)(((draw->mLineClr & 0xff0000) >> 16) & 0xff); // red
		ds1 << (qint8)(((draw->mLineClr & 0xff0000) >> 16) & 0xff); // red
		ds1 << (qint8)(((draw->mLineClr & 0xff00) >> 8) & 0xff); // green
		ds1 << (qint8)(((draw->mLineClr & 0xff00) >> 8) & 0xff); // green
		ds1 << (qint8)(draw->mLineClr & 0xff); // blue
		ds1 << (qint8)(draw->mLineClr & 0xff); // blue

		for (auto p: draw->mPolygon)
			ds1 << p.y << p.x;

		ds1 << (qint8)qCeil(draw->mLineAlpha * 0xff);
		ds1 << (qint8)(((draw->mLineClr & 0xff0000) >> 16) & 0xff); // red
		ds1 << (qint8)(((draw->mLineClr & 0xff00) >> 8) & 0xff); // green
		ds1 << (qint8)(draw->mLineClr & 0xff); // blue

		ds1 << (qint8)qCeil(draw->mFillAlpha * 0xff);
		ds1 << (qint8)(((draw->mFillClr & 0xff0000) >> 16) & 0xff); // red
		ds1 << (qint8)(((draw->mFillClr & 0xff00) >> 8) & 0xff); // green
		ds1 << (qint8)(draw->mFillClr & 0xff); // blue
	}

	qint16 encCmd = (draw->mCmd | (draw->mFlags << 8));
	ds << (qint32)0 << encCmd << (qint16)dbuf.size() << (qint16)0;
	ds.writeRawData(dbuf.constData(), dbuf.size());
	return ds;
}


QDataStream& operator>>(QDataStream &ds, QPDraw *draw)
{
	quint16 size, points;
	quint8 r, g, b;

	ds.skipRawData(4); // unused
	ds >> draw->mCmd >> size;
	ds.skipRawData(2); // unneeded offset start since this is streamed

	draw->mFlags = draw->mCmd >> 8;
	draw->mCmd &= 0xff;

	if ((draw->mCmd == QPDraw::Delete) || (draw->mCmd == QPDraw::Detonate))
		return ds;

	QByteArray dbuf;
	dbuf.resize(size);
	ds.readRawData(dbuf.data(), size);
	QDataStream ds1(&dbuf, QIODevice::ReadOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

	/*if (!ds1.device()->bytesAvailable() < size)
	{
		qWarning("[%s] Corrupt draw packet encountered!", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")));
		return ds;
	}*/

	ds1 >> draw->mPenSize >> points;

	// duplicate values for compatibility
	ds1 >> r;
	ds1.skipRawData(1);
	ds1 >> g;
	ds1.skipRawData(1);
	ds1 >> b;
	ds1.skipRawData(1);

	draw->mLineClr = draw->mFillClr = draw->mPenClr = argbToUint(0xff, r, g, b);
	draw->mLineAlpha = draw->mFillAlpha = draw->mPenAlpha = 1;

	for (quint16 i = 0; i <= points; i++)
	{
		draw->mPolygon.append(QPPoint());
		ds1 >> draw->mPolygon[i].y >> draw->mPolygon[i].x;
	}

	if (ds1.device()->bytesAvailable())
	{
		try
		{
			quint8 alphaByte;
			qreal alpha;

			ds1 >> alphaByte >> r >> g >> b;
			alpha = (qreal)alphaByte / 0xff;
			draw->mLineClr = argbToUint(alphaByte, r, g, b);
			draw->mLineAlpha = alpha;

			ds1 >> alphaByte >> r >> g >> b;
			alpha = (qreal)alphaByte / 0xff;
			draw->mFillClr = argbToUint(alphaByte, r, g, b);
			draw->mFillAlpha = alpha;
		}
		catch (QException &e)
		{
			// fallback to old behavior of using mPenClr for everything
			draw->mFillClr = draw->mLineClr = draw->mPenClr;
			draw->mFillAlpha = draw->mLineAlpha = draw->mPenAlpha;
			draw->mPenSize = 0;
		}
	}
	else
	{
		draw->mFillClr = draw->mLineClr = draw->mPenClr;
		draw->mFillAlpha = draw->mLineAlpha = draw->mPenAlpha;
		if ((draw->mFlags & QPDraw::Ellipsed) || (draw->mFlags & QPDraw::Fill))
			// no more packets, must be PChat 3
			draw->mPenSize = 0;
	}

	return ds;
}

#endif // SERVER

QPRoom::~QPRoom()
{
	if (!mConnections.empty())
		mConnections.clear();
	if (!mHotspots.empty())
		mHotspots.clear();
	if (!mImages.empty())
		mImages.clear();
	if (!mLProps.empty())
		mLProps.clear();
	if (!mPoints.empty())
		mPoints.clear();
	if (!mStates.empty())
		mStates.clear();
	if (!mScripts.empty())
		mScripts.clear();
	if (!mImgNames.empty())
		mImgNames.clear();
	if (!mSpotNames.empty())
		mSpotNames.clear();
	if (!mDraws.empty())
		mDraws.clear();
}

QPHotspot::~QPHotspot()
{
	if (!mPointPtrs.empty())
		mPointPtrs.clear();
	if (!mStatePtrs.empty())
		mStatePtrs.clear();
	if (!mScriptPtrs.empty())
		mScriptPtrs.clear();
}

void QPRoom::handleUserJoined(const QPRoom *r, QPConnection *c)
{
	if (this == r)
	{
		mConnections.append(c);
		QPMessage nprs(QPMessage::nprs, c->id());
		nprs << c;

		for (auto p: mConnections)
		{
			QDataStream ds(p->socket());
			ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
			ds << nprs;

			/*if (p != c)
				description(p, true);*/
		}
	}
}

void QPRoom::handleUserLeft(const QPRoom *r, QPConnection *c)
{
	if (this == r)
	{
		mConnections.remove(mConnections.indexOf(c));
		QPMessage eprs(QPMessage::eprs, c->id());
		delete c;

		for (auto p: mConnections)
		{
			QDataStream ds(p->socket());
			ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
			ds << eprs;
			description(p, true);
		}
	}
}

void QPRoom::handleUserMoved(const QPRoom *r, const QPConnection *c)
{
	if (this == r)
	{
		QPMessage msg(QPMessage::uLoc, c->id());
		QByteArray ba;
		QDataStream ds1(&ba, QIODevice::WriteOnly);
		ds1.device()->reset();
		ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

		ds1 << c->position().x << c->position().y;
		msg = ba;

		for (auto p: mConnections)
		{
			QDataStream ds(p->socket());
			ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
			ds << msg;
		}
	}
}

void QPRoom::handleBlowThru(const QPRoom *r, QPBlowThru *blow)
{
	if (this == r)
	{
		QPMessage msg(QPMessage::blow);
		msg << blow;

		for (auto c: mConnections)
		{
			QDataStream ds(c->socket());
			ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
			ds << msg;
		}

		delete blow;
	}
}

void QPRoom::handleUserTalked(const QPRoom *r, QPMessage *msg)
{
	if (this == r)
	{
		for (auto p: mConnections)
		{
			QDataStream ds(p->socket());
			ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
			ds << *msg;
		}
		delete msg;
	}
}

void QPRoom::handleUserDrew(const QPRoom *r, const QPMessage &msg)
{
	if (this == r)
	{
		if ((msg.data(4) == 4) && !mDraws.empty()) // cmd == delete
			mDraws.removeLast();
		else if ((msg.data(4) == 3) && !mDraws.empty()) // cmd == detonate
			mDraws.clear();
		else
			mDraws.append(QByteArray(msg.data(), msg.size()));

		for (auto p: mConnections)
		{
			QDataStream ds(p->socket());
			ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
			ds << msg;
		}
	}
}
