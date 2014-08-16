#ifndef _USERID_H
#define _USERID_H

#include <QtGlobal>

class QPUserId final
{
public:
	QPUserId();
	QPUserId(const char *regcode);
	inline quint32 counter() const { return mCounter; }
	inline quint32 crc() const { return mCrc; };
	static const char* generate();
private:
	quint32 mCounter, mCrc;
	void computeCrc(quint32 seed);
};

#endif // _USERID_H
