#include "connection.hpp"

#ifndef SERVER
QPConnection::QPConnection(QTcpSocket *sock, const char *name, const QPRegistration &reg, AuxFlags auxFlags,
	const QPRegistration &uid, qint16 initRoom, const char *wizpwd): mReg(reg), mUserName(name), mSocket(sock),
	mUid(uid), mAuxFlags(auxFlags), mVendor(QPConnection::Vendor::QPalace)
{
	mWizardPwd = wizpwd ? wizpwd : QByteArray();
}
#endif // SERVER

QPConnection::~QPConnection()
{
	/*if (!mMsgQueue.isEmpty())
	{
		for (auto m: mMsgQueue)
			delete m;
		mMsgQueue.clear();
	}*/
	mSocket->abort();
	delete mSocket;
}

QPConnection::Vendor QPConnection::vendorFromString(const char *str)
{
	if (qstrncmp(str, "PC", 2) == 0) // PalaceChat has its version in its signature, so ignore that
		return QPConnection::Vendor::PalaceChat;
	else if (qstrncmp(str, "OPNPAL", 6) == 0)
		return QPConnection::Vendor::OpenPalace;
	else if (qstrncmp(str, "QtPAL1", 6) == 0)
		return QPConnection::Vendor::QPalace;
	else if (qstrncmp(str, "350211", 6) == 0)
		return QPConnection::Vendor::LinPal;
	else
		return QPConnection::Vendor::Unknown;
}

const char* QPConnection::vendorToString(QPConnection::Vendor vendor)
{
	switch (vendor)
	{
		case QPConnection::Vendor::PalaceChat:
			return "PalaceChat";
		case QPConnection::Vendor::OpenPalace:
			return "OpenPalace";
		case QPConnection::Vendor::LinPal:
			return "LinPal";
		case QPConnection::Vendor::Phalanx:
			return "Phalanx";
		case QPConnection::Vendor::ThePalace:
			return "ThePalace";
		case QPConnection::Vendor::InstantPalace:
			return "InstantPalace";
		case QPConnection::Vendor::QPalace:
			return "QPalace";
		default:
			return "unknown";
	}
}
