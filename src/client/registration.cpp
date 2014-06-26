#include <QDataStream>
#include <QDateTime>
#include <QRegExp>
#include <QtEndian>
#include "registration.hpp"

/*static const uint CRCMASK[] = {
	0xebe19b94, 0x7604de74, 0xe3f9d651, 0x604fd612, 0xe8897c2c, 0xadc40920, 0x37ecdfb7, 0x334989ed, 0x2834c33b, 0x8bd2fe15, 0xcbf001a7, 0xbd96b9d6, 0x315e2ce0, 0x4f167884, 0xa489b1b6, 0xa51c7a62,
	0x54622636, 0xbc016fc, 0x68de2d22, 0x3c9d304c, 0x44fd06fb, 0xbbb3f772, 0xd637e099, 0x849aa9f9, 0x5f240988, 0xf8373bb7, 0x30379087, 0xc7722864, 0xb0a2a643, 0xe3316071, 0x956fed7c, 0x966f937d,
	0x9945ae16, 0xf0b237ce, 0x223479a0, 0xd8359782, 0x5ae1b89, 0xe3653292, 0xc34eea0d, 0x2691dfc2, 0xe9145f51, 0xd9aa7f35, 0xc7c4344e, 0x4370eba1, 0x1e43833e, 0x634bcf18, 0xc50e26b, 0x6492118,
	0xf78b8bfe, 0x5f2bb95c, 0xa3eb54a6, 0x1e15a2f0, 0x6cc01887, 0xde4e7405, 0x1c1d7374, 0x85757feb, 0xe372517e, 0x9b9979c7, 0xf37807e8, 0x18f97235, 0x645a149b, 0x9556c6cf, 0xf389119e, 0x1d6cbf85,
	0xa9760ce5, 0xa985c5ff, 0x5f4db574, 0x13176cac, 0x2f14aa85, 0xf520832c, 0xd21ee917, 0x6f307a5b, 0xc1fb01c6, 0x19415378, 0x797fa2c3, 0x24f42481, 0x4f652c30, 0x39bc02ed, 0x11eda1d7, 0x8c79a136,
	0x6bd37a86, 0x80b354ee, 0xc424e066, 0xaae16427, 0x6bd3be12, 0x868d8e37, 0xd1d43c54, 0x4d62081f, 0x433056d7, 0xf2e4cb02, 0x43fc5a2, 0x9da58ca4, 0x1ed63321, 0x20679f26, 0xb38a4758, 0x846419f7,
	0x6bdc6352, 0xabf2c24d, 0x40ac386c, 0x27588588, 0x5e1ab2e5, 0x76bdead4, 0x71444d32, 0x2fc6084, 0x92db41fb, 0xef86baeb, 0xf7d8572a, 0xb75aeabf, 0x84dc5c93, 0xcbc13881, 0x641d6e73, 0xcb27a99,
	0xded369a6, 0x617e5dfa, 0x248bd13e, 0xb8596d66, 0x9b36a9fa, 0x52edaf1c, 0x3c659784, 0x146df599, 0x109fcae8, 0xc9ed4841, 0xbf593f49, 0xc94a6e73, 0x5afa0d2f, 0xb2035002, 0xcab31104, 0x7c4f5a82,
	0xeac93638, 0x63fc5385, 0xdf0cae06, 0x26e55be3, 0x2921b9b8, 0xb80b3408, 0x917e137d, 0x127a48bc, 0xe031858a, 0x722213d7, 0x2dbc96fa, 0x5359f112, 0xab256019, 0x6e2a756e, 0x4dc62f76, 0x268832de,
	0x5980e578, 0xd338b668, 0xeee2e4d7, 0x1fff8fc6, 0x9b17ed10, 0xf3e6be0f, 0xc1ba9d78, 0xbb8693c5, 0x24d57ec0, 0x5d640aed, 0xee87979b, 0x96323e11, 0xccbc1601, 0xe83f43b, 0x2c2f7495, 0x5f150b2a,
	0x710a77e2, 0x281b51dc, 0x2385d03c, 0x67239bff, 0xa719e8f9, 0x21c3b9de, 0x26489c22, 0xde68989, 0xca758f0d, 0x417e8cd2, 0x67ed61f8, 0xd15fc001, 0x3ba2f272, 0x57e2f7a9, 0xe723b883, 0x914e43e1,
	0x71aa5b97, 0xfceb1be1, 0x7ffa4fd9, 0x67a0b494, 0x5e1c741e, 0xc8c2a5e6, 0xe13ba068, 0x24525548, 0x397a9cf6, 0x3dddd4d6, 0xb626234c, 0x39e7b04d, 0x36ca279f, 0x89aea387, 0xcfe93789, 0x4e1761b,
	0x9d620edc, 0x6e9df1e7, 0x4a15dfa6, 0xd44641ac, 0x39796769, 0x6d062637, 0xf967af35, 0xddb4a233, 0x48407280, 0xa9f22e7e, 0xd9878f67, 0xa05b3bc1, 0xe8c9237a, 0x81cec53e, 0x4be53e70, 0x60308e5e,
	0xf03de922, 0xa712af7b, 0xbb6168b4, 0xcc6c15b5, 0x2f202775, 0x304527e3, 0xd32bc1e6, 0xba958058, 0xa01f7214, 0xc6e8d190, 0xab96f14b, 0x18669984, 0x4f93a385, 0x403b5b40, 0x580755f1, 0x59de50e8,
	0xf746729f, 0xff6f7d47, 0x8022ea34, 0xb24b0bcd, 0xf687a7cc, 0x7e95bab3, 0x8dc1583d, 0xb443fe9, 0xe6e45618, 0x224d746f, 0xf30624bb, 0xb7427258, 0xc78e19bf, 0xd1ee98a6, 0x66be7d3a, 0x791e342f,
	0x68cbaab0, 0xbbb5355d, 0x8dda9081, 0xdc2736dc, 0x573355ad, 0xc3ffec65, 0xe97f0270, 0xc6a265e8, 0xd9d49152, 0x4bb35bdb, 0xa1c7bbe6, 0x15a3699a, 0xe69e1eb5, 0x7cdda410, 0x488609df, 0xd19678d3
};*/

static const QByteArray code_asc("ABCDEFGHJKLMNPQRSTUVWXYZ23456789");

/*QPRegistration::QPRegistration() {
	uint seed = QDateTime::currentDateTime.toTime_t();
	computeCRC(seed);
	mCounter = (seed ^ 0x9602c9bf) ^ mCRC;
}

void QPRegistration::computeCRC(uint seed) {
	mCRC = 0xa95ade76;
	uint current_byte;
	seed = qToBigEndian(seed);
	
	for (int i = 0; i < 4; i++) {
		current_byte = seed & 0xff;
		mCRC = ((mCRC << 1) | (((mCRC & 0x80000000) == 0) ? 0 : 1)) ^ CRCMASK[current_byte];
		seed >>= 8;
	}
}*/

QPRegistration::QPRegistration(const QByteArray &regcode) {
	QString code = regcode;
	QRegExp rx("[^ABCDEFGHJKLMNPQRSTUVWXYZ23456789]");
	int nbits = 64, sn = 0, ocnt = 0, mask = 0x0080, charidx = 0, pos = 0;
	uint temp = 0;
	QByteArray s(8, 0);
	
	code = code.toUpper();
	code.replace(rx, "");
	
	while(nbits--) {
		if (ocnt == 0) {
			sn = code_asc.indexOf(code.at(charidx++));
			ocnt = 5;
		}
		if (sn & 0x10) {
			temp = (uint)(s[pos] & 0xff);
			temp |= mask;
			temp &= 0xff;
			s[pos] = (uchar)temp;
		}
		sn <<= 1;
		sn &= 0xffff;
		--ocnt;
		mask >>= 1;
		mask &= 0xffff;
		if (mask == 0) {
			mask = 0x80;
			++pos;
			s[pos] = 0;
		}
	}
	QDataStream sbuf(s);
	sbuf.device()->reset();
	sbuf >> mCRC;
	mCRC = qToBigEndian(mCRC);
	sbuf >> mCounter;
	mCounter = qToBigEndian(mCounter);
}

QByteArray QPRegistration::generate() {
	QByteArray s(15, 0);
	qsrand(QDateTime::currentDateTime().toTime_t());
	
	for (int i = 0; i < 15; i++)
		switch (i) {
			case 4: case 9:
				s[i] = '-';
				break;
			default:
				s[i] = code_asc[qrand() % code_asc.size()];
		}
	return s;
}

/*QByteArray QPRegistration::serverGen() {
	QByteArray s(23, 0);
	qsrand(QDateTime::currentDateTime().toTime_t());
	
	for (int i = 0; i < 23; i++)
		switch (i) {
			case 5: case 11: case 17:
				s[i] = '-';
				break;
			default:
				s[i] = code_asc[qrand() % code_asc.size()];
		}
	return s;
}*/
