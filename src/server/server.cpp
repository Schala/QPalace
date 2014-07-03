#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>
#include "server.hpp"

static QString dbPath;

const char roomTableDefault[] = "CREATE TABLE room(id INTEGER PRIMARY KEY, \
name VARCHAR, flags INTEGER, faces_id INTEGER, img_name VARCHAR, artist_name VARCHAR, \
password VARCHAR, hotspots VARCHAR)";

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

QPServer::QPServer(QObject *parent): QObject(parent), mServer(new QTcpServer(this))
{
	mDb = QSqlDatabase::addDatabase("QSQLITE");
	
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

bool QPServer::loadConf(const QJsonObject &data)
{
	mPort = (quint16)data["port"].toInt();
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
	
	createRoom(q, 86, "Lobby", QPRoom::Flag::DropZone, "lobby.png");
	
	qDebug("`qpserver.db` was not available, so it has been generated.\n");
	
	return QSqlError();
}

QVariant QPServer::createRoom(QSqlQuery &q, qint16 id, const QString &name, QPRoom::Flags flags,
	const QString &bg, const QString &pwd, const QString &artist)
{
	q.prepare("INSERT INTO room(id, name, flags, img_name, artist_name, password) VALUES(?, ?, ?, ?, ?, ?)");
	
	q.addBindValue(id);
	q.addBindValue(name);
	q.addBindValue((qint32)flags);
	q.addBindValue(bg);
	q.addBindValue(artist == QString() ? QVariant(QVariant::String) : artist);
	q.addBindValue(pwd == QString() ? QVariant(QVariant::String) : pwd);
	q.exec();
	
	return q.lastInsertId();
}
