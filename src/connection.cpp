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
	
	mProps = new QPAssetSpec[9];
	for (quint8 i = 0; i < 9; i++)
		mProps[i].id = mProps[i].crc = 0;
}
#else
QPConnection::QPConnection(QTcpSocket *sock): mSocket(sock)
{
	mProps = new QPAssetSpec[9];
	
	for (quint8 i = 0; i < 9; i++)
		mProps[i].id = mProps[i].crc = 0;
}
#endif // SERVER

QPConnection::~QPConnection()
{
	delete[] mProps;
	mSocket->abort();
	delete mSocket;
}

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
		case QPConnection::Vendor::PalaceChat:
			return "PalaceChat";
		case QPConnection::Vendor::OpenPalace:
			return "OpenPalace";
		case QPConnection::Vendor::PalaceViewer:
			return "ThePalace Viewer";
		case QPConnection::Vendor::Phalanx:
			return "Phalanx";
		case QPConnection::Vendor::InstantPalace:
			return "InstantPalace";
		case QPConnection::Vendor::QPalace:
			return "QPalace";
		default:
			return "unknown";
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
	qint32 flags = mAuxFlags & 0xff;
	
	switch (flags)
	{
		case Win32:
			return "Windows (32-bit)";
		case Java:
			return "Java";
		case MacIntel:
			return "Mac OS X";
		case Win64:
			return "Windows (64-bit)";
		case LinuxX86:
			return "Linux (x86)";
		case LinuxX86_64:
			return "Linux (x86_64)";
		case LinuxARM:
			return "Linux (ARM)";
		case AndroidARMv5:
			return "Android (ARMv5)";
		case AndroidARMv7:
			return "Android (ARMv7)";
		case AndroidX86:
			return "Android (x86)";
		case Dalvik:
			return "Dalvik";
		case iOS:
			return "iOS";
		case iOSSim:
			return "iOS Simulator";
		case WinRT:
			return "Windows RT";
		case CLR:
			return ".NET";
		case Python:
			return "Python";
		case Ruby:
			return "Ruby";
		case Flash:
			return "Flash";
		case Web:
			return "the web";
		default:
			return "an unknown machine";
	}
}

#ifndef QT_NO_DEBUG
QString QPConnection::loginDebugInfo() const
{
	QString info;
	QTextStream ts(&info);

#ifndef SERVER
	ts << "crc = " << mReg.crc() << '\n';
	ts << "counter = " << mReg.counter() << '\n';
#endif // SERVER
	ts << "userName = " << mUserName << '\n';
	ts << "wizPassword = " << (mWizardPwd.isEmpty() ? "(no password given)" : mWizardPwd) << '\n';
	ts << "auxFlags = "; hex(ts); ts << mAuxFlags << '\n'; dec(ts);
#ifndef SERVER
	ts << "puidCtr = " << mUid.counter() << '\n';
	ts << "puidCRC = " << mUid.crc() << '\n';
#endif // SERVER
	ts << "desiredRoom = " << mRoom << '\n';
	ts << "reserved[6] = " << mRawVendor << '\n';
	
	return info;
}
#endif // QT_NO_DEBUG
