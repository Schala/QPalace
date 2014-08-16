#include <QtGlobal>
#include "../src/client/userid.hpp"

int main(int argc, char *argv[]) {
	QPUserId *reg;
	char *code;
	
	//9YAT-C8MM-GJVZL and FBF2-DS78-9NK7W were tested
	//reg = argc > 1 ? new QPUserId(argv[1]) : new QPUserId();
	if (argc > 1) {
		reg = new QPUserId(argv[1]) ;
		qDebug("Counter/CRC: %X / %X", reg->counter(), reg->crc());
		delete reg;
	}
	code = (char*)QPUserId::generate();
	qDebug("Random registration code: %s", code);
	reg = new QPUserId(code);
	qDebug("Counter/CRC: %X / %X", reg->counter(), reg->crc());
	delete reg;
	
	return 0;
}
