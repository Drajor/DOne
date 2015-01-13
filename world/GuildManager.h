#pragma once

#include "Constants.h"
#include "Character.h"

// For the #guildsearch command.
struct GuildSearchEntry {
	GuildID mID;
	String mName;
};
typedef std::list<GuildSearchEntry> GuildSearchResults;

class IDataStore;
class IlogFactory;
class ILog;
class ZoneManager;
class Guild;
struct GuildMember;

class GuildManager {
public:
	GuildManager();
	~GuildManager();

	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory, ZoneManager* pZoneManager);

	bool isLeader(Character* pCharacter);
	bool isOfficer(Character* pCharacter);

	// Character Packet Events
	void handleCreate(Character* pCharacter, const String pGuildName);
	void handleDelete(Character* pCharacter);
	void handleRemove(Character* pCharacter, const String& pRemoveCharacterName);
	void handleInviteSent(Character* pCharacter, const String& pInviteCharacterName);
	void handleInviteAccept(Character* pCharacter, const String& pInviterName);
	void handleInviteDecline(Character* pCharacter, const String& pInviterName);
	void handleMessage(Character* pCharacter, const String& pMessage);
	void handleSetMOTD(Character* pCharacter, const String& pMOTD);
	void handleGetMOTD(Character* pCharacter);
	void handleSetURL(Character* pCharacter, const String& pURL);
	void handleSetChannel(Character* pCharacter, const String& pChannel);
	void handleSetPublicNote(Character* pCharacter, const String& pCharacterName, const String& pNote);
	void handleStatusRequest(Character* pCharacter, const String& pCharacterName);
	void handlePromote(Character* pCharacter, const String& pPromoteName);
	void handleDemote(Character* pCharacter, const String& pDemoteName);
	void handleSetBanker(Character* pCharacter, const String& pOtherName, const bool pBanker);
	void handleSetAlt(Character* pCharacter, const String& pAltName, const bool pAlt);
	void handleMakeLeader(Character* pCharacter, const String& pLeaderName);

	// Character Events
	void onConnect(Character* pCharacter, uint32 pGuildID);
	void onEnterZone(Character* pCharacter);
	void onLeaveZone(Character* pCharacter);
	void onCamp(Character* pCharacter);
	void onLinkdead(Character* pCharacter);
	void onLevelChange(Character* pCharacter);

	std::list<String> getGuildNames();

	char* _getGuildNames() { return &mGuildNames[0][0]; } // For sending OP_GuildsList
	GuildID getHighestGuildID();

	GuildSearchResults getAllGuilds();

private:

	bool mInitialised = false;
	ILogFactory* mLogFactory = nullptr;
	ILog* mLog = nullptr;
	IDataStore* mDataStore = nullptr;
	ZoneManager* mZoneManager = nullptr;

	void _sendMessage(Guild* pGuild, const String& pSenderName, const String& pMessage, Character* pExclude = nullptr);

	// Sends the Guild MOTD to all online members.
	void _sendMOTD(const Guild* pGuild);

	// Sends the Guild URL to all online members.
	void _sendURL(Guild* pGuild);
	
	// Sends the Guild channel to all online members.
	void _sendChannel(Guild* pGuild);

	void _sendMemberLevelUpdate(const Guild* pGuild, const GuildMember* pMember);
	void _sendMemberZoneUpdate(const Guild* pGuild, const GuildMember* pMember);
	void _sendMOTDReply(Character* pCharacter);
	void _sendGuildInformation(Character* pCharacter);
	void _sendMembers(Guild* pGuild);
	void _sendMemberRemoved(const Guild* pGuild, const String& pRemoveCharacterName);

	void _changeRank(GuildMember* pMember, const GuildRank pNewRank);

	Guild* _findByGuildName(const String& pGuildName);
	Guild* _findByID(const GuildID pID);
	GuildMember* _findByCharacterName(const String& pCharacterName);
	uint32 getNextGuildID();
	void _save();
	std::list<Guild*> mGuilds;

	void _storeGuildName(GuildID pGuildID, String pGuildName);
	char mGuildNames[Limits::Guild::MAX_GUILDS][Limits::Guild::MAX_NAME_LENGTH];
};