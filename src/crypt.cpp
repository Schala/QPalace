#include "crypt.hpp"

QPCrypt::QPCrypt()
{
	mKey = 0xa2c2a;
	qint32 quo, rem, k;
	
	for (quint16 i = 0; i < 512; i++)
	{
		quo = mKey / 0x1f31d;
		rem = mKey % 0x1f31d;
		k = 16807 * rem - 2836 * quo;
		mKey = k > 0 ? k : k + 0x7fffffff;
		mLut[i] = (qint32)(((double)mKey / (double)2147483647) * (double)256);
	}
	mKey = 1;
}

void QPCrypt::encrypt(char *buf, quint32 len)
{
	qint32 lastChar = 0, rc = 0;
	quint8 b;
	
	for (qint32 i = len-1; i >= 0; i--)
	{
		b = buf[i];
		buf[i] = (quint8)(b ^ mLut[rc++] ^ lastChar);
		lastChar = (quint8)(buf[i] ^ mLut[rc++]);
	}
}

void QPCrypt::decrypt(char *buf, quint32 len)
{
	quint8 lastChar = 0, b;
	qint32 rc = 0;
	
	for (qint32 i = len-1; i >= 0; i--)
	{
		b = buf[i];
		buf[i] = (quint8)(b ^ mLut[rc++] ^ lastChar);
		lastChar = (quint8)(b ^ mLut[rc++]);
	}
}
