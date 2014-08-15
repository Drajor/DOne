#pragma once

#include "Constants.h"

// For the #guildsearch command.
struct GuildSearchEntry {
	GuildID mID;
	String mName;
};
typedef std::list<GuildSearchEntry> GuildSearchResults;

class Character;

struct GuildMember {
	GuildMember() : mID(0), mName(""), mLevel(0), mClass(0), mRank(0), mBanker(false), mTributeEnabled(false), mAlt(false), mTimeLastOn(0), mTotalTribute(0), mLastTribute(0), mPublicNote(""), mZoneID(0), mInstanceID(0) {}
	uint32 mID; // Character ID
	String mName;
	std::uint32_t mLevel; // TODO: Should really be uint16
	ClassID mClass;
	GuildRank mRank;
	bool mBanker;
	bool mTributeEnabled;
	bool mAlt;
	std::uint32_t mTimeLastOn;
	std::uint32_t mTotalTribute;
	std::uint32_t mLastTribute;
	String mPublicNote;
	ZoneID mZoneID;
	InstanceID mInstanceID;
	struct Note {
		String mName;
		String mNote;
	};
	std::list<Note> mPersonalNotes;
};

class Guild {
	Guild() : mID(0), mName(""), mMOTD(""), mMOTDSetter(""), mURL(""), mChannel("") {}
	friend class GuildManager;
	uint32 mID;
	String mName;
	String mMOTD;
	String mMOTDSetter; // The Character name of who set the current MOTD.
	String mURL;
	String mChannel;
	std::list<Character*> mOnlineMembers;
	std::list<GuildMember*> mMembers;

	GuildMember* getMember(const String& pCharacterName) {
		for (auto i : mMembers) {
			if (i->mName == pCharacterName)
				return i;
		}

		return nullptr;
	}
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
	void handleRemove(Character* pCharacter, const String& pRemoveCharacterName);
	void handleInviteSent(Character* pCharacter, const String& pInviteCharacterName);
	void handleInviteAccept(Character* pCharacter, const String& pInviterName);
	void handleInviteDecline(Character* pCharacter, const String& InviterName);
	void handleMessage(Character* pCharacter, const String& pMessage);
	void handleSetMOTD(Character* pCharacter, const String& pMOTD);
	void handleGetMOTD(Character* pCharacter);

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
	void _sendMessage(Guild* pGuild, const String& pSenderName, const String& pMessage, Character* pExclude = nullptr);
	void _sendMOTD(Guild* pGuild);
	void _sendMOTD(Character* pCharacter);

	Guild* _findGuildByName(const String pGuildName);
	Guild* _findByID(const GuildID pID);
	uint32 getNextGuildID();
	void _save();
	std::list<Guild*> mGuilds;

	void _storeGuildName(GuildID pGuildID, String pGuildName);
	char mGuildNames[Limits::Guild::MAX_GUILDS][Limits::Guild::MAX_NAME_LENGTH];

	GuildManager() {};
	~GuildManager();
	GuildManager(GuildManager const&);
	void operator=(GuildManager const&);
};