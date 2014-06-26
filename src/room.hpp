#ifndef _ROOM_H
#define _ROOM_H

#include <QByteArray>
#include <QDataStream>
#include <QFlags>
#include <QHash>
#include <QSet>
#include <QSharedData>
#include <QSharedDataPointer>

struct QPPoint final {
	short x, y;
};

class QPHotspot final {
public:
	enum {
		Normal = 0,
		Door,
		ShutableDoor,
		LockableDoor,
		Bolt,
		NavArea
	};
	enum ScriptEvent {
		Select = 0x00000001,
		Lock = 0x00000002,
		Unlock = 0x00000004,
		Hide = 0x00000008,
		Show = 0x00000010,
		Startup = 0x00000020,
		Alarm = 0x00000040,
		Custom = 0x00000080,
		InChat = 0x00000100,
		PropChange = 0x00000200,
		Enter = 0x00000400,
		Leave = 0x00000800,
		OutChat = 0x00001000,
		SignOn = 0x00002000,
		SignOff = 0x00004000,
		Macro0 = 0x00008000,
		Macro1 = 0x00010000,
		Macro2 = 0x00020000,
		Macro3 = 0x00040000,
		Macro4 = 0x00080000,
		Macro5 = 0x00100000,
		Macro6 = 0x00200000,
		Macro7 = 0x00400000,
		Macro8 = 0x00800000,
		Macro9 = 0x01000000
	};
	Q_DECLARE_FLAGS(ScriptEvents, ScriptEvent)
#ifdef SERVER
	QPHotspot(short room_id, short id);
#else
	QPHotspot(QDataStream &buf);
#endif // SERVER
	~QPHotspot();
private:
	int mScriptEventMask, mFlags;
	QPPoint mLoc;
	short mID, mDest, mType, mState;
	QHash<short, QPPoint> mImgStates; // id, loc
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QPHotspot::ScriptEvents)

struct QPLooseProp final {
	int link;
	QHash<int, int> spec; // id, crc
	int flags, clientArbitrary;
	QPPoint location;
}

class QPConnection final: public QSharedData;
typedef QSharedDataPointer<QPConnection> QPConnectionPtr;

class QPRoom final: virtual public QSharedData, virtual public QObject {
	Q_OBJECT
public:
	enum Flag {
		AuthorLocked = 0x0001,
		Private = 0x0002,
		NoPainting = 0x0004,
		Closed = 0x0008,
		NoCyborgs = 0x0010,
		Hidden = 0x0020,
		NoGuests = 0x0040,
		WizardsOnly = 0x0080,
		DropZone = 0x0100
	};
	Q_DECLARE_FLAGS(Flags, Flag)
#ifdef SERVER
	QPRoom(short id);
	QByteArray& description() const;
#else
	inline QPRoom(QDataStream &buf) {
		description(buf);
	};
	void description(QDataStream &buf);
#endif // SERVER
	~QPRoom();
private:
	QSet<QPConnectionPtr> mConnections;
	int mFlags, mFacesID;
	short mID;
	QSet<QPHotspot*> mHotspots;
	QHash<short, short> mImages; // id, alpha
	QSet<QPLooseProp*> mLProps;
	QByteArray mName, mImgName, mArtistName, mPwd;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QPRoom::Flags)
typedef QSharedDataPointer<QPRoom> QPRoomPtr;

#endif // _ROOM_H
