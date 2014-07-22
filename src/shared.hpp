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

#endif // _SHARED_H
