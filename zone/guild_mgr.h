#ifndef GUILD_MGR_H_
#define GUILD_MGR_H_

#include "../common/types.h"
#include "../common/guild_base.h"
#include <map>
#include <list>
#include "../zone/petitions.h"

extern PetitionList petition_list;
//extern GuildRanks_Struct guilds[512];
//extern ZoneDatabase database;

#define PBUFFER 50
#define MBUFFER 50

#define GUILD_BANK_MAIN_AREA_SIZE	200
#define GUILD_BANK_DEPOSIT_AREA_SIZE	20
class Client;
class ServerPacket;

struct GuildBankItem {
	uint32 mItemID;
	uint32 mQuantity;
	char mDonator[64];
	uint8 mPermissions;
	char mWhoFor[64];
};

struct GuildBankItems {
	GuildBankItem mMainArea[GUILD_BANK_MAIN_AREA_SIZE];
	GuildBankItem mDepositArea[GUILD_BANK_DEPOSIT_AREA_SIZE];
};

struct GuildBank {
	uint32 mGuildID;
	GuildBankItems mItems;
};

enum {
	GuildBankBulkItems = 0, GuildBankItemUpdate = 1, GuildBankPromote = 3, GuildBankViewItem = 4, GuildBankDeposit = 5,
	GuildBankPermissions = 6, GuildBankWithdraw = 7, GuildBankSplitStacks = 8, GuildBankMergeStacks = 9, GuildBankAcknowledge = 10
};

enum { GuildBankDepositArea = 0, GuildBankMainArea = 1 };

enum { GuildBankBankerOnly = 0, GuildBankSingleMember = 1, GuildBankPublicIfUsable = 2, GuildBankPublic = 3 };

class GuildApproval {
public:
	GuildApproval(const char* pGuildName, Client* pOwner, uint32 pID);
	~GuildApproval();

	bool processApproval();
	bool addMemberApproval(Client* pAddition);
	uint32 getID() { return mRefID; }
	Client*	getOwner() { return mOwner; }
	void guildApproved();
	void approvedMembers(Client* pRequestee);

private:
	Timer* mDeletionTimer;
	char mGuild[16];
	char mFounders[3];
	Client* mOwner;
	Client* mMembers[6];
	uint32 mRefID;
};

class ZoneGuildManager : public BaseGuildManager {
public:
	~ZoneGuildManager(void);

	void addGuildApproval(const char* pGuildName, Client* pOwner);
	void addMemberApproval(uint32 pRefID, Client* pClient);
	GuildApproval* getGuildApproval(uint32 pRefID);
	uint32 getFreeID() { return mID + 1; }
	//called by worldserver when it receives a message from world.
	void processWorldPacket(ServerPacket* pPacket);

	
	void listGuilds(Client* pClient) const;
	void describeGuild(Client* pClient, uint32 pGuildID) const;

	uint8* makeGuildMembers(uint32 pGuildID, const char* pPrefixName, uint32& pLength);

	void recordInvite(uint32 pCharacterID, uint32 pGuildID, uint8 pRank);
	bool verifyAndClearInvite(uint32 pCharacterID, uint32 pGuildID, uint8 pRank);
	void sendGuildMemberUpdateToWorld(const char* pMemberName, uint32 pGuildID, uint16 pZoneID, uint32 pLastSeen);
	void requestOnlineGuildMembers(uint32 pFromID, uint32 pGuildID);

protected:

	virtual void sendGuildRefresh(uint32 pGuildID, bool pName, bool pMOTD, bool pRank, bool pRelation);
	virtual void sendCharRefresh(uint32 pOldGuildID, uint32 pGuildID, uint32 pCharacterID);
	virtual void sendRankUpdate(uint32 pCharacterID);
	virtual void sendGuildDelete(uint32 pGuildID);

	std::map<uint32, std::pair<uint32, uint8> > mInviteQueue;	//map from char ID to guild,rank

private:
	LinkedList<GuildApproval*> mGuildApprovals;
	uint32 mID; // TODO: I do not think this is ever initialised.

};


class GuildBankManager
{

public:
	~GuildBankManager();
	void sendGuildBank(Client* pClient);
	bool addItem(uint32 pGuildID, uint8 pArea, uint32 pItemID, int32 pQtyOrCharges, const char* pDonator, uint8 pPermissions, const char* WhoFor);
	int promote(uint32 pGuildID, int pSlotID);
	void setPermissions(uint32 pGuildID, uint16 pSlotID, uint32 pPermissions, const char* pMemberName);
	ItemInst* getItem(uint32 pGuildID, uint16 pArea, uint16 pSlotID, uint32 pQuantity);
	bool deleteItem(uint32 GuildID, uint16 Area, uint16 SlotID, uint32 Quantity);
	bool hasItem(uint32 pGuildID, uint32 pItemID);
	bool isAreaFull(uint32 pGuildID, uint16 pArea);
	bool mergeStacks(uint32 pGuildID, uint16 pSlotID);
	bool splitStack(uint32 pGuildID, uint16 pSlotID, uint32 pQuantity);
	bool allowedToWithdraw(uint32 pGuildID, uint16 pArea, uint16 pSlotID, const char* pName);

private:
	bool isLoaded(uint32 pGuildID);
	bool load(uint32 pGuildID);
	GuildBank* getGuildBank(uint32 pGuildID);
	void updateItemQuantity(uint32 pGuildID, uint16 pArea, uint16 pSlotID, uint32 pQuantity);

	std::list<GuildBank*> mBanks;

};

extern ZoneGuildManager guild_mgr;
extern GuildBankManager *GuildBanks;


#endif /*GUILD_MGR_H_*/

