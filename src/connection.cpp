#ifndef QT_NO_DEBUG
#include <QTextStream>
#endif // QT_NO_DEBUG
#include "connection.hpp"

#ifndef SERVER
QPConnection::QPConnection(QTcpSocket *sock, const char *name, const QPRegistration &reg, AuxFlags auxFlags,
	const QPRegistration &uid, qint16 initRoom, const char *wizpwd): mReg(reg), mUserName(name), mSocket(sock),
	mUid(uid), mAuxFlags(auxFlags), mVendor(QPConnection::Vendor::QPalace)
{
	mWizardPwd = wizpwd ? wizpwd : QByteArray();

	for (quint8 i = 0; i < 9; i++)
		mProps[i].id = mProps[i].crc = 0;
}
#else
QPConnection::QPConnection(QObject *parent, QTcpSocket *sock): QObject(parent), mSocket(sock)
{
	for (quint8 i = 0; i < 9; i++)
		mProps[i].id = mProps[i].crc = 0;
	connect(mSocket, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
}
#endif // SERVER

QPConnection::Vendor QPConnection::vendorFromString(const char *str)
{
	if (qstrncmp(str, "PC", 2) == 0) // PalaceChat has its version in its signature, so ignore that
		return QPConnection::Vendor::PalaceChat;
	else if (qstrncmp(str, "OPNPAL", 6) == 0)
		return QPConnection::Vendor::OpenPalace;
	else if (qstrncmp(str, "350211", 6) == 0)
		return QPConnection::Vendor::PalaceViewer;
	else if (qstrncmp(str, "QtPAL1", 6) == 0)
		return QPConnection::Vendor::QPalace;
	else
		return QPConnection::Vendor::Unknown;
}

const char* QPConnection::vendorToString(QPConnection::Vendor vendor)
{
	switch (vendor)
	{
		case QPConnection::Vendor::PalaceChat: return "PalaceChat";
		case QPConnection::Vendor::OpenPalace: return "OpenPalace";
		case QPConnection::Vendor::PalaceViewer: return "ThePalace Viewer";
		case QPConnection::Vendor::Phalanx: return "Phalanx";
		case QPConnection::Vendor::InstantPalace: return "InstantPalace";
		case QPConnection::Vendor::QPalace: return "QPalace";
		default: return "unknown";
	}
}

bool QPConnection::isSecureVendor() const
{
	switch (mVendor)
	{
		case QPConnection::Vendor::PalaceChat:
		case QPConnection::Vendor::OpenPalace:
		case QPConnection::Vendor::QPalace:
			return true;
		default:
			return false;
	}
}

const char* QPConnection::osToString() const
{
	qint32 flags = mAux & 0xff;
	
	switch (flags)
	{
		case Win32: return "Windows (32-bit)";
		case Java: return "Java";
		case MacX86: return "Mac OS X";
		case Win64: return "Windows (64-bit)";
		case LinuxX86: return "Linux (x86)";
		case LinuxX86_64: return "Linux (x86_64)";
		case Flash: return "Flash";
		default: return "old or unknown platform";
	}
}

void QPConnection::handleReadyRead()
{
	emit readyRead();
}

QPMessage& operator<<(QPMessage &msg, const QPConnection *c)
{
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::WriteOnly);
	ds.device()->reset();
	ds.setByteOrder(Q_BYTE_ORDER == Q_BIG_ENDIAN ? QDataStream::BigEndian : QDataStream::LittleEndian);

	ds << c->id();
	ds << c->position().x << c->position().y;
	for (quint8 j = 0; j < 9; j++)
		ds << c->prop(j).id << c->prop(j).crc;
	ds << c->room();
	ds << c->face();
	ds << c->color();
	ds << (qint32)0; // not used

	qint16 activeProps = 0;
	for (quint8 j = 0; j < 9; j++)
		if (c->prop(j).id && c->prop(j).crc)
			activeProps++;
	ds << activeProps;

	quint8 nlen = qstrlen(c->userName());
	ds << nlen;
	ds.writeRawData(c->userName(), nlen);
	for (quint8 j = 0; j < (31 - nlen); j++)
		ds << (quint8)0;

	msg << ba;
	return msg;
}
