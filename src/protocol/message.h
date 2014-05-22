#ifndef _MESSAGES_H
#define _MESSAGES_H

#include <QDataStream>

#define MSG_ALTLOGONREPLY					'rep2'
#define MSG_ASSETNEW							'aAst'
#define MSG_ASSETQUERY						'qAst'
#define MSG_ASSETREGI							'rAst'
#define MSG_ASSETSEND							'sAst'
#define MSG_AUTHENTICATE					'auth'
#define MSG_AUTHRESPONSE					'autr'
#define MSG_BLOWTHRU							'blow'
#define MSG_DISPLAYURL						'durl'
#define MSG_DIYIT									'ryit'
#define MSG_DOORLOCK							'lock'
#define MSG_DOORUNLOCK						'unlo'
#define MSG_DRAW									'draw'
#define MSG_EXTENDEDINFO					'sInf'
#define MSG_FILENOTFND						'fnfe'
#define MSG_FILEQUERY							'qFil'
#define MSG_FILESEND							'sFil'
#define MSG_GMSG									'gmsg'
#define MSG_HTTPSERVER						'HTTP'
#define MSG_INITCONNECTION				'cLog'
#define MSG_KILLUSER							'kill'
#define MSG_LISTOFALLROOMS				'rLst'
#define MSG_LISTOFALLUSERS				'uLst'
#define MSG_LOGOFF								'bye '
#define MSG_LOGON									'regi'
#define MSG_NAVERROR							'sErr'
#define MSG_NOOP									'NOOP'
#define MSG_PICTDEL								'FPSq'
#define MSG_PICTMOVE							'pLoc'
#define MSG_PICTNEW								'nPct'
#define MSG_PICTSETDESC						'sPct'
#define MSG_PING									'ping'
#define MSG_PONG									'pong'
#define MSG_PROPDEL								'dPrp'
#define MSG_PROPMOVE							'mPrp'
#define MSG_PROPNEW								'nPrp'
#define MSG_PROPSETDESC						'sPrp'
#define MSG_RESPORT								'resp'
#define MSG_RMSG									'rmsg'
#define MSG_ROOMDESC							'room'
#define MSG_ROOMDESCEND						'endr'
#define MSG_ROOMGOTO							'navR'
#define MSG_ROOMNEW								'nRom'
#define MSG_ROOMSETDESC						'sRom'
#define MSG_SERVERDOWN						'down'
#define MSG_SERVERINFO						'sinf'
#define MSG_SERVERUP							'init'
#define MSG_SMSG									'smsg'
#define MSG_SPOTDEL								'opSd'
#define MSG_SPOTMOVE							'coLs'
#define MSG_SPOTNEW								'opSn'
#define MSG_SPOTSETDESC						'opSs'
#define MSG_SPOTSTATE							'sSta'
#define MSG_SUPERUSER							'susr'
#define MSG_TALK									'talk'
#define MSG_TIMYID								'timy'
#define MSG_TIYID									'tiyr'
#define MSG_TROPSER								'pser'
#define MSG_USERCOLOR							'usrC'
#define MSG_USERDESC							'usrD'
#define MSG_USERENTER							'wprs'
#define MSG_USEREXIT							'eprs'
#define MSG_USERFACE							'usrF'
#define MSG_USERLIST							'rprs'
#define MSG_USERLOG								'log '
#define MSG_USERMOVE							'uLoc'
#define MSG_USERNAME							'usrN'
#define MSG_USERNEW								'nprs'
#define MSG_USERPROP							'usrP'
#define MSG_USERSTATUS						'uSta'
#define MSG_VERSION								'vers'
#define MSG_WHISPER								'whis'
#define MSG_WMSG									'wmsg'
#define MSG_XTALK									'xtlk'
#define MSG_XWHISPER							'xwis'

enum {
	SE_INTERNAL = 0,
	SE_ROOMUNKNOWN,
	SE_ROOMFULL,
	SE_ROOMCLOSED,
	SE_CANTAUTHOR,
	SE_PALACEFULL
};

class ServerMsg {
public:
	void read(QDataStream &data, qint32 refnum);
	QDataStream& write();
protected:
	quint32 id, length;
	qint32 refnum;
};

#endif // _MESSAGES_H
