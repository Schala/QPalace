#include <QtGlobal>
#include <QByteArray>
#include "../src/codec.hpp"

int main(int argc, char *argv[]) {
	if (argc > 1) {
		QByteArray s(argv[1]);
		QPCodec enc;
		enc.encode(s.data(), s.size());
		qDebug("Encrypted: %s", s.data());
		enc.decodec(s.data(), s.size());
		qDebug("Decrypted: %s", s.data());
	}
	return 0;
}
