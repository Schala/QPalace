#ifndef BLOWTHRU_HPP
#define BLOWTHRU_HPP

#include <QByteArray>
#include <QtGlobal>

#include "message.hpp"

class QPBlowThru final
{
	friend QPMessage& operator>>(QPMessage &msg, QPBlowThru *blow);
public:
	enum
	{
		Global = -1,
		Local = 0
	};
	QPBlowThru(): mRelay(0), mUserCount(Global), mUserIds(nullptr), mTag(0) {}
	QPBlowThru(qint32 relay, qint32 userCount, quint32 tag);
	~QPBlowThru();
	inline quint32 size() const { return mData.size(); }
	inline const char* data() const { return mData; }
	inline void setData(const QByteArray &data) { mData = data; }
	inline quint32 tag() const { return mTag; }
	inline qint32 userCount() const { return mUserCount; }
	inline qint32 relay() const { return mRelay; }
	inline qint32 user(qint32 i) const { if (mUserIds) return mUserIds[i]; else return 0; }
	inline const qint32* users() const { return mUserIds; }
	inline void addUser(qint32 id) { if ((mIdCap < mUserCount) && mUserIds) mUserIds[mIdCap++] = id; }
private:
	QByteArray mData;
	qint32 mRelay, mUserCount, *mUserIds, mIdCap;
	quint32 mTag;
};

QPMessage& operator<<(QPMessage &msg, const QPBlowThru *blow);
QPMessage& operator>>(QPMessage &msg, QPBlowThru *blow);

#endif // BLOWTHRU_HPP
