#ifndef CLIENTENTRY_H_
#define CLIENTENTRY_H_

#include "../common/types.h"
#include "../common/md5.h"
#include "../common/servertalk.h"


#define CLE_Status_Never		-1
#define CLE_Status_Offline		0
#define CLE_Status_Online		1	// Will not overwrite more specific online status
#define CLE_Status_CharSelect	2
#define CLE_Status_Zoning		3
#define CLE_Status_InZone		4

class ZoneServer;
struct ServerClientList_Struct;

class ClientListEntry {
public:
	ClientListEntry(uint32 id, uint32 iLSID, const char* iLoginName, const char* iLoginKey, int16 iWorldAdmin = 0, uint32 ip = 0, uint8 local=0);
	~ClientListEntry();

	bool	CheckStale();
	void	Update(ZoneServer* zoneserver, ServerClientList_Struct* scl, int8 iOnline = CLE_Status_InZone);
	bool	CheckAuth(uint32 iLSID, const char* key);
	void	SetOnline(ZoneServer* iZS, int8 iOnline);
	void	SetOnline(int8 iOnline = CLE_Status_Online);
	void	SetChar(uint32 iCharID, const char* iCharName);
	inline int8		Online()		{ return mOnline; }
	inline const uint32	GetID() const	{ return id; }
	inline const uint32	GetIP() const	{ return mIP; }
	inline void			SetIP(const uint32& iIP) { mIP = iIP; }
	inline void			KeepAlive()		{ mStale = 0; }
	inline uint8			GetStaleCounter() const { return mStale; }
	void	LeavingZone(ZoneServer* iZS = 0, int8 iOnline = CLE_Status_Offline);
	void	Camp(ZoneServer* iZS = 0);

	// Login Server
	inline uint32 getLoginServerAccountID() const { return mLoginServerID; }
	inline const char* getLoginServerAccountName() const { return mLoginServerAccountName; }
	inline const char* getLoginServerKey() const { return mLoginServerKey; }
	inline int16 getWorldAdminStatus() const { return mWorldAdminStatus; } // TODO: Not really used as far as I can tell.
	

	// Account stuff
	inline uint32		AccountID() const		{ return paccountid; }
	inline const char*	AccountName() const		{ return paccountname; }
	inline int16		Admin() const			{ return padmin; }
	inline void			SetAdmin(uint16 iAdmin)	{ padmin = iAdmin; }

	// Character info
	inline ZoneServer*	Server() const		{ return pzoneserver; }
	inline void			ClearServer()		{ pzoneserver = 0; }
	inline uint32		CharID() const		{ return pcharid; }
	inline const char*	name() const		{ return pname; }
	inline uint32		zone() const		{ return pzone; }
	inline uint16		instance() const	{ return pinstance; }
	inline uint8			level() const		{ return plevel; }
	inline uint8			class_() const		{ return pclass_; }
	inline uint16		race() const		{ return prace; }
	inline uint8			Anon()				{ return panon; }
	inline uint8			TellsOff() const	{ return ptellsoff; }
	inline uint32		GuildID() const	{ return pguild_id; }
	inline void			SetGuild(uint32 guild_id) { pguild_id = guild_id; }
	inline bool			LFG() const			{ return pLFG; }
	inline uint8			GetGM() const		{ return gm; }
	inline void			SetGM(uint8 igm)	{ gm = igm; }
	inline void			SetZone(uint32 zone) { pzone = zone; }
	inline bool	IsLocalClient() const { return plocal; }
	inline uint8			GetLFGFromLevel() const { return pLFGFromLevel; }
	inline uint8			GetLFGToLevel() const { return pLFGToLevel; }
	inline bool			GetLFGMatchFilter() const { return pLFGMatchFilter; }
	inline const char*		GetLFGComments() const { return pLFGComments; }
	inline uint8	GetClientVersion() { return pClientVersion; }

private:
	void	ClearVars(bool iAll = false);

	const uint32 id; // Internal (unique) ID, non-persistent. (referred to as WID or WorldID outside)
	uint32 mIP;
	int8 mOnline;
	uint8 mStale;

	// Login Server stuff
	uint32 mLoginServerID; // Provided by Login Server.
	char	mLoginServerAccountName[32];
	char	mLoginServerKey[16];
	int16	mWorldAdminStatus;		// Login server's suggested admin status setting
	bool	plocal;

	// Account stuff
	uint32	paccountid;
	char	paccountname[32];
	MD5		pMD5Pass;
	int16	padmin;

	// Character info
	ZoneServer* pzoneserver;
	uint32	pzone;
	uint16	pinstance;
	uint32	pcharid;
	char	pname[64];
	uint8	plevel;
	uint8	pclass_;
	uint16	prace;
	uint8	panon;
	uint8	ptellsoff;
	uint32	pguild_id;
	bool	pLFG;
	uint8	gm;
	uint8	pClientVersion;
	uint8	pLFGFromLevel;
	uint8	pLFGToLevel;
	bool	pLFGMatchFilter;
	char	pLFGComments[64];
};

#endif /*CLIENTENTRY_H_*/

