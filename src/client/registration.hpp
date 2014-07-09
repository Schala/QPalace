#ifndef _REGISTRATION_H
#define _REGISTRATION_H

#include <QtGlobal>

class QPRegistration final
{
public:
	//QPRegistration();
	QPRegistration(const char *regcode);
#ifdef SERVER
	QPRegistration() {}
	QPRegistration(quint32 counter, quint32 crc): mCounter(counter), mCrc(crc) {}
#endif // SERVER
	inline quint32 counter() const { return mCounter; }
	inline quint32 crc() const { return mCrc; };
	static const char* generate();
	//static const char* serverGen();
private:
	quint32 mCounter, mCrc;
	void computeCrc(quint32 seed);
};

#endif // _REGISTRATION_H
