#pragma once

#include "Constants.h"
#include "Character.h"

#include "Types.h"
#include <list>

// For the #guildsearch command.
struct GuildSearchEntry {
	u32 mID;
	String mName;
};
typedef std::list<GuildSearchEntry> GuildSearchResults;

class IDataStore;
class IlogFactory;
class ILog;
class ZoneManager;
class Guild;
class GuildMember;

namespace Data {
	struct Guild;
}

class GuildManager {
public:
	~GuildManager();

	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory);

	inline const bool exists(const String& pGuildName) const { return search(pGuildName) != nullptr; }

	// Guild Message.
	void onMessage(Character* pCharacter, const String& pMessage);

	// Character Events
	void onConnect(Character* pCharacter, const u32 pGuildID);
	void onEnterZone(Character* pCharacter);
	void onLeaveZone(Character* pCharacter);
	void onCamp(Character* pCharacter);
	void onLinkdead(Character* pCharacter);
	void onLevelChange(Character* pCharacter);
	void onCharacterDelete(const String& pCharacterName);

	// Character is creating a guild.
	const bool onCreate(Character* pCharacter, const String& pGuildName);

	// Character is deleting their guild.
	const bool onDelete(Character* pCharacter);

	// Character is inviting another Character into their guild.
	const bool onInvite(Character* pInviter, Character* pInvitee);

	// Character has accepted a guild invite.
	const bool onInviteAccept(Character* pCharacter);

	// Character is promoting a guild member.
	const bool onPromote(Character* pPromoter, Character* pPromotee);

	// Character is demoting a guild member.
	const bool onDemote(Character* pCharacter, Character* pDemotee);

	// Character is transferring leadership to a guild member.
	const bool onMakeLeader(Character* pCharacter, const String& pLeaderName);

	// Character is removing a guild member.
	const bool onRemove(Character* pRemover, const String& pRemoveeName);

	// Character is leaving their guild.
	const bool onLeave(Character* pCharacter);

	// Character is setting the guild MOTD.
	const bool onSetMOTD(Character* pCharacter, const String& pMOTD);

	// Character is requesting the guild MOTD.
	const bool onMOTDRequest(Character* pCharacter);

	// Character is setting the guild URL.
	const bool onSetURL(Character* pCharacter, const String& pURL);

	// Character is setting the guild channel.
	const bool onSetChannel(Character* pCharacter, const String& pChannel);

	// Character is setting the public note of a guild member.
	const bool onSetPublicNote(Character* pCharacter, const String& pCharacterName, const String& pPublicNote);
	
	// Character is setting the flags of a guild member.
	const bool onSetFlags(Character* pCharacter, const String& pCharacterName, const u32 pFlags);

	inline const std::list<Guild*>& getGuilds() const { return mGuilds; }

	GuildSearchResults getAllGuilds();

private:

	bool mInitialised = false;
	ILogFactory* mLogFactory = nullptr;
	ILog* mLog = nullptr;
	IDataStore* mDataStore = nullptr;

	std::list<Data::Guild*> mData;
	std::list<Guild*> mGuilds;
	
	// Returns a Guild, searching by name.
	Guild* search(const String& pGuildName) const;

	Guild* _findByID(const u32 pID);

	GuildMember* searchMemberByName(const String& pCharacterName);

	Guild* findGuildByMemberName(const String& pCharacterName);
	
	u32 mNextID = 0;
	const u32 getNextGuildID();
	
	const bool save();
};