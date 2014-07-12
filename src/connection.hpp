#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <QByteArray>
//#include <QQueue>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QtGlobal>
#ifndef NDEBUG
#include <QString>
#endif // NDEBUG

#include "message.hpp"
#include "registration.hpp"

class QPConnection final
{
public:
	enum
	{
		UnknownMachine = 0,
		Mac68k,
		MacPPC,
		Win16,
		Win32,
		Java,
		MacIntel,
		Win64,
		Linux32,
		Linux64,
		LinuxARM,
		AndroidARMv5,
		AndroidARMv7,
		AndroidIntel,
		Dalvik,
		iOS,
		iOSSim,
		WinRT,
		CLR,
		Python,
		Ruby,
		Flash,
		Web,
		OSMask = 0x000000ff,
		Authenticate = 0x80000000
	};
	enum class Vendor: quint8
	{
		Unknown = 0,
		PalaceViewer, // LinPal shares this
		InstantPalace,
		Phalanx,
		PalaceChat,
		OpenPalace,
		QPalace = 0xff
	};
	enum
	{
		SuperUser = 0x0001,
		God,
		Kill = 0x0004,
		CommError = 0x0040,
		Gag = 0x0080,
		Pin = 0x0100,
		Hide = 0x0200,
		RejectEsp = 0x0400,
		RejectPrivate = 0x0800,
		PropGag = 0x1000
	};
#ifdef SERVER
	QPConnection(QTcpSocket *sock): mSocket(sock) {}
	inline Vendor vendor() const { return mVendor; }
	static Vendor vendorFromString(const char *str);
	static const char* vendorToString(Vendor vendor);
	bool isSecureVendor() const;
	inline void setRegistration(const QPRegistration &reg) { mReg = reg; }
	inline void setPseudoId(const QPRegistration &uid) { mUid = uid; }
	inline void setVendor(Vendor v) { mVendor = v; }
	inline void setWizardPassword(const char *pwd, quint8 l) { if (pwd) mWizardPwd = QByteArray(pwd, l); }
	inline void setAuxFlags(qint32 flags) { mAuxFlags = flags; }
#else
	QPConnection(QTcpSocket *sock, const char *name, const QPRegistration &reg, AuxFlags auxFlags,
		const QPRegistration &uid, qint16 initRoom, const char *wizpwd = nullptr);
#endif // SERVER
	~QPConnection();
	inline QTcpSocket* socket() const { return mSocket; }
	inline qint32 auxFlags() const { return mAuxFlags; }
	inline QPRegistration registration() const { return mReg; }
	inline QPRegistration pseudoId() const { return mUid; }
	inline const char* userName() const { return mUserName.data(); }
	inline void setUserName(const char *username) { if (username) mUserName = QByteArray(username); }
	inline const char* wizardPassword() const { return mWizardPwd; }
	inline qint16 status() const { return mStatus; }
	inline void setStatus(qint16 flags) { mStatus = flags; }
private:
	//QQueue<QPMessage*> mMsgQueue;
	QPRegistration mReg, mUid;
	QTcpSocket *mSocket;
	//QPRoomPtr mRoom;
	QByteArray mUserName, mWizardPwd;
	qint32 mAuxFlags;
	qint16 mStatus;
	Vendor mVendor;
#ifndef NDEBUG
	const char *mRawVendor;

public:
	QString debugInfo() const;
	inline void setRawVendor(const char *v) { mRawVendor = v; }
#endif // NDEBUG
};

typedef QSharedPointer<QPConnection> QPConnectionPtr;

#endif // _CONNECTION_H
