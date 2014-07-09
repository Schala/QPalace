#ifndef _ROOM_H
#define _ROOM_H

#include <QByteArray>
#include <QFlags>
#include <QHash>
#include <QSet>
#include <QSharedPointer>
#include <QtGlobal>

#include "message.hpp"
#include "connection.hpp"

struct QPPoint final
{
	qint16 x, y;
};

class QPHotspot final
{
/*public:
	enum
	{
		Normal = 0,
		Door,
		ShutableDoor,
		LockableDoor,
		Bolt,
		NavArea
	};
	enum ScriptEvent
	{
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
	QPHotspot(qint16 room_id, qint16 id);
#else
	QPHotspot(QDataStream &buf);
#endif // SERVER
	~QPHotspot();
private:
	qint32 mScriptEventMask, mFlags;
	QPPoint mLoc;
	qint16 mId, mDest, mType, mState;
	QHash<qint16, QPPoint> mImgStates; // id, loc*/
};

//Q_DECLARE_OPERATORS_FOR_FLAGS(QPHotspot::ScriptEvents)

struct QPLooseProp final
{
	qint32 link;
	QHash<qint32, qint32> spec; // id, crc
	qint32 flags, clientArbitrary;
	QPPoint location;
}

class QPRoom final
{
public:
	enum Flag
	{
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
	QPRoom(qint16 id) {};
	bool load() {return true;};
	bool save() {return true;};
#else
	QPRoom() {};
#endif // SERVER
	QPMessage& description() const;
	void setDescription(QPMessage &msg);
	~QPRoom();
private:
	QSet<QPConnectionPtr> mConnections;
	qint32 mFlags, mFacesId;
	qint16 mId;
	QSet<QPHotspot*> mHotspots;
	QHash<qint16, qint16> mImages; // id, alpha
	QSet<QPLooseProp*> mLProps;
	QByteArray mName, mImgName, mArtistName, mPwd;
};

typedef QSharedPointer<QPRoom> QPRoomPtr;

Q_DECLARE_OPERATORS_FOR_FLAGS(QPRoom::Flags)


#endif // _ROOM_H
