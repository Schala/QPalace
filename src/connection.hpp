#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <QByteArray>
#include <QTcpSocket>
#include <QtGlobal>
#ifndef QT_NO_DEBUG
#include <QString>
#endif // QT_NO_DEBUG

#include "message.hpp"
#include "shared.hpp"

#ifndef SERVER
#include "client/userid.hpp"
#endif // SERVER

class QPConnection final: public QObject
{
	Q_OBJECT
public:
	enum
	{
		UnknownMachine = 0,
		Mac68K,
		MacPPC,
		Win16,
		Win32,
		Java,
		MacX86,
		Win64,
		LinuxX86,
		LinuxX86_64,
		Flash,
		OSMask = 0xff,
		Utf8 = 0x100,
		EncodingMask = 0xff00,
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
	enum class Draw: quint8
	{
		None = 0,
		Paint,
		Erase
	};
	enum
	{
		SuperUser = 0x1,
		God,
		Kill = 0x4,
		CommError = 0x40,
		Gag = 0x80,
		Pin = 0x100,
		Hide = 0x200,
		RejectEsp = 0x400,
		RejectPrivate = 0x800,
		PropGag = 0x1000
	};
#ifdef SERVER
	QPConnection(QObject *parent, QTcpSocket *sock);
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
		const QPUserId &uid, qint16 initRoom, const char *wizpwd = nullptr);
	inline QPUserId& registration() const { return mReg; }
	inline QPUserId& pseudoId() const { return mUid; }
#endif // SERVER
	inline QTcpSocket* socket() const { return mSocket; }
	inline qint32 auxFlags() const { return mAuxFlags; }
	inline const char* userName() const { return mUserName; }
	inline void setUserName(const char *username) { if (username) mUserName = QByteArray(username); }
	inline const char* wizardPassword() const { return mWizardPwd; }
	inline qint16 status() const { return mStatus; }
	inline void setStatus(qint16 flags) { mStatus = flags; }
	inline qint16 room() const { return mRoom; }
	inline qint32 id() const { return mId; }
	inline qint16 face() const { return mFace; }
	inline void setFace(qint16 f) { if (f < 16) mFace = f; }
	inline const QPPoint position() const { return mPos; }
	inline void setPosition(qint16 x, qint16 y) { mPos.x = x; mPos.y = y; }
	inline QPAssetSpec prop(quint8 i) const { return mProps[i]; }
	inline void setProp(quint8 i, const QPAssetSpec &p) { mProps[i] = p; }
	inline qint16 color() const { return mColor; }
	inline void setColor(qint16 c) { if (c < 16) mColor = c; }
	inline Draw drawFlag() const { return mDraw; }
	inline void setDrawFlag(Draw d) { mDraw = d; }
signals:
	void readyRead();
#ifndef SERVER
private slots:
	void handleAboutToClose();
	void handleDisconnected();
#endif // SERVER
public slots:
	void handleReadyRead();
private:
#ifndef SERVER
	QPUserId mReg, mUid;
#endif // SERVER
	QTcpSocket *mSocket;
	QPAssetSpec mProps[9];
	QByteArray mUserName, mWizardPwd;
	QPPoint mPos;
	qint32 mAuxFlags, mId;
	qint16 mStatus, mRoom, mFace, mColor;
	Vendor mVendor;
	Draw mDraw;
#ifndef SERVER
	bool mNetEndian;
#endif // SERVER
};

#endif // _CONNECTION_H
