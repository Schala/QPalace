#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include "server.hpp"

static const char genPwdAsc[] = "abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789";

static const char roomTableDefault[] = "CREATE TABLE room(id INTEGER PRIMARY KEY, \
name VARCHAR, flags INTEGER, faces_id INTEGER, img_name VARCHAR, artist_name VARCHAR, \
password BLOB, hotspots BLOB, images BLOB, loose_props BLOB, scripts BLOB, points BLOB, \
states BLOB, spot_names BLOB, draw_buffer BLOB)";

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
	mLastRoomId = 0;
	connect(mServer, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
	
	QString dbPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/qpserver.db";
	QString dataloc = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	QDir dir = dataloc;
	if (!dir.exists())
		if (!dir.mkpath(dataloc))
			qFatal("Unable to make path %s", qPrintable(dataloc[0]));
	QFile dbFile(dbPath);
	
	if (!dbFile.exists())
		generateDefaultDb(dbPath);
	else
	{
		mDb.setDatabaseName(dbPath);
		mDb.open();
		loadDb();
	}
}

bool QPServer::loadDb()
{
	QSqlQuery q;
	q.exec("SELECT * FROM room");
	
	while (q.next())
	{
		qint16 roomId = (qint16)q.value("id").toInt();
		mRooms.append(new QPRoom(this, roomId));
		mRoomLut[roomId] = mRooms.last();
		connect(this, SIGNAL(userJoinedRoom(const QPRoom*,QPConnection*)), mRooms.last(), SLOT(handleUserJoined(const QPRoom*,QPConnection*)));
		connect(this, SIGNAL(userLeftRoom(const QPRoom*,QPConnection*)), mRooms.last(), SLOT(handleUserLeft(const QPRoom*,QPConnection*)));
		connect(this, SIGNAL(userMoved(const QPRoom*,const QPConnection*)), mRooms.last(), SLOT(handleUserMoved(const QPRoom*,const QPConnection*)));
		connect(this, SIGNAL(roomBlowThru(const QPRoom*,QPBlowThru*)), mRooms.last(), SLOT(handleBlowThru(const QPRoom*,QPBlowThru*)));
		connect(this, SIGNAL(userTalked(const QPRoom*,QPMessage&)), mRooms.last(), SLOT(handleUserTalked(const QPRoom*,QPMessage&)));
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

	QThread *thread = new QThread(this);
	QTimer *timer = new QTimer(nullptr);
	timer->setInterval(mPing);
	timer->moveToThread(thread);
	connect(timer, SIGNAL(timeout()), this, SLOT(checkConnections()), Qt::DirectConnection);
	connect(thread, SIGNAL(started()), timer, SLOT(start()));
	thread->start();
	
	return true;
}

bool QPServer::loadConf(const QJsonObject &data)
{
	mName = data["name"].toString().toLatin1();
	mPort = (quint16)data["port"].toInt();
	mMediaUrl = data["mediaUrl"].toString().toLatin1();
	mPing = (quint32)data["pingIntervalSecs"].toInt() * 1000;
	mPong = (quint32)data["pongIntervalSecs"].toInt() * 1000;
	int maxThreadCount = data["maxThreadCount"].toInt();
	if (maxThreadCount > 2)
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
	if (data["unicodeEnabled"].toBool())
		mOptions |= Unicode;
	
	return true;
}

QSqlError QPServer::generateDefaultDb(QString path)
{
	mDb.setDatabaseName(path);
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
	
	createRoom("Lobby", QPRoom::DropZone, "lobby.png");
	
	qDebug("`qpserver.db` was not available, so it has been generated.\n");
	generatePassword(true);
	
	return QSqlError();
}

void QPServer::generatePassword(bool god)
{
	char *pwd = new char[8];
	pwd[7] = '\0';
	qsrand(QDateTime::currentDateTime().toTime_t());
	
	for (quint8 i = 0; i < 7; i++)
		pwd[i] = genPwdAsc[qrand() % qstrlen(genPwdAsc)];
	
	createPassword(pwd, god ? PWD_GOD : 0);
	qDebug("Your generated password is '%s' and will be overriden once a new password is set. You are recommended to write this down, as it is unobtainable once this message is cleared!\n", pwd);
	delete[] pwd;
}

QVariant QPServer::createRoom(const QString &name, qint32 flags, const QString &bg, const char *pwd, const QString &artist)
{
	QSqlQuery q;
	q.prepare("INSERT INTO room(id, name, flags, img_name, artist_name, password) VALUES(?, ?, ?, ?, ?, ?)");
	
	q.addBindValue(++mLastRoomId);
	q.addBindValue(name);
	q.addBindValue(flags);
	q.addBindValue(bg);
	q.addBindValue(artist == QString() ? QVariant(QVariant::String) : artist);
	
	if (pwd)
	{
		QByteArray encPwd(pwd);
		mCrypt.encrypt(encPwd.data(), encPwd.size());
		q.addBindValue(encPwd);
	}
	else
		q.addBindValue(QVariant(QVariant::ByteArray));
	
	q.exec();
	mRooms.append(new QPRoom(this, mLastRoomId));
	mRoomLut[mLastRoomId] = mRooms.last();
	connect(this, SIGNAL(userJoinedRoom(const QPRoom*,QPConnection*)), mRooms.last(), SLOT(handleUserJoined(const QPRoom*,QPConnection*)));
	connect(this, SIGNAL(userLeftRoom(const QPRoom*,QPConnection*)), mRooms.last(), SLOT(handleUserLeft(const QPRoom*,QPConnection*)));
	connect(this, SIGNAL(userMoved(const QPRoom*,const QPConnection*)), mRooms.last(), SLOT(handleUserMoved(const QPRoom*,const QPConnection*)));
	connect(this, SIGNAL(roomBlowThru(const QPRoom*,QPBlowThru*)), mRooms.last(), SLOT(handleBlowThru(const QPRoom*,QPBlowThru*)));
	return q.lastInsertId();
}

QVariant QPServer::createPassword(const char *pwd, quint8 flags)
{
	QByteArray encPwd(pwd);
	QPCrypt crypt;
	
	QSqlQuery q;
	q.prepare("INSERT INTO password(flags, checksum) VALUES(?, ?)");
	q.addBindValue(flags);
	
	mCrypt.encrypt(encPwd.data(), encPwd.size());
	QString pwHash(QCryptographicHash::hash(encPwd, QCryptographicHash::Sha256).toHex());
	q.addBindValue(pwHash);
	
	q.exec();
	return q.lastInsertId();
}

void QPServer::userStatus(QDataStream &ds, QPConnection *c)
{
	QPMessage ustatus(QPMessage::uSta, c->id());
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1 << c->status();
	
	ustatus = ba;
	ds << ustatus;
}

void QPServer::mediaUrl(QDataStream &ds, QPConnection *c)
{
	QPMessage http(QPMessage::HTTP, c->id());
	QByteArray ba;
	QDataStream ds1(&ba, QIODevice::WriteOnly);
	ds1.device()->reset();
	ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	
	ds1.writeRawData(mMediaUrl.data(), mMediaUrl.size());
	ds1 << (quint8)0;
	
	http = ba;
	ds << http;
}

void QPServer::userMove(QPConnection *c, QPMessage &msg)
{
	QByteArray ba(msg.data(), msg.size());
	QDataStream ds(ba);
	ds.device()->reset();
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

	qint16 x, y;
	ds >> x >> y;
	c->setPosition(x, y);
}

void QPServer::blowThru(QPBlowThru *blow)
{
	QPMessage msg(QPMessage::blow);
	msg << blow;

	if (blow->userCount() == QPBlowThru::Global)
		for (auto c: mConnections)
		{
			QDataStream ds(c->socket());
			ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
			ds << msg;
		}
	else
		for (qint32 i = 0; i < blow->userCount(); i++)
		{
			QDataStream ds(mUserLut[blow->user(i)]->socket());
			ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
			ds << msg;
		}

	delete blow;
}

void QPServer::logoff(QPConnection *c)
{
	mConnections.remove(mConnections.indexOf(c));
	mUserLut.remove(c->id());
	QPMessage bye(QPMessage::bye, c->id());
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::WriteOnly);
	ds.device()->reset();
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

	ds << (qint32)mConnections.size();
	bye = ba;

	for (auto u: mConnections)
	{
		QDataStream ds2(u->socket());
		ds2.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

		ds2 << bye;
	}

	emit userLeftRoom(mRoomLut[c->room()], c);
}

void QPServer::checkConnections()
{
	for (auto c: mConnections)
	{
		QPMessage ping(QPMessage::ping);
		QDataStream ds(c->socket());
		ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
		ds << ping;

		if (!c->socket()->waitForReadyRead(mPong))
		{
			qDebug("[%s] Connection timed out.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")));
			logoff(c);
		}
	}
}

void QPServer::handleNewConnection()
{
	QPConnection *c = new QPConnection(this, mServer->nextPendingConnection());
	connect(c, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));

	// MSG_TIYID
	QPMessage tiyid(QPMessage::tiyr, ++mUserCount);
	QDataStream ds(c->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);
	ds << tiyid;
	c->setId(mUserCount);

	if (!c->socket()->waitForReadyRead(mPong))
		qDebug("[%s] Connection timed out.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")));
}

void QPServer::handleReadyRead()
{
	QPMessage msg;
	QPConnection *c = qobject_cast<QPConnection*>(sender());
	QDataStream ds(c->socket());
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

	if (c->socket()->bytesAvailable() >= 12)
		ds >> msg;
	else
	{
		qWarning("[%s] %s:%u connection dropped because of incomplete packet data!", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
			qPrintable(c->socket()->peerAddress().toString()), c->socket()->peerPort());
		delete c;
		mUserCount--;
	}

	switch (msg.id())
	{
		case QPMessage::regi:
		{
			// MSG_LOGON
			{
				QByteArray ba(msg.data(), msg.size());
				QDataStream ds1(ba);
				ds1.device()->reset();
				ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

				ds1.skipRawData(9); // 8 bits of legacy data; also discard name length byte, QByteArray will auto determine
				char *idstr = new char[31];
				ds1.readRawData(idstr, 31);
				c->setUserName(idstr);
				delete[] idstr;

				quint8 slen;
				ds1 >> slen; // encrypted password might have null chars
				if (slen)
				{
					idstr = new char[slen];
					ds1.readRawData(idstr, slen);
					c->setWizardPassword(idstr, slen);
					delete[] idstr;
				}
				ds1.skipRawData(31 - slen);

				qint32 flags;
				ds1 >> flags;
				c->setAuxFlags(flags);

				ds1.skipRawData(20); // legacy data

				qint16 roomId;
				ds1 >> roomId;
				c->setRoom(roomId);

				idstr = new char[7];
				idstr[6] = '\0';
				ds1.readRawData(idstr, 6);
				c->setVendor(QPConnection::vendorFromString(idstr));
				delete[] idstr;

				ds1.skipRawData(24); // unused data
			}

			if ((mAccessFlags & AllowInsecureClients) || c->isSecureVendor())
			{
				mConnections.append(c);
				mUserLut[c->id()] = c;
				qDebug("[%s] %s logged in from %s:%u using %s client on %s.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
					c->userName(), qPrintable(c->socket()->peerAddress().toString()), c->socket()->peerPort(), QPConnection::vendorToString(c->vendor()),
					c->osToString());
			}
			else
			{
				qDebug("[%s] %s:%u connection dropped because of use of insecure client: %s.", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
					qPrintable(c->socket()->peerAddress().toString()), c->socket()->peerPort(), QPConnection::vendorToString(c->vendor()));
				delete c;
				mUserCount--;
				return;
			}

			emit userLoggedOn(c);
			qsrand(QDateTime::currentDateTime().toTime_t());
			c->setFace(qrand() % 16);
			c->setColor(qrand() % 16);
			c->setPosition(0, 0); // placeholder, todo: fetch room bg width/height and randomise pos
			c->setRoom(1);
			c->setStatus(0);

			// MSG_SERVERVERSION
			{
				QPMessage version(QPMessage::vers, 0x00010016); // version number was packet sniffed from PServer 4.5.1
				ds << version;
			}

			// MSG_SERVERINFO
			{
				QPMessage sinfo(QPMessage::sinf, c->id());
				QByteArray ba;
				QDataStream ds1(&ba, QIODevice::WriteOnly);
				ds1.device()->reset();
				ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

				quint8 slen = (quint8)qstrlen(mName);
				ds1 << mAccessFlags << slen;
				ds1.writeRawData(mName, slen);
				// according to packet sniffing, options + ul/dl caps aren't sent?
				sinfo = ba;
				ds << sinfo;
			}

			userStatus(ds, c);

			// MSG_USERLOG
			{
				QPMessage ulog(QPMessage::log, c->id());
				QByteArray ba;
				QDataStream ds1(&ba, QIODevice::WriteOnly);
				ds1.device()->reset();
				ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

				ds1 << (qint32)mConnections.size();
				ulog = ba;

				for (auto u: mConnections)
				{
					QDataStream ds2(u->socket());
					ds2.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

					ds2 << ulog;
				}
			}

			mediaUrl(ds, c);
			mRoomLut[c->room()]->description(c, false);
			mRoomLut[c->room()]->users(c);
			emit userJoinedRoom(mRoomLut[c->room()], c);

			break;
		}
		case QPMessage::sRom:
		{
			emit roomEdited(mRoomLut[c->room()]);
			break;
		}
		case QPMessage::uLoc:
		{
			userMove(c, msg);
			emit userMoved(mRoomLut[c->room()], c);
			break;
		}
		case QPMessage::blow:
		{
			QPBlowThru *blow = new QPBlowThru();
			msg >> blow;
			if (blow->userCount() == QPBlowThru::Local)
				emit roomBlowThru(mRoomLut[c->room()], blow);
			else
				blowThru(blow);
			break;
		}
		case QPMessage::xtlk:
		{
			if (mOptions & ChatLog)
			{
				QByteArray ba(msg.data(), msg.size());
				QDataStream ds1(ba);
				ds1.device()->reset();
				ds1.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

				qint16 len;
				ds1 >> len;
				len -= 2; // 2 extra null chars ?
				char text[len];
				ds1.readRawData(text, len);
				mCrypt.decrypt(text, len-1);
				qDebug("[%s - %s] %s: %s", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
					mRoomLut[c->room()]->name(), c->userName(), text);
				mCrypt.encrypt(text, len-1);
			}
			emit userTalked(mRoomLut[c->room()], msg);
			break;
		}
		case QPMessage::ping:
		{
			msg.setId(QPMessage::pong);
			ds << msg;
			break;
		}
		case QPMessage::bye:
		{
			logoff(c);
			break;
		}
		case QPMessage::pong:
			break;
		default:
			qWarning("[%s] %s:%u has sent an unknown or unsupported packet (%X). Ignoring...", qPrintable(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss A")),
				qPrintable(c->socket()->peerAddress().toString()), c->socket()->peerPort(), msg.id());
	}
}
