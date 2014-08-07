#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <QDataStream>
#include <QtGlobal>

class QPMessage final
{
	friend QDataStream& operator>>(QDataStream &in, QPMessage &msg);
public:
	enum
	{
		FPSq = 0x46505371, // ?
		HTTP = 0x48545450,
		NOOP = 0x4e4f4f50,
		aAst = 0x61417374, // ?
		auth = 0x61757468,
		autr = 0x61757472,
		blow = 0x626c6f77,
		bye = 0x62796520,
		cLog = 0x634c6f67, // ?
		coLs = 0x636f4c73,
		dPrp = 0x64507270,
		down = 0x646f776e,
		draw = 0x64726177,
		durl = 0x6475726c,
		endr = 0x656e6472,
		eprs = 0x65707273,
		fnfe = 0x666e6665,
		gmsg = 0x676d7367,
		init = 0x696e6974, // ?
		kill = 0x6b696c6c,
		lock = 0x6c6f636b,
		log = 0x6c6f6720,
		mPrp = 0x6d507270,
		nPct = 0x6e506374, // ?
		nPrp = 0x6e507270,
		nRom = 0x6e526f6d,
		navR = 0x6e617652,
		nprs = 0x6e707273,
		opSd = 0x6f705364,
		opSn = 0x6f70536e,
		opSs = 0x6f705373, // ?
		pLoc = 0x704c6f63,
		ping = 0x70696e67,
		pong = 0x706f6e67,
		pser = 0x70736572, // ?
		qAst = 0x71417374,
		qFil = 0x7146696c,
		rAst = 0x72417374,
		rLst = 0x724c7474,
		regi = 0x72656769,
		rep2 = 0x72657032, // legacy, unused in server
		resp = 0x72657370, // ?
		rmsg = 0x726d7367,
		room = 0x726f6f6d,
		rprs = 0x72707273,
		ryit = 0x72796974,
		sAst = 0x73417374,
		sErr = 0x73457272,
		sFil = 0x7346696c,
		sInf = 0x73496e66,
		sPct = 0x73506374, // ?
		sPrp = 0x73507270, // ?
		sRom = 0x73526f6d,
		sSta = 0x73537461,
		sinf = 0x73696e66,
		smsg = 0x736d7367,
		susr = 0x73757372,
		talk = 0x74616c6b,
		timy = 0x74696d79, // ?
		tiyr = 0x74697972,
		uLoc = 0x754c6f63,
		uLst = 0x754c7374,
		uSta = 0x75537461,
		unlo = 0x756e6c6f,
		usrC = 0x75737243,
		usrD,
		usrF = 0x75737246,
		usrN = 0x7573724e,
		usrP = 0x75737250,
		vers = 0x76657273,
		whis = 0x77686973,
		wmsg = 0x776d7367, // ?
		wprs = 0x77707273, // ?
		xtlk = 0x78746c6b,
		xwis = 0x78776973
	};
	
	QPMessage(): mId(NOOP), mRef(0) {}
	QPMessage(quint32 id, qint32 ref = 0): mId(id), mRef(ref) {}
	inline quint32 id() const { return mId; }
	inline quint32 size() const { return (quint32)mData.size(); }
	inline qint32 ref() const { return mRef; }
	inline void setRef(qint32 i) { mRef = i; }
	inline const char* data() const { return mData; }
	inline QPMessage& operator=(const QByteArray &data) { mData = data; return *this; }
#ifndef QT_NO_DEBUG
	void dump() const;
#endif // QT_NO_DEBUG
private:
	QByteArray mData;
	quint32 mId;
	qint32 mRef;
};

QDataStream& operator<<(QDataStream &out, const QPMessage &msg);
QDataStream& operator>>(QDataStream &in, QPMessage &msg);

#endif // _MESSAGE_H
