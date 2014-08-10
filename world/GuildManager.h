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
	void handleCharacterLogIn(Character* pCharacter, uint32 pGuildID);
	void handleMemberRemove(Character* pCharacter, String pRemoveCharacterName);

	bool makeGuild(Character* pCharacter, const String pGuildName);
	std::list<String> getGuildNames();

	char* _getGuildNames() { return &mGuildNames[0][0]; } // For sending OP_GuildsList
	GuildID getHighestGuildID();

	GuildSearchResults getAllGuilds();
private:
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