#ifndef _ROOM_H
#define _ROOM_H

#include <QByteArray>
#include <QDataStream>
#include <QtGlobal>
#include <QVector>

#ifndef QT_NO_DEBUG
#include <QString>
#endif // QT_NO_DEBUG

#ifdef SERVER
#include <QJsonObject>
#endif // SERVER

#include "blowthru.hpp"
#include "message.hpp"
#include "connection.hpp"
#include "shared.hpp"

struct QPSpotState final
{
	QPPoint location;
	qint16 imgId;
};

struct QPScriptEvent final
{
	enum
	{
		Select = 0x1,
		Lock = 0x2,
		Unlock = 0x4,
		Hide = 0x8,
		Show = 0x10,
		Startup = 0x20,
		Alarm = 0x40,
		Custom = 0x80,
		InChat = 0x100,
		PropChange = 0x200,
		Enter = 0x400,
		Leave = 0x800,
		OutChat = 0x1000,
		SignOn = 0x2000,
		SignOff = 0x4000,
		Macro0 = 0x8000,
		Macro1 = 0x10000,
		Macro2 = 0x20000,
		Macro3 = 0x40000,
		Macro4 = 0x80000,
		Macro5 = 0x100000,
		Macro6 = 0x200000,
		Macro7 = 0x400000,
		Macro8 = 0x800000,
		Macro9 = 0x1000000
	};
};

class QPHotspot final
{
	friend class QPRoom;
public:
	enum
	{
		Normal = 0,
		Door,
		ShutableDoor,
		LockableDoor,
		Bolt,
		NavArea
	};
#ifdef SERVER
	QPHotspot() {} // workaround ugly compiler problem
	QPHotspot(const QJsonObject &data);
#else
	QPHotspot(QDataStream &buf);
#endif // SERVER
	~QPHotspot();
private:
	QVector<qint16> mScriptPtrs, mPointPtrs, mStatePtrs;
	qint32 mScriptEventMask, mFlags;
	QPPoint mLoc;
	qint16 mDest, mType, mState, mNamePtr;
};

struct QPLooseProp final
{
	QPAssetSpec prop;
	qint32 flags, ref;
	QPPoint location;
};

struct QPImage final
{
	qint16 id, alpha;
};

class QPDraw final
{
	friend class QPRoom;
	friend QDataStream& operator<<(QDataStream &ds, const QPDraw &draw);
	friend QDataStream& operator>>(QDataStream &ds, QPDraw &draw);
public:
	enum
	{
		Path = 0,
		Shape,
		Text,
		Detonate,
		Delete,
		Ellipse
	};
	enum
	{
		Back = 0,
		Fill = 0x01,
		Ellipsed = 0x40,
		Front = 0x80,
	};
#ifdef SERVER
	QPDraw() {}
	QPDraw(const QJsonObject &data);
#else
#endif // SERVER
private:
	QVector<QPPoint> mPolygon;
	qreal mPenAlpha, mFillAlpha, mLineAlpha;
	qint32 mPenClr, mFillClr, mLineClr;
	qint16 mPenSize, mFlags, mCmd;
};

QDataStream& operator<<(QDataStream &ds, const QPDraw &draw);
QDataStream& operator>>(QDataStream &ds, QPDraw &draw);

class QPRoom final: public QObject
{
	Q_OBJECT
public:
	enum
	{
		AuthorLocked = 0x1,
		Private = 0x2,
		NoPainting = 0x4,
		Closed = 0x8,
		NoCyborgs = 0x10,
		Hidden = 0x20,
		NoGuests = 0x40,
		WizardsOnly = 0x80,
		DropZone = 0x100
	};
#ifdef SERVER
	QPRoom(QObject *parent, qint16 id);
	bool save() {return true;}
	void description(QPConnection *c, bool revised) const;
	void users(QPConnection *c) const;
#else
	QPRoom(): QObject(nullptr) {}
	void description(QPMessage &msg);
#endif // SERVER
	~QPRoom();
	inline void createHotspot(qint16 id, const QPHotspot &p) { mHotspots[id] = p; }
	inline qint16 id() const { return mId; }
	inline qint32 population() const { return mConnections.size(); }
	inline QPConnection* user(qint32 i) const { return mConnections[i]; }
	inline const char* name() const { return mName; }
public slots:
	void handleBlowThru(const QPRoom *r, QPBlowThru *blow);
	void handleUserJoined(const QPRoom *r, QPConnection *c);
	void handleUserLeft(const QPRoom *r, QPConnection *c);
	void handleUserMoved(const QPRoom *r, const QPConnection *c);
	void handleUserTalked(const QPRoom *r, QPMessage &msg);
private:
	QVector<QPConnection*> mConnections;
	QByteArray mName, mImgName, mArtistName, mPwd;
	qint32 mFlags, mFaces;
	qint16 mId;
	QVector<QPHotspot> mHotspots;
	QVector<QPImage> mImages;
	QVector<QPLooseProp> mLProps;
	QVector<QPPoint> mPoints;
	QVector<QPSpotState> mStates;
	QVector<QPDraw> mDraws;
	QVector<QByteArray> mScripts, mImgNames, mSpotNames;
};

#endif // _ROOM_H
