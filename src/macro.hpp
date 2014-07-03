#ifndef _MACROS_H
#define _MACROS_H

#define MSG_ALTLOGONREPLY 0x72657032
#define MSG_ASSETNEW 0x61417374
#define MSG_ASSETQUERY 0x71417374
#define MSG_ASSETREGI 0x72417374
#define MSG_ASSETSEND 0x73417374
#define MSG_AUTHENTICATE 0x61757468
#define MSG_AUTHRESPONSE 0x61757472
#define MSG_BLOWTHRU 0x626c6f77
#define MSG_DISPLAYURL 0x6475726c
#define MSG_DIYIT 0x72796974
#define MSG_DOORLOCK 0x6c6f636b
#define MSG_DOORUNLOCK 0x756e6c6f
#define MSG_DRAW 0x64726177
#define MSG_EXTENDEDINFO 0x73496e66
#define MSG_FILENOTFND 0x666e6665
#define MSG_FILEQUERY 0x7146696c
#define MSG_FILESEND 0x7346696c
#define MSG_GMSG 0x676d7367
#define MSG_HTTPSERVER 0x48545450
#define MSG_INITCONNECTION 0x634c6f67
#define MSG_KILLUSER 0x6b696c6c
#define MSG_LISTOFALLROOMS 0x724c7374
#define MSG_LISTOFALLUSERS 0x754c7374
#define MSG_LOGOFF 0x62796520
#define MSG_LOGON 0x72656769
#define MSG_NAVERROR 0x73457272
#define MSG_NOOP 0x4e4f4f50
#define MSG_PICTDEL 0x46505371
#define MSG_PICTMOVE 0x704c6f63
#define MSG_PICTNEW 0x6e506374
#define MSG_PICTSETDESC 0x73506374
#define MSG_PING 0x70696e67
#define MSG_PONG 0x706f6e67
#define MSG_PROPDEL 0x64507270
#define MSG_PROPMOVE 0x6d507270
#define MSG_PROPNEW 0x6e507270
#define MSG_PROPSETDESC 0x73507270
#define MSG_RESPORT 0x72657370
#define MSG_RMSG 0x726d7367
#define MSG_ROOMDESC 0x726f6f6d
#define MSG_ROOMDESCEND 0x656e6472
#define MSG_ROOMGOTO 0x6e617652
#define MSG_ROOMNEW 0x6e526f6d
#define MSG_ROOMSETDESC 0x73526f6d
#define MSG_SERVERDOWN 0x646f776e
#define MSG_SERVERINFO 0x73696e66
#define MSG_SERVERUP 0x696e6974
#define MSG_SMSG 0x736d7367
#define MSG_SPOTDEL 0x6f705364
#define MSG_SPOTMOVE 0x636f4c73
#define MSG_SPOTNEW 0x6f70536e
#define MSG_SPOTSETDESC 0x6f705373
#define MSG_SPOTSTATE 0x73537461
#define MSG_SUPERUSER 0x73757372
#define MSG_TALK 0x74616c6b
#define MSG_TIMYID 0x74696d79
#define MSG_TIYID 0x74697972
#define MSG_TROPSER 0x70736572
#define MSG_USERCOLOR 0x75737243
#define MSG_USERDESC 0x75737244
#define MSG_USERENTER 0x77707273
#define MSG_USEREXIT 0x65707273
#define MSG_USERFACE 0x75737246
#define MSG_USERLIST 0x72707273
#define MSG_USERLOG 0x6c6f6720
#define MSG_USERMOVE 0x754c6f63
#define MSG_USERNAME 0x7573724e
#define MSG_USERNEW 0x6e707273
#define MSG_USERPROP 0x75737250
#define MSG_USERSTATUS 0x75537461
#define MSG_VERSION 0x76657273
#define MSG_WHISPER 0x77686973
#define MSG_WMSG 0x776d7367
#define MSG_XTALK 0x78746c6b
#define MSG_XWHISPER 0x78776973

#define SI_AVATAR_URL 0x00000001
#define SI_SERVER_VERSION 0x00000002
#define SI_SERVER_TYPE 0x00000004
#define SI_SERVER_FLAGS 0x00000008
#define SI_NUM_USERS 0x00000010
#define SI_SERVER_NAME 0x00000020
#define SI_HTTP_URL 0x00000040

#define SI_EXT_NAME 0x4e414d45
#define SI_EXT_PASS 0x50415353
#define SI_EXT_TYPE 0x54595045

#define SI_ERR_AUTH 0x41555448
#define SI_ERR_UNKN 0x554e4b4e

#define SI_INF_AURL 0x4155524c
#define SI_INF_VERS 0x56455253
#define SI_INF_TYPE 0x54595045
#define SI_INF_FLAG 0x464c4147
#define SI_INF_NUM_USERS 0x4e555352
#define SI_INF_NAME 0x4e414d45
#define SI_INF_HURL 0x4855524c

#define FF_DIRECTPLAY 0x0001
#define FF_CLOSEDSERVER 0x0002
#define FF_GUESTSAREMEMBERS 0x0004
#define FF_UNUSED1 0x0008
#define FF_INSTANTPALACE 0x0010
#define FF_PALACEPRESENTS 0x0020

#define U_SUPERUSER 0x0001
#define U_GOD 0x0002
#define U_KILL 0x0004
#define U_GUEST 0x0008
#define U_BANISHED 0x0010
#define U_PENALIZED 0x0020
#define U_COMMERROR 0x0040
#define U_GAG 0x0080
#define U_PIN 0x0100
#define U_HIDE 0x0200
#define U_REJECTESP 0x0400
#define U_REJECTPRIVATE 0x0800
#define U_PROPGAG 0x1000

#define LI_ULCAPS_ASSETS_PALACE 0x00000001
#define LI_ULCAPS_ASSETS_FTP 0x00000002
#define LI_ULCAPS_ASSETS_HTTP 0x00000004
#define LI_ULCAPS_ASSETS_OTHER 0x00000008
#define LI_ULCAPS_FILES_PALACE 0x00000010
#define LI_ULCAPS_FILES_FTP 0x00000020
#define LI_ULCAPS_FILES_HTTP 0x00000040
#define LI_ULCAPS_FILES_OTHER 0x00000080
#define LI_ULCAPS_EXTEND_PKT 0x00000100

#define LI_DLCAPS_ASSETS_PALACE 0x00000001
#define LI_DLCAPS_ASSETS_FTP 0x00000002
#define LI_DLCAPS_ASSETS_HTTP 0x00000004
#define LI_DLCAPS_ASSETS_OTHER 0x00000008
#define LI_DLCAPS_FILES_PALACE 0x00000010
#define LI_DLCAPS_FILES_FTP 0x00000020
#define LI_DLCAPS_FILES_HTTP 0x00000040
#define LI_DLCAPS_FILES_OTHER 0x00000080
#define LI_DLCAPS_FILES_HTTPSRVR 0x00000100
#define LI_DLCAPS_EXTEND_PKT 0x00000200

#define LI_2DGRAPHCAP_GIF87 0x00000001
#define LI_2DGRAPHCAP_GIF89A 0x00000002
#define LI_2DGRAPHCAP_JPG 0x00000004
#define LI_2DGRAPHCAP_TIFF 0x00000008
#define LI_2DGRAPHCAP_TARGA 0x00000010
#define LI_2DGRAPHCAP_BMP 0x00000020
#define LI_2DGRAPHCAP_PCT 0x00000040

#define PM_ALLOWGUESTS 0x0001
#define PM_ALLOWCYBORGS 0x0002
#define PM_ALLOWPAINTING 0x0004
#define PM_ALLOWCUSTOMPROPS 0x0008
#define PM_ALLOWWIZARDS 0x0010
#define PM_WIZARDSMAYKILL 0x0020
#define PM_WIZARDSMAYAUTHOR 0x0040
#define PM_PLAYERSMAYKILL 0x0080
#define PM_CYBORGSMAYKILL 0x0100
#define PM_DEATHPENALTY 0x0200
#define PM_PURGEINACTIVEPROPS 0x0400
#define PM_KILLFLOODERS 0x0800
#define PM_NOSPOOFING 0x1000
#define PM_MEMBERCREATEDROOMS 0x2000

#define SO_SAVESESSIONKEYS 0x00000001
#define SO_PASSWORDSECURITY 0x00000002
#define SO_CHATLOG 0x00000004
#define SO_NOWHISPER 0x00000008
#define SO_ALLOWDEMOMEMBERS 0x00000010
#define SO_AUTHENTICATE 0x00000020
#define SO_POUNDPROTECT 0x00000040
#define SO_SORTOPTIONS 0x00000080
#define SO_AUTHTRACKLOGOFF 0x00000100
#define SO_JAVASECURE 0x00000200

#define GATE_ID 86 // placeholder

enum {
	SE_INTERNAL = 0,
	SE_ROOMUNKNOWN,
	SE_ROOMFULL,
	SE_ROOMCLOSED,
	SE_CANTAUTHOR,
	SE_PALACEFULL
};

enum {
	DC_PATH = 0,
	DC_SHAPE,
	DC_TEXT,
	DC_DETONATE,
	DC_DELETE,
	DC_ELLIPSE
};

enum {
	PLAT_MACINTOSH = 0,
	PLAT_WINDOWS95,
	PLAT_WINDOWSNT,
	PLAT_UNIX
};

enum {
	LI_AUXFLAGS_UNKNOWNMACH = 0,
	LI_AUXFLAGS_MAC68K,
	LI_AUXFLAGS_MACPPC,
	LI_AUXFLAGS_WIN16,
	LI_AUXFLAGS_WIN32,
	LI_AUXFLAGS_JAVA,
	LI_AUXFLAGS_OSMASK = 0x0000000F,
	LI_AUXFLAGS_AUTHENTICATE = 0x80000000
};

enum {
	LI_2DENGINECAP_PALACE = 0x00000001,
	LI_2DENGINECAP_DOUBLEBYTE
};

enum {
	LI_3DENGINECAP_VRML1 = 0x00000001,
	LI_3DENGINECAP_VRML2
};

enum {
	K_UNKNOWN = 0,
	K_LOGGEDOFF,
	K_COMMERROR,
	K_FLOOD,
	K_KILLEDBYPLAYER,
	K_SERVERDOWN,
	K_UNRESPONSIVE,
	K_KILLEDBYSYSOP,
	K_SERVERFULL,
	K_INVALIDSERIALNUMBER,
	K_DUPLICATEUSER,
	K_DEATHPENALTYACTIVE,
	K_BANISHED,
	K_BANISHKILL,
	K_NOGUESTS,
	K_DEMOEXPIRED,
	K_VERBOSE
};

#endif // _MACROS_H