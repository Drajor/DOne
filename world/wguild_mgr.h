#ifndef GUILD_MGR_H_
#define GUILD_MGR_H_

#include "../common/types.h"
#include "../common/guild_base.h"

class Client;
class ServerPacket;

class WorldGuildManager : public BaseGuildManager {
public:

	//called by zoneserver when it receives a guild message from zone.
	void ProcessZonePacket(ServerPacket *pack);

	uint8 *MakeGuildMembers(uint32 guild_id, const char *prefix_name, uint32 &length);	//make a guild member list packet, returns ownership of the buffer.

protected:
	virtual void sendGuildRefresh(uint32 pGuildID, bool pName, bool pMOTD, bool pRank, bool pRelation);
	virtual void sendCharRefresh(uint32 pOldGuildID, uint32 pGuildID, uint32 pCharacterID);
	virtual void sendRankUpdate(uint32 pCharacterID) {};
	virtual void sendGuildDelete(uint32 pGuildID);

	//map<uint32, uint32> m_tribute;	//map from guild ID to current tribute ammount
};

extern WorldGuildManager guild_mgr;


#endif /*GUILD_MGR_H_*/

