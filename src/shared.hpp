#ifndef _SHARED_H
#define _SHARED_H

#include <QtGlobal>

struct QPPoint final
{
	qint16 x, y;
};

struct QPAssetSpec final
{
	qint32 id;
	quint32 crc;
};

inline quint32 argbToUint(quint32 a, quint32 r, quint32 g, quint32 b)
{
	quint32 c = 0;
	c |= a << 24;
	c |= r << 16;
	c |= g << 8;
	c |= b;
	return c;
}

#endif // _SHARED_H
