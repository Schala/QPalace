#include <QtGlobal>
#include <QByteArray>
#include "../src/codec.hpp"

int main() {
	QByteArray b("\x13bh)\x8DT\x84S");
	QPCodec enc;
	enc.decode(b.data(), b.size());
	qDebug("Decrypted: %s", b.data());
	enc.encode(b.data(), b.size());
	qDebug("Encrypted: %s", b.data());
	return 0;
}
