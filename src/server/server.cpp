#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>
#include "../message.hpp"
#include "../crypt.hpp"
#include "server.hpp"

static QString dbPath;
static const char genPwdAsc[] = "abcdefghijkmnopqrstuvwxyzABCDEFGHIJKLMNPQRSTUVWXYZ23456789";

const char roomTableDefault[] = "CREATE TABLE room(id INTEGER PRIMARY KEY, \
name VARCHAR, flags INTEGER, faces_id INTEGER, img_name VARCHAR, artist_name VARCHAR, \
password_checksum VARCHAR, hotspots BLOB)";

const char imageTableDefault[] = "CREATE TABLE image(id INTEGER PRIMARY KEY, \
name VARCHAR, alpha INTEGER)";

const char propTableDefault[] = "CREATE TABLE prop(id INTEGER PRIMARY KEY, crc INTEGER)";

const char banlistTableDefault[] = "CREATE TABLE banlist(id INTEGER PRIMARY KEY AUTOINCREMENT, \
ip_address VARCHAR, expiration VARCHAR)";

const char passwordTableDefault[] = "CREATE TABLE password(id INTEGER PRIMARY KEY AUTOINCREMENT, \
flags INTEGER, checksum VARCHAR)";

#define PWD_GOD 0x01
#define PWD_TEMP 0x02

QPServer::QPServer(QObject *parent): QObject(parent), mUserCount(0)
{
	mDb = QSqlDatabase::addDatabase("QSQLITE");
	mOptions = AllowGuests;
	mAccessFlags = TrackLogoff;
	mServer = new QTcpServer(this);
	connect(mServer, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
	
	QStringList dataloc = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
	QDir dir = dataloc[0];
	if (!dir.exists())
		if (!dir.mkpath(dataloc[0]))
			qFatal("Unable to make path %s", dataloc[0].toLocal8Bit().data());
	QTextStream ts(&dbPath);
	ts << dataloc[0] << "/qpserver.db";
	QFile dbFile(dbPath);
	
	if (!dbFile.exists())
		generateDefaultDb();
	else
	{
		mDb.setDatabaseName(dbPath);
		mDb.open();
		
		QStringList tables = mDb.tables();
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
	delete mServer;
}

bool QPServer::loadConf(const QJsonObject &data)
{
	mName = data["name"].toString().toLatin1();
	mPort = (quint16)data["port"].toInt();
	mMediaUrl = data["mediaUrl"].toString();
	if (data["allowInsecureClients"].toBool())
		mOptions |= AllowInsecureClients;
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
	if (!q.exec(imageTableDefault))
		return mDb.lastError();
	if (!q.exec(propTableDefault))
		return mDb.lastError();
	if (!q.exec(banlistTableDefault))
		return mDb.lastError();
	if (!q.exec(passwordTableDefault))
		return mDb.lastError();
	
	//createRoom(q, 86, "Lobby", QPRoom::Flag::DropZone, "lobby.png");
	
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
	
	createPassword(q, pwd, god ? PWD_GOD | PWD_TEMP : PWD_TEMP);
	qDebug("Your generated password is '%s' and will be overriden once a new password is set. You are recommended to write this down, as it is unobtainable once this message is cleared!\n", pwd);
	delete[] pwd;
}

/*QVariant QPServer::createRoom(QSqlQuery &q, qint16 id, const QString &name, QPRoom::Flags flags,
	const QString &bg, const QString &pwd, const QString &artist)
{
	q.prepare("INSERT INTO room(id, name, flags, img_name, artist_name, password_checksum) VALUES(?, ?, ?, ?, ?, ?)");
	
	q.addBindValue(id);
	q.addBindValue(name);
	q.addBindValue((qint32)flags);
	q.addBindValue(bg);
	q.addBindValue(artist == QString() ? QVariant(QVariant::String) : artist);
	if (pwd == QString())
		q.addBindValue(QVariant(QVariant::String));
	else
	{
		QString pwHash(QCryptographicHash::hash(pwd.toLatin1(), QCryptographicHash::Sha256).toHex());
		q.addBindValue(pwHash);
	}
	q.exec();
	return q.lastInsertId();
}*/

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
	ds1.skipRawData(2); // desired room id, skip until QPRoom is supported
	
	idstr = new char[6];
	ds1.readRawData(idstr, 6);
	cptr->setVendor(QPConnection::vendorFromString(idstr));
#ifndef QT_NO_DEBUG
	cptr->setRawVendor(idstr);
	idstr = nullptr;
#else
	delete[] idstr;
#endif // QT_NO_DEBUG
	
	ds1.skipRawData(24); // unused data
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
}

void QPServer::sendInfo(QPConnectionPtr cptr)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending server info");
#endif // QT_NO_DEBUG
	QPMessage sinfo(QPMessage::sinf, mConnections.key(cptr));
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
}

void QPServer::sendUserStatus(QPConnectionPtr cptr)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending user status");
#endif // QT_NO_DEBUG
	QPMessage ustatus(QPMessage::uSta, mConnections.key(cptr));
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1 << cptr->status();
	
	ustatus.setData(ba);
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << ustatus;
}

void QPServer::sendUserLog(QPConnectionPtr cptr)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending userlog");
#endif // QT_NO_DEBUG
	QPMessage ulog(QPMessage::log, mConnections.key(cptr));
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1 << mUserCount;
	
	ulog.setData(ba);
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << ulog;
}

void QPServer::sendMediaUrl(QPConnectionPtr cptr)
{
#ifndef QT_NO_DEBUG
	qDebug("Sending HTTP");
#endif // QT_NO_DEBUG
	QPMessage http(QPMessage::HTTP, mConnections.key(cptr));
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1.writeRawData(mMediaUrl.toString().toLatin1().data(), mMediaUrl.toString().size());
	ds1 << '\0';
	
	http.setData(ba);
	QDataStream ds(cptr->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << http;
}

void QPServer::handleNewConnection()
{
	QPConnectionPtr client = QPConnectionPtr(new QPConnection(mServer->nextPendingConnection()));
	sendTiyid(client);
	
	if (!client->socket()->waitForReadyRead())
		qDebug("Connection from %s timed out.", qPrintable(client->socket()->peerAddress().toString()));
	
	if (receiveLogon(client))
		if ((mOptions & AllowInsecureClients) || client->isSecureVendor())
		{
			mConnections[mUserCount] = client;
			qDebug("[%s] %s logged in from %s using %s client.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
				client->userName(), qPrintable(client->socket()->peerAddress().toString()), QPConnection::vendorToString(client->vendor()));
#ifndef QT_NO_DEBUG
			qDebug("\n%s", qPrintable(client->debugInfo()));
#endif // QT_NO_DEBUG
		}
		else
		{
			qDebug("[%s] %s connection dropped because of use of insecure client: %s.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
				qPrintable(client->socket()->peerAddress().toString()), QPConnection::vendorToString(client->vendor()));
#ifndef QT_NO_DEBUG
			qDebug("\n%s", qPrintable(client->debugInfo()));
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
	
	sendVersion(client);
	sendInfo(client);
	sendUserStatus(client);
	sendUserLog(client);
	sendMediaUrl(client);
}
