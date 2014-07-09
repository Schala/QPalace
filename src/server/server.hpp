#ifndef _SERVER_H
#define _SERVER_H

#include <QJsonObject>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QTcpServer>
#include <QtGlobal>
#include <QVariant>

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
	inline quint16 port() const { return mServer->serverPort(); }
	inline const char* name() const { return mName; }
	inline bool start() const { return mServer->listen(QHostAddress::Any, mPort); }
private slots:
	void handleNewConnection();
private:
//	QSet<QPRoomPtr> mRooms;
	QSet<QPConnectionPtr> mConnections;
	QSqlDatabase mDb;
	QTcpServer *mServer;
	quint16 mPort;
	QByteArray mName;
	qint32 mUserCount; // for ID assignment
	
	QSqlError genDefaultDb();
};

#endif // _SERVER_H
