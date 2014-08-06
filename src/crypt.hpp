#ifndef _CRYPT_H
#define _CRYPT_H

#include <QtGlobal>

class QPCrypt final
{
public:
	QPCrypt();
	void encrypt(char *buf, quint32 len = 254);
	void decrypt(char *buf, quint32 len = 254);
private:
	qint32 mKey, mLut[512];
};

#endif // _CRYPT_H
