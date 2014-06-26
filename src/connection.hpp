#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <QByteArray>
#include <QQueue>
#include <QSharedData>

class QPConnection final: public QSharedData {
private:
	QPRoom *mRoom;
	QQueue<QPMessage*> mMsgQueue;
	int mID;
	QByteArray mUserName;
	QPRegistration mReg;
};

#endif // _CONNECTION_H
