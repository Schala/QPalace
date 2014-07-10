#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <QByteArray>
//#include <QQueue>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QtGlobal>

#include "message.hpp"
#include "client/registration.hpp"

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
#ifdef SERVER
	QPConnection(QTcpSocket *sock): mSocket(sock) {}
	inline Vendor vendor() const { return mVendor; }
	static Vendor vendorFromString(const char *str);
	static const char* vendorToString(Vendor vendor);
	bool isSecureVendor() const;
	inline void setRegistration(const QPRegistration &reg) { mReg = reg; }
	inline void setPseudoId(const QPRegistration &uid) { mUid = uid; }
	inline void setVendor(Vendor v) { mVendor = v; }
	inline void setWizardPassword(const char *pwd) { if (pwd) mWizardPwd = pwd; }
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
	inline void setUserName(const char *username) { if (username) mUserName = username; }
	inline const char* wizardPassword() const { return mWizardPwd; }
private:
	//QPRoomPtr mRoom;
	//QQueue<QPMessage*> mMsgQueue;
	QByteArray mUserName, mWizardPwd;
	QPRegistration mReg, mUid;
	qint32 mAuxFlags;
	QTcpSocket *mSocket;
	Vendor mVendor;
};

typedef QSharedPointer<QPConnection> QPConnectionPtr;

#endif // _CONNECTION_H
