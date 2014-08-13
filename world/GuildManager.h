#pragma once

#include "Constants.h"

// For the #guildsearch command.
struct GuildSearchEntry {
	GuildID mID;
	String mName;
};
typedef std::list<GuildSearchEntry> GuildSearchResults;

class Character;

class Guild {
	friend class GuildManager;
private:
	struct Member {
		uint32 mID; // Character ID
		GuildRank mRank;
	};
	uint32 mID;
	String mName;
	String mMOTD;
	String mMOTDSetter; // The Character name of who set the current MOTD.
	std::list<Character*> mOnlineMembers;
	std::list<Member> mMembers;
};

class GuildManager {
public:
	static GuildManager& getInstance() {
		static GuildManager instance;
		return instance;
	}

	bool initialise();

	// Character Packet Events
	void handleCreate(Character* pCharacter, const String pGuildName);
	void handleDelete(Character* pCharacter);
	void handleRemove(Character* pCharacter, String pRemoveCharacterName);
	void handleInviteSent(Character* pCharacter, String pInviteCharacterName);
	void handleInviteAccept(Character* pCharacter, String pInviterName);
	void handleInviteDecline(Character* pCharacter, String InviterName);
	void handleMessage(Character* pCharacter, const String& pMessage);
	void handleSetMOTD(Character* pCharacter, const String& pMOTD);
	void handleGetMOTD(Character* pCharacter);

	// Character Events
	void onConnect(Character* pCharacter, uint32 pGuildID);
	void onEnterZone(Character* pCharacter);
	void onLeaveZone(Character* pCharacter);
	void onCamp(Character* pCharacter);
	void onLinkdead(Character* pCharacter);

	std::list<String> getGuildNames();

	char* _getGuildNames() { return &mGuildNames[0][0]; } // For sending OP_GuildsList
	GuildID getHighestGuildID();

	GuildSearchResults getAllGuilds();

private:
	void _sendMessage(Guild* pGuild, const String& pSenderName, const String& pMessage, Character* pExclude = nullptr);
	void _sendMOTD(Guild* pGuild);

	Guild* _findGuildByName(const String pGuildName);
	Guild* _findByID(const GuildID pID);
	uint32 getNextGuildID();
	void _save();
	std::list<Guild*> mGuilds;

	void _storeGuildName(GuildID pGuildID, String pGuildName);
	char mGuildNames[MAX_GUILDS][MAX_GUILD_NAME_LENGTH];

	GuildManager() {};
	~GuildManager();
	GuildManager(GuildManager const&);
	void operator=(GuildManager const&);
};