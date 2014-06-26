#ifndef _REGISTRATION_H
#define _REGISTRATION_H

#include <QByteArray>

class QPRegistration final {
public:
	//QPRegistration();
	QPRegistration(const QByteArray &regcode);
	
	inline uint counter() const {
		return mCounter;
	};
	
	inline uint crc() const {
		return mCRC;
	};
	
	static QByteArray generate();
	//static QByteArray serverGen();
private:
	uint mCounter, mCRC;
	void computeCRC(uint seed);
};

#endif // _REGISTRATION_H
