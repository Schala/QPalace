#ifndef _CRYPT_H
#define _CRYPT_H

#include <QtGlobal>

class QPCryptEngine final
{
public:
	QPCryptEngine();
	~QPCryptEngine();
	void encrypt(char *buf, quint32 len = 254);
	void decrypt(char *buf, quint32 len = 254);
private:
	qint32 mKey, *mLut;
};

#endif // _CRYPT_H
