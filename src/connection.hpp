#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <QByteArray>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QtGlobal>
#ifndef QT_NO_DEBUG
#include <QString>
#endif // QT_NO_DEBUG

#include "message.hpp"
#include "shared.hpp"

#ifndef SERVER
#include "client/registration.hpp"
#endif // SERVER

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
		LinuxX86,
		LinuxX86_64,
		LinuxARM,
		AndroidARMv5,
		AndroidARMv7,
		AndroidX86,
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
	QPConnection(QTcpSocket *sock);
	inline Vendor vendor() const { return mVendor; }
	static Vendor vendorFromString(const char *str);
	static const char* vendorToString(Vendor vendor);
	bool isSecureVendor() const;
	inline void setVendor(Vendor v) { mVendor = v; }
	inline void setWizardPassword(const char *pwd, quint8 l) { if (pwd) mWizardPwd = QByteArray(pwd, l); }
	inline void setAuxFlags(qint32 flags) { mAuxFlags = flags; }
	inline void setRoom(qint16 id) { mRoom = id; }
	inline void setId(qint32 id) { mId = id; }
	const char* osToString() const;
#else
	QPConnection(QTcpSocket *sock, const char *name, const QPRegistration &reg, AuxFlags auxFlags,
		const QPRegistration &uid, qint16 initRoom, const char *wizpwd = nullptr);
	inline QPRegistration registration() const { return mReg; }
	inline QPRegistration pseudoId() const { return mUid; }
#endif // SERVER
	~QPConnection();
	inline QTcpSocket* socket() const { return mSocket; }
	inline qint32 auxFlags() const { return mAuxFlags; }
	inline const char* userName() const { return mUserName.data(); }
	inline void setUserName(const char *username) { if (username) mUserName = QByteArray(username); }
	inline const char* wizardPassword() const { return mWizardPwd; }
	inline qint16 status() const { return mStatus; }
	inline void setStatus(qint16 flags) { mStatus = flags; }
	inline qint16 room() const { return mRoom; }
	inline qint32 id() const { return mId; }
	inline qint16 face() const { return mFace; }
	inline void setFace(qint16 f) { if (f < 16) mFace = f; }
	inline QPPoint position() const { return mPos; }
	inline void setPosition(qint16 x, qint16 y) { mPos.x = x; mPos.y = y; }
	inline QPAssetSpec prop(quint8 i) const { return mProps[i]; }
	inline void setProp(quint8 i, const QPAssetSpec &p) { mProps[i] = p; }
	inline qint16 color() const { return mColor; }
	inline void setColor(qint16 c) { if (c < 16) mColor = c; }
private:
#ifndef SERVER
	QPRegistration mReg, mUid;
#endif // SERVER
	QTcpSocket *mSocket;
	QPAssetSpec *mProps;
	QByteArray mUserName, mWizardPwd;
	QPPoint mPos;
	qint32 mAuxFlags, mId;
	qint16 mStatus, mRoom, mFace, mColor;
	Vendor mVendor;
#ifndef QT_NO_DEBUG
	const char *mRawVendor;

public:
	QString loginDebugInfo() const;
	inline void setRawVendor(const char *v) { mRawVendor = v; }
#endif // QT_NO_DEBUG
};

typedef QSharedPointer<QPConnection> QPConnectionPtr;

#endif // _CONNECTION_H
