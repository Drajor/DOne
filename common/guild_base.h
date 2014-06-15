#ifndef GUILD_BASE_H_
#define GUILD_BASE_H_

#include "guilds.h"
#include <map>
#include <string>
#include <vector>

class Database;

class CharacterGuildInfo {
public:
	//fields from `characer_`
	uint32 mCharacterID;
	std::string mCharacterName;
	uint8 mClass_;
	uint16 mLevel;
	uint32 mTimeLastOn;
	uint32 mZoneID;

	//fields from `guild_members`
	uint32 mGuildID;
	uint8 mRank;
	bool mTributeEnable;
	uint32 mTotalTribute;
	uint32 mLastTribute;		//timestamp
	bool mBanker;
	bool mAlt;
	std::string mPublicNote;
};

//this object holds guild functionality shared between world and zone.
class BaseGuildManager {
public:
	BaseGuildManager();
	virtual ~BaseGuildManager();

	//this must be called before doing anything else with this object
	void setDatabase(Database* pDatabase) { mDatabase = pDatabase; }

	bool loadGuilds();
	bool refreshGuild(uint32 pGuildID);

	//guild edit actions.
	uint32 createGuild(const char* pName, uint32 pLeaderCharacterID);
	bool deleteGuild(uint32 pGuildID);
	bool renameGuild(uint32 pGuildID, const char* pName);
	bool setGuildMOTD(uint32 pGuildID, const char* pMOTD, const char* pSetter);
	bool setGuildURL(uint32 pGuildID, const char* pURL);
	bool setGuildChannel(uint32 pGuildID, const char* pChannel);

	//character edit actions
	bool setGuildLeader(uint32 pGuildID, uint32 pLeaderCharacterID);
	bool setGuild(uint32 pCharacterID, uint32 pGuildID, uint8 pRank);
	bool setGuildRank(uint32 pCharacterID, uint8 pRank);
	bool setBankerFlag(uint32 pCharacterID, bool pIsBanker);
	bool getAltFlag(uint32 pCharacterID);
	bool setAltFlag(uint32 pCharacterID, bool pIsAlt);
	bool getBankerFlag(uint32 pCharacterID);
	bool setTributeFlag(uint32 pCharacterID, bool pEnabled);
	bool setPublicNote(uint32 pCharacterID, const char* pNote);

	//queries
	bool getCharInfo(const char* pCharacterName, CharacterGuildInfo& pInto);
	bool getCharInfo(uint32 pCharacterID, CharacterGuildInfo& pInto);
	bool getEntireGuild(uint32 pGuildID, std::vector<CharacterGuildInfo*>& pMembers);	//caller is responsible for deleting each pointer in the resulting vector.
	bool guildExists(uint32 pGuildID) const;
	bool getGuildMOTD(uint32 pGuildID, char* pMOTDBuffer, char* pSetterBuffer) const;
	bool getGuildURL(uint32 pGuildID, char* pURLBuffer) const;
	bool getGuildChannel(uint32 pGuildID, char* pChannelBuffer) const;
	const char* getRankName(uint32 pGuildID, uint8 pRank) const;
	const char* getGuildName(uint32 pGuildID) const;
	bool getGuildNameByID(uint32 pGuildID, std::string& pInto) const;
	uint32 getGuildIDByName(const char* pGuildName);
	bool isGuildLeader(uint32 pGuildID, uint32 pCharacterID) const;
	uint8 getDisplayedRank(uint32 pGuildID, uint8 pRank, uint32 pCharacterID) const;
	bool checkGMStatus(uint32 pGuildID, uint8 pStatus) const;
	bool checkPermission(uint32 pGuildID, uint8 pRank, GuildAction pAction) const;
	uint32 findGuildByLeader(uint32 pLeader) const;

	uint8* makeGuildList(const char* pHeadName, uint32& pLength) const;	//make a guild list packet, returns ownership of the buffer.

	static const char * const GuildActionNames[_MaxGuildAction];
	uint32 doesAccountContainAGuildLeader(uint32 pAccountID);

protected:
	//the methods which must be defined by base classes.
	virtual void sendGuildRefresh(uint32 pGuildID, bool pName, bool pMOTD, bool pRank, bool pRelation) = 0;
	virtual void sendCharRefresh(uint32 pOldGuildID, uint32 pGuildID, uint32 pCharacterID) = 0;
	virtual void sendRankUpdate(uint32 pCharacterID) = 0;
	virtual void sendGuildDelete(uint32 pGuildID) = 0;

	uint32 DBCreateGuild(const char* pName, uint32 pLeaderCharacterID);
	bool DBDeleteGuild(uint32 pGuildID);
	bool DBRenameGuild(uint32 pGuildID, const char* pName);
	bool DBSetGuildLeader(uint32 pGuildID, uint32 pLeaderCharacterID);
	bool DBSetGuildMOTD(uint32 pGuildID, const char* pMOTD, const char* pSetter);
	bool DBSetGuildURL(uint32 pGuildID, const char* pURL);
	bool DBSetGuildChannel(uint32 pGuildID, const char* pChannel);
	bool DBSetGuild(uint32 pCharacterID, uint32 pGuildID, uint8 pRank);
	bool DBSetGuildRank(uint32 pCharacterID, uint8 pRank);
	bool DBSetBankerFlag(uint32 pCharacterID, bool pIsBanker);
	bool DBSetAltFlag(uint32 pCharacterID, bool pIsAlt);
	bool DBSetTributeFlag(uint32 pCharacterID, bool pEnabled);
	bool DBSetPublicNote(uint32 pCharacterID, const char* pNote);
	bool _RunQuery(char*& pQuery, int pLength, const char* pErrorMessage);

	bool localDeleteGuild(uint32 pGuildID);

	class RankInfo {
	public:
		RankInfo();
		std::string mName;
		bool mPermissions[_MaxGuildAction];
	};
	class GuildInfo {
	public:
		GuildInfo();
		std::string mName;
		std::string mMOTD;
		std::string mMOTDSetter;
		std::string mURL;
		std::string mChannel;

		uint32 mLeaderCharacterID;
		uint8 mMinStatus;
		//tribute is not in here on purpose, since it is only valid in world!
		RankInfo mRanks[GUILD_MAX_RANK+1];
	};

	std::map<uint32, GuildInfo *> mGuilds;	//we own the pointers in this map
	void clearGuilds();	//clears internal structure

	Database *mDatabase;	//we do not own this

	bool _storeGuildDB(uint32 pGuildID);
	GuildInfo *_createGuild(uint32 pGuildID, const char* pGuildName, uint32 pAccountID, uint8 pMinStatus, const char* pGuildMOTD, const char* pMOTDSetter, const char* pChannel, const char* pURL);
	uint32 _getFreeGuildID();
};


#endif /*GUILD_BASE_H_*/


