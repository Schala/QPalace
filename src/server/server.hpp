#ifndef _SERVER_H
#define _SERVER_H

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
//#include "../room.hpp"

class QPServer final: public QObject
{
	Q_OBJECT
public:
	QPServer(QObject *parent = nullptr);
	~QPServer();
	bool loadConf(const QJsonObject &data);
	/*QVariant createRoom(QSqlQuery &q, qint16 id, const QString &name, QPRoom::Flags flags,
		const QString &bg, const QString &pwd = QString(), const QString &artist = QString());*/
	QVariant createPassword(QSqlQuery &q, const char *pwd, quint8 flags = 0);
	inline quint16 port() const { return mServer->serverPort(); }
	inline const char* name() const { return mName; }
	inline bool start() const { return mServer->listen(QHostAddress::Any, mPort); }
private slots:
	void handleNewConnection();
private:
//	QVector<QPRoomPtr> mRooms;
	QVector<QPConnectionPtr> mConnections;
	QSqlDatabase mDb;
	QTcpServer *mServer;
	quint16 mPort;
	QByteArray mName;
	qint32 mUserCount; // for ID assignment
	
	QSqlError genDefaultDb();
	void generatePassword(QSqlQuery &q, bool god = false);
};

#endif // _SERVER_H
