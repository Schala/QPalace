#ifndef _SERVER_H
#define _SERVER_H

#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QString>
#include <QTcpServer>
#include <QtGlobal>
#include <QVariant>
#include <QVector>

#include "../blowthru.hpp"
#include "../connection.hpp"
#include "../crypt.hpp"
#include "../message.hpp"
#include "../room.hpp"

class QPServer final: public QObject
{
	Q_OBJECT
public:
	enum
	{
		AllowGuests = 0x1,
		AllowCyborgs,
		AllowPainting = 0x4,
		AllowCustomProps = 0x8,
		AllowWizards = 0x10,
		WizardsMayKill = 0x20,
		WizardsMayAuthor = 0x40,
		PlayersMayKill = 0x80,
		CyborgsMayKill = 0x100,
		DeathPenalty = 0x200,
		PurgeInactiveProps = 0x400,
		KillFlooders = 0x800,
		NoSpoofing = 0x1000,
		UserCreatedRooms = 0x2000,
		AllowInsecureClients = 0x4000
	};
	enum
	{
		PasswordSecurity = 0x2,
		ChatLog = 0x4,
		NoWhisper = 0x8,
		Authenticate = 0x20,
		PoundProtect = 0x40, // employs heuristics to evade hackers?
		SortOptions = 0x80, // ?
		TrackLogoff = 0x100,
		JavaSecure = 0x200,
		FlashSecure = 0x400, // new addition, but unused due to port limitations
		Unicode = 0x800
	};
	QPServer(QObject *parent = nullptr);
	~QPServer();
	bool loadConf(const QJsonObject &data);
	QVariant createRoom(const QString &name, qint32 flags,
		const QString &bg, const char *pwd = nullptr, const QString &artist = QString());
	QVariant createPassword(const char *pwd, quint8 flags = 0);
	inline quint16 port() const { return mServer->serverPort(); }
	inline const char* name() const { return mName; }
	bool start();
signals:
	void roomBlowThru(const QPRoom *r, QPBlowThru *blow);
	void roomEdited(const QPRoom *r);
	void userJoinedRoom(const QPRoom *r, QPConnection *c);
	void userLeftRoom(const QPRoom *r, QPConnection *c);
	void userLoggedOn(const QPConnection *c);
	void userLoggedOff(const QPConnection *c);
	void userMoved(const QPRoom *r, const QPConnection *c);
	void userDrew(const QPRoom *r, const QPConnection *c, QPDraw *draw);
	void userTalked(const QPRoom *r, QPMessage &msg);
private slots:
	void handleNewConnection();
	void handleReadyRead();
private:
	QPCrypt mCrypt;
	QVector<QPRoom*> mRooms;
	QHash<qint32, QPConnection*> mUserLut;
	QHash<qint16, QPRoom*> mRoomLut;
	QVector<QPConnection*> mConnections;
	QSqlDatabase mDb;
	QTcpServer *mServer;
	QByteArray mName, mMediaUrl;
	quint32 mOptions;
	qint32 mAccessFlags, mUserCount; // for ID assignment
	quint16 mPort;
	qint16 mLastRoomId; // for ID assignment
	
	QSqlError generateDefaultDb(QString path);
	bool loadDb();
	void generatePassword(bool god = false);
	void tiyid(QPConnection *c);
	void logon(QPConnection *c, QPMessage &msg);
	void info(QDataStream &ds, QPConnection *c);
	void userStatus(QDataStream &ds, QPConnection *c);
	void userLog(QDataStream &ds, QPConnection *c);
	void mediaUrl(QDataStream &ds, QPConnection *c);
	void userMove(QPConnection *c, QPMessage &msg);
	void blowThru(QPBlowThru *blow);
	void userTalk(const QPConnection *c, QPMessage &msg);
};

#endif // _SERVER_H
