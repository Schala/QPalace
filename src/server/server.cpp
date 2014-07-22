#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include "../message.hpp"
#include "../crypt.hpp"
#include "server.hpp"

static QString dbPath;
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
	mLastRoomId = 86;
	connect(mServer, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
	
	QStringList dataloc = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
	QDir dir = dataloc[0];
	if (!dir.exists())
		if (!dir.mkpath(dataloc[0]))
			qFatal("Unable to make path %s", qPrintable(dataloc[0]));
	QTextStream ts(&dbPath);
	ts << dataloc[0] << "/qpserver.db";
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
			c.clear();
		mConnections.clear();
	}
	if (!mRooms.isEmpty())
	{
		for (auto r: mRooms)
			r.clear();
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
		mRooms.push_back(QPRoomPtr::create(q, roomId));
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
		QPCryptEngine crypt;
		crypt.encrypt(encPwd.data(), encPwd.size());
		q.addBindValue(encPwd);
	}
	else
		q.addBindValue(QVariant(QVariant::ByteArray));
	
	q.exec();
	mRooms[mRooms.size()-1] = QPRoomPtr::create(q, mLastRoomId);
	mRoomLut[mLastRoomId] = mRooms.size()-1;
	mLastRoomId++;
	return q.lastInsertId();
}

QVariant QPServer::createPassword(QSqlQuery &q, const char *pwd, quint8 flags)
{
	QByteArray encPwd(pwd);
	QPCryptEngine crypt;
	
	q.prepare("INSERT INTO password(flags, checksum) VALUES(?, ?)");
	q.addBindValue(flags);
	
	crypt.encrypt(encPwd.data(), encPwd.size());
	QString pwHash(QCryptographicHash::hash(encPwd, QCryptographicHash::Sha256).toHex());
	q.addBindValue(pwHash);
	
	q.exec();
	return q.lastInsertId();
}

void QPServer::sendTiyid(QPConnectionPtr cptr)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending TIYID");
#endif // QT_NO_DEBUG
	QPMessage tiyid(QPMessage::tiyr, ++mUserCount);
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << tiyid;
	cptr->setId(mUserCount);
#ifndef QT_NO_DEBUG
	tiyid.dump();
#endif // QT_NO_DEBUG
}

bool QPServer::receiveLogon(QPConnectionPtr cptr)
{
#ifndef QT_NO_DEBUG
	qDebug("Receiving logon");
#endif // QT_NO_DEBUG
	QPMessage logon;
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	if (cptr->socket()->bytesAvailable() >= 12)
		ds >> logon;
	else
		return false;
	
	QByteArray lba(logon.data(), logon.size());
	QDataStream ds1(lba);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1.skipRawData(9); // 8 bits of legacy data; also discard length byte, QByteArray will auto determine
	char *idstr = new char[31];
	ds1.readRawData(idstr, 31);
	cptr->setUserName(idstr);
	delete[] idstr;
	
	quint8 slen;
	ds1 >> slen; // encrypted password might have null chars
#ifndef QT_NO_DEBUG
	qDebug("Read password length of %u characters.", slen);
#endif // QT_NO_DEBUG
	if (slen)
	{
		idstr = new char[slen];
		ds1.readRawData(idstr, slen);
#ifndef QT_NO_DEBUG
		QPCryptEngine crypt;
		crypt.decrypt(idstr, slen);
#endif // QT_NO_DEBUG
		cptr->setWizardPassword(idstr, slen);
		delete[] idstr;
	}
	ds1.skipRawData(31 - slen);
	
	qint32 flags;
	ds1 >> flags;
	cptr->setAuxFlags(flags);
	
	ds1.skipRawData(20); // legacy data
	
	qint16 roomId;
	ds1 >> roomId;
	cptr->setRoom(roomId);
	
	idstr = new char[7];
	idstr[6] = '\0';
	ds1.readRawData(idstr, 6);
	cptr->setVendor(QPConnection::vendorFromString(idstr));
#ifndef QT_NO_DEBUG
	cptr->setRawVendor(idstr);
	idstr = nullptr;
#else
	delete[] idstr;
#endif // QT_NO_DEBUG
	
	ds1.skipRawData(24); // unused data
#ifndef QT_NO_DEBUG
	logon.dump();
#endif // QT_NO_DEBUG
	return true;
}

void QPServer::sendVersion(QPConnectionPtr cptr)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending version");
#endif // QT_NO_DEBUG
	QPMessage version(QPMessage::vers, 0x00010016);
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << version;
#ifndef QT_NO_DEBUG
	version.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendInfo(QPConnectionPtr cptr)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending server info");
#endif // QT_NO_DEBUG
	QPMessage sinfo(QPMessage::sinf, cptr->id());
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	quint8 slen = (quint8)qstrlen(mName);
	ds1 << mAccessFlags << slen;
	ds1.writeRawData(mName, slen);
	ds1 << mOptions << mUlCaps << mDlCaps;
	
	sinfo.setData(ba);
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << sinfo;
#ifndef QT_NO_DEBUG
	sinfo.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendUserStatus(QPConnectionPtr cptr)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending user status");
#endif // QT_NO_DEBUG
	QPMessage ustatus(QPMessage::uSta, cptr->id());
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1 << cptr->status();
	
	ustatus.setData(ba);
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << ustatus;
#ifndef QT_NO_DEBUG
	ustatus.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendUserLog(QPConnectionPtr cptr)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending userlog");
#endif // QT_NO_DEBUG
	QPMessage ulog(QPMessage::log, cptr->id());
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1 << mUserCount;
	
	ulog.setData(ba);
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << ulog;
#ifndef QT_NO_DEBUG
	ulog.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendMediaUrl(QPConnectionPtr cptr)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending HTTP");
#endif // QT_NO_DEBUG
	QPMessage http(QPMessage::HTTP, cptr->id());
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1.writeRawData(mMediaUrl.data(), mMediaUrl.size());
	ds1 << (quint8)0;
	
	http.setData(ba);
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << http;
#ifndef QT_NO_DEBUG
	http.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendRoomInfo(QPConnectionPtr cptr, qint16 id)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending room description");
#endif // QT_NO_DEBUG
	QPMessage *room = mRooms[mRoomLut[id]]->description();
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << *room;
#ifndef QT_NO_DEBUG
	room->dump();
#endif // QT_NO_DEBUG
	delete room;
}

void QPServer::sendRoomUsers(QPConnectionPtr cptr, qint16 id)
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
	
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	rprs.setData(ba);
	ds << rprs << endr;
#ifndef QT_NO_DEBUG
	rprs.dump();
	endr.dump();
#endif // QT_NO_DEBUG
}

void QPServer::sendNewUser(QPConnectionPtr cptr)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending new user");
#endif // QT_NO_DEBUG
	QPMessage nprs(QPMessage::nprs, cptr->id());
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1 << cptr->id();
	ds1 << cptr->position().x << cptr->position().y;
	for (quint8 j = 0; j < 9; j++)
		ds1 << cptr->prop(j).id << cptr->prop(j).crc;
	ds1 << cptr->room();
	ds1 << cptr->face();
	ds1 << cptr->color();
	ds1 << (qint32)0; // not used
	
	qint16 activeProps = 0;
	for (quint8 j = 0; j < 9; j++)
		if (cptr->prop(j).id && cptr->prop(j).crc)
			activeProps++;
	ds1 << activeProps;
	
	quint8 nlen = qstrlen(cptr->userName());
	ds1 << nlen;
	ds1.writeRawData(cptr->userName(), nlen);
	for (quint8 j = 0; j < (31 - nlen); j++)
		ds1 << (quint8)0;
	
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	nprs.setData(ba);
	ds << nprs;
#ifndef QT_NO_DEBUG
	nprs.dump();
#endif // QT_NO_DEBUG
}

void QPServer::handleNewConnection()
{
	QPConnectionPtr client = QPConnectionPtr(new QPConnection(mServer->nextPendingConnection()));
	sendTiyid(client);
	
	if (!client->socket()->waitForReadyRead())
		qDebug("Connection from %s timed out.", qPrintable(client->socket()->peerAddress().toString()));
	
	if (receiveLogon(client))
		if ((mAccessFlags & AllowInsecureClients) || client->isSecureVendor())
		{
			mConnections.push_back(client);
			qDebug("[%s] %s logged in from %s using %s client on %s.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
				client->userName(), qPrintable(client->socket()->peerAddress().toString()), QPConnection::vendorToString(client->vendor()),
				client->osToString());
#ifndef QT_NO_DEBUG
			qDebug("\n%s", qPrintable(client->loginDebugInfo()));
#endif // QT_NO_DEBUG
		}
		else
		{
			qDebug("[%s] %s connection dropped because of use of insecure client: %s.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
				qPrintable(client->socket()->peerAddress().toString()), QPConnection::vendorToString(client->vendor()));
#ifndef QT_NO_DEBUG
			qDebug("\n%s", qPrintable(client->loginDebugInfo()));
#endif // QT_NO_DEBUG
			client.clear();
			mUserCount--;
			return;
		}
	else
	{
		qWarning("[%s] %s connection dropped because of corrupt or incomplete logon data!", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
			qPrintable(client->socket()->peerAddress().toString()));
		client.clear();
		mUserCount--;
		return;
	}
	
	qsrand(QDateTime::currentDateTime().toTime_t());
	client->setFace(qrand() % 16);
	client->setColor(qrand() % 16);
	client->setPosition(0, 0); // placeholder, todo: fetch room bg width/height and randomise pos
	client->setRoom(86);
	
	sendVersion(client);
	sendInfo(client);
	sendUserStatus(client);
	sendUserLog(client);
	sendMediaUrl(client);
	sendRoomInfo(client, 86);
#ifndef QT_NO_DEBUG
			qDebug("\n%s", qPrintable(mRooms[mRoomLut[86]]->descDebugInfo()));
#endif // QT_NO_DEBUG
	sendRoomUsers(client, 86);
	sendNewUser(client);
}
