#include <iostream>
#include <QByteArray>
#include "../src/crypt.hpp"

int main() {
	QByteArray b("\x95\x4f\x95\x43\x0");
	QPCryptEngine enc;
	enc.decrypt(b, b.size());
	std::cout << "Decrypted: " << b.data() << std::endl;
	enc.encrypt(b, b.size());
	std::cout << "Encrypted: " << b.data() << std::endl;
	return 0;
}
