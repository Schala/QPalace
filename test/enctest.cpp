#include <iostream>
#include <QLatin1String>
#include "../src/crypt.hpp"

int main(int argc, char *argv[]) {
	if (argc > 1) {
		QLatin1String s(argv[1]);
		QPCryptEngine enc;
		enc.encrypt(s, s.size());
		std::cout << "Encrypted: " << s.data() << std::endl;
		enc.decrypt(s, s.size());
		std::cout << "Decrypted: " << s.data() << std::endl;
	}
	return 0;
}
