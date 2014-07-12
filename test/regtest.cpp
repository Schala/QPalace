#include <QByteArray>
#include "../src/registration.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
	QPRegistration *reg;
	QByteArray code;
	
	//9YAT-C8MM-GJVZL and FBF2-DS78-9NK7W were tested
	//reg = argc > 1 ? new QPRegistration(argv[1]) : new QPRegistration();
	if (argc > 1) {
		reg = new QPRegistration(QByteArray(argv[1])) ;
		std::cout << "Counter/CRC: " << std::hex;
		std::cout << reg->counter() << " " << reg->crc() << std::endl;
		delete reg;
	}
	code = QPRegistration::generate();
	std::cout << "Random registration code: " << code.data() << std::endl;
	reg = new QPRegistration(code);
	std::cout << "Counter/CRC: " << std::hex;
	std::cout << reg->counter() << " " << reg->crc() << std::endl;
	delete reg;
	
	code = QPRegistration::serverGen();
	std::cout << "Random server registration code: " << code.data() << std::endl;
	return 0;
}
