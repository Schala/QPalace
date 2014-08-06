#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QThreadPool>
#include "../crypt.hpp"
#include "server.hpp"

static QString dbPath = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[0] + (char*)"/qserver.db";
static const char genPwdAsc[] = "abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789";

static const char roomTableDefault[] = "CREATE TABLE room(id INTEGER PRIMARY KEY, \
name VARCHAR, flags INTEGER, faces_id INTEGER, img_name VARCHAR, artist_name VARCHAR, \
password BLOB, hotspots BLOB, images BLOB, loose_props BLOB, scripts BLOB, points BLOB, \
states BLOB, spot_names BLOB)";

static const char propTableDefault[] = "CREATE TABLE prop(id INTEGER PRIMARY KEY, crc INTEGER)";

static const char banlistTableDefault[] = "CREATE TABLE banlist(id INTEGER PRIMARY KEY AUTOINCREMENT, \
ip_address VARCHAR, expiration VARCHAR)";

static const char passwordTableDefault[] = "CREATE TABLE password(id INTEGER PRIMARY KEY AUTOINCREMENT, \
flags INTEGER, checksum VARCHAR)";

static const char imageTableDefault[] = "CREATE TABLE image(id INTEGER PRIMARY KEY AUTOINCREMENT, \
filename VARCHAR)";

#define PWD_GOD 0x01

QPServer::QPServer(QObject *parent): QObject(parent), mUserCount(0)
{
	mDb = QSqlDatabase::addDatabase("QSQLITE");
	mOptions = AllowGuests;
	mAccessFlags = TrackLogoff;
	mServer = new QTcpServer(this);
	mLastRoomId = mLobbyId;
	connect(mServer, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
	
	QStringList dataloc = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    QDir dir = dataloc[0];
	if (!dir.exists())
		if (!dir.mkpath(dataloc[0]))
			qFatal("Unable to make path %s", qPrintable(dataloc[0]));
	QFile dbFile(dbPath);
	
	if (!dbFile.exists())
		generateDefaultDb();
	else
	{
		mDb.setDatabaseName(dbPath);
		mDb.open();
		loadDb();
	}
}

QPServer::~QPServer()
{
	if (!mConnections.isEmpty())
	{
		for (auto c: mConnections)
			delete c;
		mConnections.clear();
	}
	if (!mRooms.isEmpty())
	{
		for (auto r: mRooms)
			delete r;
		mRooms.clear();
	}
	delete mServer;
}

bool QPServer::loadDb()
{
	QSqlQuery q;
	q.exec("SELECT * FROM room");
	
	while (q.next())
	{
		qint16 roomId = (qint16)q.value("id").toInt();
		mRooms.push_back(new QPRoom(q, roomId));
		mRoomLut[roomId] = mRooms.size()-1;
	}
	
	return true;
}

bool QPServer::start()
{
	if (!mServer->listen(QHostAddress::Any, mPort))
	{
		qFatal("Unable to start server: %s", qPrintable(mServer->errorString()));
		return false;
	}
	
	return true;
}

bool QPServer::loadConf(const QJsonObject &data)
{
	mName = data["name"].toString().toLatin1();
	mPort = (quint16)data["port"].toInt();
	mMediaUrl = data["mediaUrl"].toString().toLatin1();
	mLobbyId = (qint16)data["lobbyRoomId"].toInt();
	int maxThreadCount = data["maxThreadCount"].toInt();
	if (maxThreadCount > 0)
		QThreadPool::globalInstance()->setMaxThreadCount(maxThreadCount);
	if (data["allowInsecureClients"].toBool())
		mAccessFlags |= AllowInsecureClients;
	if (data["allowScripts"].toBool())
		mAccessFlags |= AllowCyborgs;
	if (data["allowDrawing"].toBool())
		mAccessFlags |= AllowPainting;
	if (data["allowCustomProps"].toBool())
		mAccessFlags |= AllowCustomProps;
	if (data["allowWizards"].toBool())
		mAccessFlags |= AllowWizards;
	if (data["wizardsMayKick"].toBool())
		mAccessFlags |= WizardsMayKill;
	if (data["wizardsMayAuthor"].toBool())
		mAccessFlags |= WizardsMayAuthor;
	if (data["usersMayKick"].toBool())
		mAccessFlags |= PlayersMayKill;
	if (data["scriptsMayKick"].toBool())
		mAccessFlags |= CyborgsMayKill;
	if (data["enforceBans"].toBool())
		mAccessFlags |= DeathPenalty;
	if (data["purgeInactiveProps"].toBool())
		mAccessFlags |= PurgeInactiveProps;
	if (data["antiSpamProtection"].toBool())
		mAccessFlags |= KillFlooders;
	if (data["disableSpoofing"].toBool())
		mAccessFlags |= NoSpoofing;
	if (data["allowUserCreatedRooms"].toBool())
		mAccessFlags |= UserCreatedRooms;
	if (data["enforceLogonPassword"].toBool())
		mOptions |= PasswordSecurity;
	if (data["logChat"].toBool())
		mOptions |= ChatLog;
	if (data["disableWhispering"].toBool())
		mOptions |= NoWhisper;
	
	return true;
}

QSqlError QPServer::generateDefaultDb()
{
	mDb.setDatabaseName(dbPath);
	if (!mDb.open())
		return mDb.lastError();
	
	QSqlQuery q;
	if (!q.exec(roomTableDefault))
		return mDb.lastError();
	if (!q.exec(propTableDefault))
		return mDb.lastError();
	if (!q.exec(banlistTableDefault))
		return mDb.lastError();
	if (!q.exec(passwordTableDefault))
		return mDb.lastError();
	if (!q.exec(imageTableDefault))
		return mDb.lastError();
	
	createRoom(q, "Lobby", QPRoom::DropZone, "lobby.png");
	
	qDebug("`qpserver.db` was not available, so it has been generated.\n");
	generatePassword(q, true);
	
	return QSqlError();
}

void QPServer::generatePassword(QSqlQuery &q, bool god)
{
	char *pwd = new char[8];
	pwd[7] = '\0';
	qsrand(QDateTime::currentDateTime().toTime_t());
	
	for (quint8 i = 0; i < 7; i++)
		pwd[i] = genPwdAsc[qrand() % qstrlen(genPwdAsc)];
	
	createPassword(q, pwd, god ? PWD_GOD : 0);
	qDebug("Your generated password is '%s' and will be overriden once a new password is set. You are recommended to write this down, as it is unobtainable once this message is cleared!\n", pwd);
	delete[] pwd;
}

QVariant QPServer::createRoom(QSqlQuery &q, const QString &name, qint32 flags,
	const QString &bg, const char *pwd, const QString &artist)
{
	q.prepare("INSERT INTO room(id, name, flags, img_name, artist_name, password) VALUES(?, ?, ?, ?, ?, ?)");
	
	q.addBindValue(mLastRoomId);
	q.addBindValue(name);
	q.addBindValue(flags);
	q.addBindValue(bg);
	q.addBindValue(artist == QString() ? QVariant(QVariant::String) : artist);
	
	if (pwd)
	{
		QByteArray encPwd(pwd);
		QPCrypt crypt;
		crypt.encrypt(encPwd.data(), encPwd.size());
		q.addBindValue(encPwd);
	}
	else
		q.addBindValue(QVariant(QVariant::ByteArray));
	
	q.exec();
	mRooms[mRooms.size()-1] = new QPRoom(q, mLastRoomId);
	mRoomLut[mLastRoomId] = mRooms.size()-1;
	mLastRoomId++;
	return q.lastInsertId();
}

QVariant QPServer::createPassword(QSqlQuery &q, const char *pwd, quint8 flags)
{
	QByteArray encPwd(pwd);
	QPCrypt crypt;
	
	q.prepare("INSERT INTO password(flags, checksum) VALUES(?, ?)");
	q.addBindValue(flags);
	
	crypt.encrypt(encPwd.data(), encPwd.size());
	QString pwHash(QCryptographicHash::hash(encPwd, QCryptographicHash::Sha256).toHex());
	q.addBindValue(pwHash);
	
	q.exec();
	return q.lastInsertId();
}

void QPServer::sendTiyid(QPConnection *c)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending TIYID");
#endif // QT_NO_DEBUG
	QPMessage tiyid(QPMessage::tiyr, ++mUserCount);
	QDataStream ds(c->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << tiyid;
	c->setId(mUserCount);
#ifndef QT_NO_DEBUG
	tiyid.dump();
#endif // QT_NO_DEBUG
}

void QPServer::receiveLogon(QPConnection *c, QPMessage &msg)
{
#ifndef QT_NO_DEBUG
	qDebug("Receiving logon");
#endif // QT_NO_DEBUG
	QByteArray lba(msg.data(), msg.size());
	QDataStream ds(lba);
	ds.device()->reset();
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

	ds.skipRawData(9); // 8 bits of legacy data; also discard length byte, QByteArray will auto determine
	char *idstr = new char[31];
	ds.readRawData(idstr, 31);
	c->setUserName(idstr);
	delete[] idstr;

	quint8 slen;
	ds >> slen; // encrypted password might have null chars
#ifndef QT_NO_DEBUG
	qDebug("Read password length of %u characters.", slen);
#endif // QT_NO_DEBUG
	if (slen)
	{
		idstr = new char[slen];
		ds.readRawData(idstr, slen);
#ifndef QT_NO_DEBUG
		QPCrypt crypt;
		crypt.decrypt(idstr, slen);
#endif // QT_NO_DEBUG
		c->setWizardPassword(idstr, slen);
		delete[] idstr;
	}
	ds.skipRawData(31 - slen);

	qint32 flags;
	ds >> flags;
	c->setAuxFlags(flags);

	ds.skipRawData(20); // legacy data

	qint16 roomId;
	ds >> roomId;
	c->setRoom(roomId);

	idstr = new char[7];
	idstr[6] = '\0';
	ds.readRawData(idstr, 6);
	c->setVendor(QPConnection::vendorFromString(idstr));
#ifndef QT_NO_DEBUG
	c->setRawVendor(idstr);
	idstr = nullptr;
#else
	delete[] idstr;
#endif // QT_NO_DEBUG

	ds.skipRawData(24); // unused data
#ifndef QT_NO_DEBUG
	msg.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendVersion(QDataStream &ds)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending version");
#endif // QT_NO_DEBUG
	QPMessage version(QPMessage::vers, 0x00010016);
	ds << version;
#ifndef QT_NO_DEBUG
	version.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendInfo(QDataStream &ds, QPConnection *c)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending server info");
#endif // QT_NO_DEBUG
	QPMessage sinfo(QPMessage::sinf, c->id());
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	quint8 slen = (quint8)qstrlen(mName);
	ds1 << mAccessFlags << slen;
	ds1.writeRawData(mName, slen);
	ds1 << mOptions << mUlCaps << mDlCaps;
	
	sinfo.setData(ba);
	ds << sinfo;
#ifndef QT_NO_DEBUG
	sinfo.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendUserStatus(QDataStream &ds, QPConnection *c)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending user status");
#endif // QT_NO_DEBUG
	QPMessage ustatus(QPMessage::uSta, c->id());
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1 << c->status();
	
	ustatus.setData(ba);
	ds << ustatus;
#ifndef QT_NO_DEBUG
	ustatus.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendUserLog(QDataStream &ds, QPConnection *c)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending userlog");
#endif // QT_NO_DEBUG
	QPMessage ulog(QPMessage::log, c->id());
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1 << mUserCount;
	
	ulog.setData(ba);
	ds << ulog;
#ifndef QT_NO_DEBUG
	ulog.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendMediaUrl(QDataStream &ds, QPConnection *c)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending HTTP");
#endif // QT_NO_DEBUG
	QPMessage http(QPMessage::HTTP, c->id());
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1.writeRawData(mMediaUrl.data(), mMediaUrl.size());
	ds1 << (quint8)0;
	
	http.setData(ba);
	ds << http;
#ifndef QT_NO_DEBUG
	http.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendRoomInfo(QDataStream &ds, QPConnection *c)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending room description");
#endif // QT_NO_DEBUG
	QPMessage *room = mRooms[mRoomLut[c->room()]]->description();
	ds << *room;
#ifndef QT_NO_DEBUG
	room->dump();
#endif // QT_NO_DEBUG
	delete room;
	sendRoomUsers(ds, c->room());
}

void QPServer::sendRoomUsers(QDataStream &ds, qint16 id)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending room users");
#endif // QT_NO_DEBUG
	QPMessage rprs(QPMessage::rprs, mRooms[mRoomLut[id]]->population());
	QPMessage endr(QPMessage::endr);
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

	for (qint32 i = 0; i < mRooms[mRoomLut[id]]->population(); i++)
	{
		ds1 << mRooms[mRoomLut[id]]->user(i)->id();
		ds1 << mRooms[mRoomLut[id]]->user(i)->position().x << mRooms[mRoomLut[id]]->user(i)->position().y;
		for (quint8 j = 0; j < 9; j++)
			ds1 << mRooms[mRoomLut[id]]->user(i)->prop(j).id << mRooms[mRoomLut[id]]->user(i)->prop(j).crc;
		ds1 << mRooms[mRoomLut[id]]->user(i)->room();
		ds1 << mRooms[mRoomLut[id]]->user(i)->face();
		ds1 << mRooms[mRoomLut[id]]->user(i)->color();
		ds1 << (qint32)0; // not used

		qint16 activeProps = 0;
		for (quint8 j = 0; j < 9; j++)
			if (mRooms[mRoomLut[id]]->user(i)->prop(j).id && mRooms[mRoomLut[id]]->user(i)->prop(j).crc)
				activeProps++;
		ds1 << activeProps;

		quint8 nlen = qstrlen(mRooms[mRoomLut[id]]->user(i)->userName());
		ds1 << nlen;
		ds1.writeRawData(mRooms[mRoomLut[id]]->user(i)->userName(), nlen);
		for (quint8 j = 0; j < (31 - nlen); j++)
			ds1 << (quint8)0;
	}

	rprs.setData(ba);
	ds << rprs << endr;
#ifndef QT_NO_DEBUG
	rprs.dump();
	endr.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendNewUser(QDataStream &ds, QPConnection *c)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending new user");
#endif // QT_NO_DEBUG
	QPMessage nprs(QPMessage::nprs, c->id());
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

	ds1 << c->id();
	ds1 << c->position().x << c->position().y;
	for (quint8 j = 0; j < 9; j++)
		ds1 << c->prop(j).id << c->prop(j).crc;
	ds1 << c->room();
	ds1 << c->face();
	ds1 << c->color();
	ds1 << (qint32)0; // not used

	qint16 activeProps = 0;
	for (quint8 j = 0; j < 9; j++)
		if (c->prop(j).id && c->prop(j).crc)
			activeProps++;
	ds1 << activeProps;

	quint8 nlen = qstrlen(c->userName());
	ds1 << nlen;
	ds1.writeRawData(c->userName(), nlen);
	for (quint8 j = 0; j < (31 - nlen); j++)
		ds1 << (quint8)0;

	nprs.setData(ba);
	ds << nprs;
#ifndef QT_NO_DEBUG
	nprs.dump();
#endif // QT_NO_DEBUG
}

void QPServer::handleNewConnection()
{
	QPConnection *c = new QPConnection(mServer->nextPendingConnection());
	connect(c, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));

	sendTiyid(c);

	if (!c->socket()->waitForReadyRead())
		qDebug("[%s] Connection from %s timed out.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
			qPrintable(c->socket()->peerAddress().toString()));
}

void QPServer::handleReadyRead()
{
	QPMessage msg;
	QPConnection *c = (QPConnection*)sender();
	QDataStream ds(c->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

	if (c->socket()->bytesAvailable() >= 12)
		ds >> msg;
	else
	{
		qWarning("[%s] %s connection dropped because of incomplete packet data!", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
			qPrintable(c->socket()->peerAddress().toString()));
		delete c;
		mUserCount--;
	}

	switch (msg.id())
	{
		case QPMessage::regi:
			receiveLogon(c, msg);
			if ((mAccessFlags & AllowInsecureClients) || c->isSecureVendor())
			{
				mConnections.push_back(c);
				qDebug("[%s] %s logged in from %s using %s client on %s.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
					c->userName(), qPrintable(c->socket()->peerAddress().toString()), QPConnection::vendorToString(c->vendor()),
					c->osToString());
#ifndef QT_NO_DEBUG
				qDebug("\n%s", qPrintable(c->loginDebugInfo()));
#endif // QT_NO_DEBUG
			}
			else
			{
				qDebug("[%s] %s connection dropped because of use of insecure client: %s.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
					qPrintable(c->socket()->peerAddress().toString()), QPConnection::vendorToString(c->vendor()));
#ifndef QT_NO_DEBUG
				qDebug("\n%s", qPrintable(c->loginDebugInfo()));
#endif // QT_NO_DEBUG
				delete c;
				mUserCount--;
				return;
			}

			qsrand(QDateTime::currentDateTime().toTime_t());
			c->setFace(qrand() % 16);
			c->setColor(qrand() % 16);
			c->setPosition(0, 0); // placeholder, todo: fetch room bg width/height and randomise pos
			c->setRoom(mLobbyId);

			sendVersion(ds);
			sendInfo(ds, c);
			sendUserStatus(ds, c);
			sendUserLog(ds, c);
			sendMediaUrl(ds, c);
			sendRoomInfo(ds, c);
			sendNewUser(ds, c);
			break;
		default:
			qWarning("[%s] %s has sent an unknown or unsupported packet (%X). Ignoring...", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
				qPrintable(c->socket()->peerAddress().toString()), msg.id());
	}
}
