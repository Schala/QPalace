#include <QtGlobal>
#include <QByteArray>
#include "../src/crypt.hpp"

int main() {
	QByteArray b("\x13bh)\x8DT\x84S");
	QPCryptEngine enc;
	enc.decrypt(b.data(), b.size());
	qDebug("Decrypted: %s", b.data());
	enc.encrypt(b.data(), b.size());
	qDebug("Encrypted: %s", b.data());
	return 0;
}
