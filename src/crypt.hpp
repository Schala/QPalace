#ifndef _CRYPT_H
#define _CRYPT_H

#include <QByteArray>

class QPCryptEngine final {
public:
	QPCryptEngine();
	~QPCryptEngine();
	void encrypt(QByteArray &buf, uint len = 254);
	void decrypt(QByteArray &buf, uint len = 254);
private:
	int mKey, *mLUT;
};

#endif // _CRYPT_H
