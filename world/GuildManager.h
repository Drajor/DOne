#pragma once

#include "Constants.h"

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
	bool makeGuild(Character* pCharacter, const String pGuildName);
	std::list<String> getGuildNames();
private:
	Guild* _findGuildByName(const String pGuildName);
	uint32 getNextGuildID();
	void _save();
	std::list<Guild*> mGuilds;

	GuildManager() {};
	~GuildManager();
	GuildManager(GuildManager const&);
	void operator=(GuildManager const&);
};