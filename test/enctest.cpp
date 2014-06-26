#include <iostream>
#include <QByteArray>
#include "../src/crypt.hpp"

int main(int argc, char *argv[]) {
	if (argc > 1) {
		QByteArray b(argv[1]);
		QPCryptEngine enc;
		enc.encrypt(b, b.size());
		std::cout << "Encrypted: " << b.data() << std::endl;
		enc.decrypt(b, b.size());
		std::cout << "Decrypted: " << b.data() << std::endl;
	}
	return 0;
}
