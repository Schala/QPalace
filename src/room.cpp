#include "room.hpp"

QPRoom::~QPRoom() {
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
	short namePos, imgNamePos, artistNamePos, pwPos, nSpots, spotPos, nImgs, imgPos,
		nDraws, draw1Pos, nUsers, nLProps, lProp1Pos;
	ushort len;
	
	buf >> flags >> faces_id >> id >> namePos >> imgNamePos >> artistNamePos >> pwPos >>
		nSpots >> spotPos >> nImgs >> draw1Pos >> nUsers >> nLProps >> lProp1Pos;
	buf.skipRawData(2); // padding
	buf >> len;
	
	QByteArray ba(len, 0);
	buf.readRawData(ba.data(), len);
	QDataStream ds(ba);
	uchar sLen;
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
#endif // SERVER
