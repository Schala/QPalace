#include <QtGlobal>
#include <QByteArray>
#include "../src/crypt.hpp"

int main(int argc, char *argv[]) {
	if (argc > 1) {
		QByteArray s(argv[1]);
		QPCryptEngine enc;
		enc.encrypt(s.data(), s.size());
		qDebug("Encrypted: %s", s.data());
		enc.decrypt(s.data(), s.size());
		qDebug("Decrypted: %s" << s.data());
	}
	return 0;
}
