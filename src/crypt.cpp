#include "crypt.hpp"

QPCryptEngine::QPCryptEngine() {
	mKey = 0xa2c2a;
	mLUT = new int[512];
	int quo, rem, k;
	
	for (int i = 0; i < 512; i++) {
		quo = mKey / 0x1f31d;
		rem = mKey % 0x1f31d;
		k = 16807 * rem - 2836 * quo;
		mKey = k > 0 ? k : k + 0x7fffffff;
		mLUT[i] = (int)(((double)mKey / (double)2147483647) * (double)256);
	}
	mKey = 1;
}

QPCryptEngine::~QPCryptEngine() {
	if (mLUT)
		delete[] mLUT;
}

void QPCryptEngine::encrypt(QByteArray &buf, uint len) {
	int lastChar = 0, rc = 0;
	uchar b;
	
	for (int i = len-1; i >= 0; i--) {
		b = buf[i];
		buf[i] = (uchar)(b ^ mLUT[rc++] ^ lastChar);
		lastChar = (uchar)(buf[i] ^ mLUT[rc++]);
	}
}

void QPCryptEngine::decrypt(QByteArray &buf, uint len) {
	uchar lastChar = 0, b;
	int rc = 0;
	
	for (int i = len-1; i >= 0; i--) {
		b = buf[i];
		buf[i] = (uchar)(b ^ mLUT[rc++] ^ lastChar);
		lastChar = (uchar)(b ^ mLUT[rc++]);
	}
}
