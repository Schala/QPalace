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
static const QByteArray genPwdAsc("abcdefghijkmnopqrstuvwxyzABCDEFGHIJKLMNPQRSTUVWXYZ23456789");

const char roomTableDefault[] = "CREATE TABLE room(id INTEGER PRIMARY KEY, \
name VARCHAR, flags INTEGER, faces_id INTEGER, img_name VARCHAR, artist_name VARCHAR, \
password_checksum VARCHAR, hotspots VARCHAR)";

const char hotspotTableDefault[] = "CREATE TABLE hotspot(id INTEGER PRIMARY KEY, \
x INTEGER, y INTEGER, destination INTEGER, type INTEGER, state INTEGER, \
state_ids VARCHAR, script BLOB)";

const char stateTableDefault[] = "CREATE TABLE state(id INTEGER PRIMARY KEY AUTOINCREMENT, \
img_id INTEGER, img_x INTEGER, img_y INTEGER)";

const char imageTableDefault[] = "CREATE TABLE image(id INTEGER PRIMARY KEY, \
name VARCHAR, alpha INTEGER)";

const char loosePropTableDefault[] = "CREATE TABLE loose_prop(id INTEGER PRIMARY KEY AUTOINCREMENT, \
prop_id INTEGER, x INTEGER, y INTEGER)";

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
		genDefaultDb();
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
	mName = qPrintable(data["name"].toString());
	mPort = (quint16)data["port"].toInt();
	if (data["allowInsecureClients"].toBool())
		mFlags |= AllowInsecureClients;
	return true;
}

QSqlError QPServer::genDefaultDb()
{
	mDb.setDatabaseName(dbPath);
	if (!mDb.open())
		return mDb.lastError();
	
	QSqlQuery q;
	if (!q.exec(roomTableDefault))
		return mDb.lastError();
	if (!q.exec(hotspotTableDefault))
		return mDb.lastError();
	if (!q.exec(stateTableDefault))
		return mDb.lastError();
	if (!q.exec(imageTableDefault))
		return mDb.lastError();
	if (!q.exec(loosePropTableDefault))
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
		pwd[i] = genPwdAsc[qrand() % genPwdAsc.size()];
	
	createPassword(q, pwd, god ? PWD_GOD | PWD_TEMP : PWD_TEMP);
	qDebug("Your generated password is '%s' and will be overriden once a new password is set. You are recommended to write this down, as it is unobtainable once this message is cleared!\n", pwd);
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

void QPServer::handleNewConnection()
{
	QPMessage tiyid(QPMessage::tiyr, ++mUserCount), logon;
	QPConnectionPtr client = QPConnectionPtr(new QPConnection(mServer->nextPendingConnection()));
	QDataStream ds(client->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << tiyid;
	
	if (!client->socket()->waitForReadyRead())
		qDebug("Connection from %s timed out.", qPrintable(client->socket()->peerAddress().toString()));
	if (client->socket()->bytesAvailable() >= 12)
		ds >> logon;
		
	QByteArray lba(logon.data(), logon.size());
	QDataStream ds1(lba);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	quint32 ctr, crc;
	ds1 >> crc >> ctr;
	client->setRegistration(QPRegistration(ctr, crc));
	
	ds1.skipRawData(1); // discard length, QByteArray will auto determine
	char *idstr = new char[31];
	ds1.readRawData(idstr, 31);
	client->setUserName(QByteArray(idstr));
	delete[] idstr;
	
	ds1.skipRawData(1);
	idstr = new char[31];
	ds1.readRawData(idstr, 31);
	client->setWizardPassword(QByteArray(idstr));
	delete[] idstr;
	
	qint32 flags;
	ds1 >> flags;
	client->setAuxFlags(flags);
	
	ds1 >> ctr >> crc;
	client->setPseudoId(QPRegistration(ctr, crc));
	
	ds1.skipRawData(12); // demo shit not needed anymore
	ds1.skipRawData(2); // desired room id, skip until QPRoom is supported
	
	idstr = new char[6];
	ds1.readRawData(idstr, 6);
	client->setVendor(QPConnection::vendorFromString(idstr));
	delete[] idstr;
	
	if ((mFlags & AllowInsecureClients) || client->isSecureVendor())
	{
		mConnections.push_back(client);
		qDebug("[%s] %s logged in from %s using %s client.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
			client->userName(), qPrintable(client->socket()->peerAddress().toString()), QPConnection::vendorToString(client->vendor()));
	}
	else
	{
		qDebug("[%s] %s connection dropped because of use of insecure client: %s.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
			qPrintable(client->socket()->peerAddress().toString()), QPConnection::vendorToString(client->vendor()));
		client.clear();
	}
	
	/*qDebug("ID=%X size=%u ref=%d", logon.id(), logon.size(), logon.ref());
	quint32 crc, counter, demoelapsed, demolimit, totalelapsed, ulreqprot, ulcaps, dlcaps, d2engcaps, d3engcaps, gfxcaps, pcrc, pctr;
	char *uname = new char[31], *pwd = new char[31], *sig = new char[6];
	qint32 flags;
	qint16 room;
	quint8 slen;
	ds1 >> crc >> counter >> slen;
	ds1.readRawData(uname, 31);
	ds1 >> slen;
	ds1.readRawData(pwd, 31);
	ds1 >> flags >> pctr >> pcrc >> demoelapsed >> totalelapsed >> demolimit >> room;
	ds1.readRawData(sig, 6);
	ds1 >> ulreqprot >> ulcaps >> dlcaps >> d2engcaps >> gfxcaps >> d3engcaps;
	qDebug("CRC=%u\nCounter=%u\nUsername=%s\nWiz pass=%s\nAuxflags=%d\nPUIDctr=%u", crc, counter, uname, pwd, flags, pctr);
	qDebug("PUIDcrc=%u\ndemo elapse=%u\ntotal elapse=%u\ndemo limit=%u\nroom=%d", pcrc, demoelapsed, totalelapsed, demolimit, room);
	qDebug("sig=%s\nrequested protocol=%u\nul caps=%u\ndl caps=%u\n2d engine=%u\n2d gfx=%u\n3d engine=%u", sig, ulreqprot, ulcaps, dlcaps, d2engcaps, gfxcaps, d3engcaps);*/
}
