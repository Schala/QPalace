#ifndef _SERVER_H
#define _SERVER_H

#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QTcpServer>
#include <QtGlobal>
#include <QVariant>
#include <QVector>

#include "../connection.hpp"
#include "../room.hpp"

class QPServer final: public QObject
{
	Q_OBJECT
public:
	enum
	{
		AllowGuests = 0x0001,
		AllowCyborgs,
		AllowPainting = 0x0004,
		AllowCustomProps = 0x0008,
		AllowWizards = 0x0010,
		WizardsMayKill = 0x0020,
		WizardsMayAuthor = 0x0040,
		PlayersMayKill = 0x0080,
		CyborgsMayKill = 0x0100,
		DeathPenalty = 0x0200,
		PurgeInactiveProps = 0x0400,
		KillFlooders = 0x0800,
		NoSpoofing = 0x1000,
		UserCreatedRooms = 0x2000
	};
	enum
	{
		PasswordSecurity = 0x00000002,
		ChatLog = 0x00000004,
		NoWhisper = 0x00000008,
		Authenticate = 0x00000020,
		PoundProtect = 0x00000040, // employs heuristics to evade hackers?
		SortOptions = 0x00000080, // ?
		TrackLogoff = 0x00000100,
		JavaSecure = 0x00000200,
		AllowInsecureClients = 0x00000400,
		FlashSecure = 0x00000800 // new addition, but unused due to port limitations
	};
	enum
	{
		PalaceAssets = 0x00000001,
		FtpAssets,
		HttpAssets = 0x00000004,
		OtherAssets = 0x00000008,
		PalaceFiles = 0x00000010,
		FtpFiles = 0x00000020,
		HttpFiles = 0x00000040,
		OtherFiles = 0x00000080,
		ExtendUploadPacket = 0x00000100,
		ExtendDownloadPacket = 0x00000200
	};
	QPServer(QObject *parent = nullptr);
	~QPServer();
	bool loadConf(const QJsonObject &data);
	QVariant createRoom(QSqlQuery &q, const QString &name, qint32 flags,
		const QString &bg, const char *pwd = nullptr, const QString &artist = QString());
	QVariant createPassword(QSqlQuery &q, const char *pwd, quint8 flags = 0);
	inline quint16 port() const { return mServer->serverPort(); }
	inline const char* name() const { return mName; }
	bool start();
private slots:
	void handleNewConnection();
private:
	QVector<QPRoomPtr> mRooms;
	QHash<qint16, qint16> mRoomLut;
	QVector<QPConnectionPtr> mConnections;
	QSqlDatabase mDb;
	QTcpServer *mServer;
	QByteArray mName, mMediaUrl;
	quint32 mOptions, mDlCaps, mUlCaps;
	qint32 mAccessFlags, mUserCount; // for ID assignment
	quint16 mPort;
	qint16 mLastRoomId; // for ID assignment
	
	QSqlError generateDefaultDb();
	bool loadDb();
	void generatePassword(QSqlQuery &q, bool god = false);
	void sendTiyid(QPConnectionPtr cptr);
	bool receiveLogon(QPConnectionPtr cptr);
	void sendVersion(QPConnectionPtr cptr);
	void sendInfo(QPConnectionPtr cptr);
	void sendUserStatus(QPConnectionPtr cptr);
	void sendUserLog(QPConnectionPtr cptr);
	void sendMediaUrl(QPConnectionPtr cptr);
	void sendRoomInfo(QPConnectionPtr cptr, qint16 id);
	void sendRoomUsers(QPConnectionPtr cptr, qint16 id);
	void sendNewUser(QPConnectionPtr cptr);
};

#endif // _SERVER_H
