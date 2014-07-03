#include "room.hpp"

/*QPRoom::~QPRoom() {
	if (!mConnections.empty())
		mConnections.clear();
	if (!mHotspots.empty()) {
		for (auto hs: mHotspots)
			delete hs;
		mHotspots.clear();
	}
	if (!mLProps.empty()) {
		for (auto lp: mLProps)
			delete lp;
		mLProps.clear();
	}
}

#ifdef SERVER
#else
void QPRoom::description(QDataStream &buf) {
	qint16 namePos, imgNamePos, artistNamePos, pwPos, nSpots, spotPos, nImgs, imgPos,
		nDraws, draw1Pos, nUsers, nLProps, lProp1Pos;
	quint16 len;
	
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
	
	if (namePos) {
		ds >> sLen;
		ds.readRawData(mName.data(), sLen);
	}
	
	if (imgNamePos) {
		ds >> sLen;
		ds.readRawData(mImgName.data(), sLen);
	}
	
	if (artistNamePos) {
		ds >> sLen;
		ds.readRawData(mArtistName.data(), sLen);
	}
}
#endif // SERVER*/
