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

	void	AddGuildApproval(const char* guildname, Client* owner);
	void	AddMemberApproval(uint32 refid, Client* name);
	void	ClearGuildsApproval();
	GuildApproval* FindGuildByIDApproval(uint32 refid);
	GuildApproval* FindGuildByOwnerApproval(Client* owner);
	void	ProcessApproval();
	uint32	GetFreeID() { return id + 1; }
	//called by worldserver when it receives a message from world.
	void ProcessWorldPacket(ServerPacket *pack);

	void ListGuilds(Client *c) const;
	void DescribeGuild(Client *c, uint32 guild_id) const;


	//	bool	DonateTribute(uint32 charid, uint32 guild_id, uint32 tribute_amount);

	uint8 *MakeGuildMembers(uint32 guild_id, const char *prefix_name, uint32 &length);	//make a guild member list packet, returns ownership of the buffer.

	void RecordInvite(uint32 char_id, uint32 guild_id, uint8 rank);
	bool VerifyAndClearInvite(uint32 char_id, uint32 guild_id, uint8 rank);
	void SendGuildMemberUpdateToWorld(const char *MemberName, uint32 GuildID, uint16 ZoneID, uint32 LastSeen);
	void RequestOnlineGuildMembers(uint32 FromID, uint32 GuildID);

protected:
	virtual void SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation);
	virtual void SendCharRefresh(uint32 old_guild_id, uint32 guild_id, uint32 charid);
	virtual void SendRankUpdate(uint32 CharID);
	virtual void SendGuildDelete(uint32 guild_id);

	std::map<uint32, std::pair<uint32, uint8> > m_inviteQueue;	//map from char ID to guild,rank

private:
	LinkedList<GuildApproval*> list;
	uint32 id;

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

