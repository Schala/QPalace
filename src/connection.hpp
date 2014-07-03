#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <QByteArray>
#include <QQueue>
#include <QSharedData>
#include <QtGlobal>

class QPConnection final: public QSharedData
{
private:
	QPRoom *mRoom;
	QQueue<QPMessage*> mMsgQueue;
	qint32 mID;
	QByteArray mUserName;
	QPRegistration mReg;
};

#endif // _CONNECTION_H
