#ifndef _CODEC_H
#define _CODEC_H

#include <QtGlobal>

class QPCodec final
{
public:
	QPCodec();
	void encode(char *buf, quint32 len = 254);
	void decode(char *buf, quint32 len = 254);
private:
	qint32 mKey, mLut[512];
};

#endif // _CODEC_H
