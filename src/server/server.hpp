#ifndef _SERVER_H
#define _SERVER_H

#include <QJsonObject>
#include <QSet>
#include <QTcpServer>

class QPServer final: public QTcpServer {
	Q_OBJECT
public:
	QPServer(): QObject(nullptr);
	~QPServer();
	bool load(const QJsonObject &data);
	
	inline ushort port() const {
		return port_;
	}
private:
	QSet<QPRoomPtr> mRooms;
	ushort mPort;
};

#endif // _SERVER_H
