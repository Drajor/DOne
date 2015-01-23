#pragma once

#include "Types.h"
#include <list>

class Character;
class Guild;

namespace Data {
	struct Guild;
	struct GuildMember;
}

class GuildMember {
public:
	
	GuildMember(Data::GuildMember* pData);
	~GuildMember();
	inline Data::GuildMember* getData() const { return mData; }

	const String& getName() const;
	inline const u16 getZoneID() const { return mZoneID; }
	inline const u16 getInstanceID() const { return mInstanceID; }
	const u8 getRank() const;
	const u8 getLevel() const;
	const u8 getClass() const;
	const bool isBanker() const;
	const bool isTributeEnabled() const;
	const u32 getFlags() const;
	const bool isAlt() const;
	const u32 getLastSeen() const;
	const u32 getTotalTribute() const;
	const u32 getLastTribute() const;
	const String& getPublicNote() const;

	void setName(const String& pCharacterName);
	inline void setZoneID(const u16 pZoneID) { mZoneID = pZoneID; };
	inline void setInstanceID(const u16 pInstanceID) { mInstanceID = pInstanceID; };
	void setRank(const u8 pRank);
	void setLevel(const u8 pLevel);
	void setClass(const u8 pClass);
	void setIsBanker(const bool pIsBanker);
	void setIsTributeEnabled(const bool pIsTributeEnabled);
	void setIsAlt(const bool pIsAlt);
	void setLastSeen(const u32 pLastSeen);

private:

	Data::GuildMember* mData = nullptr;

	u16 mZoneID = 0;
	u16 mInstanceID = 0;
};

class Guild {
public:
	~Guild();
	const bool initialise(Data::Guild* pData);
	inline Data::Guild* getData() const { return mData; }

	void onEnterZone(Character* pCharacter);
	void onLeaveZone(Character* pCharacter);
	void onCamp(Character* pCharacter);
	void onLinkdead(Character* pCharacter);

	const u32 getID() const;
	const String& getName() const;
	const String& getMOTD() const;
	const String& getMOTDSetter() const;
	const String& getURL() const;
	const String& getChannel() const;

	const bool canDelete(Character* pCharacter) const;
	const bool canInvite(Character* pCharacter) const;
	const bool canRemove(Character* pCharacter) const;
	const bool canPromote(Character* pCharacter) const;
	const bool canBePromoted(Character* pCharacter) const;
	const bool canDemote(Character* pCharacter) const;
	const bool canBeDemoted(Character* pCharacter) const;
	
	void onDelete();
	void onJoin(Character* pCharacter, const u8 pRank);
	void onLeave(Character* pCharacter);
	void onConnect(Character* pCharacter);
	void onMemberDisconnect(Character* pCharacter);
	void onRemove(GuildMember * pMember);
	void onPromote(Character* pCharacter);
	void onDemote(Character* pCharacter);
	void onMakeLeader(Character* pCharacter, GuildMember * pMember);

	// Return a copy of the online members.
	inline std::list<Character*> getOnlineMembers() { return mOnlineMembers; }

	// Returns a copy of member data.
	inline std::list<GuildMember*> getMembers() { return mMembers; }

	GuildMember* getMember(const String& pCharacterName) const;

	void sendMessage(const String& pSenderName, const String& pMessage, Character* pExclude = nullptr);

private:

	Data::Guild* mData = nullptr;
	std::list<Character*> mOnlineMembers;
	std::list<GuildMember*> mMembers;

	void sendGuildInformation(Character* pCharacter);
	void sendMemberZoneUpdate(GuildMember* pMember);
	void sendMemberLevelUpdate(GuildMember* pMember);
	void sendMOTD();

	void updateMemberDetails(Character* pCharacter, GuildMember* pMember);

	void removeMember(GuildMember* pMember);
	void removeCharacter(Character* pCharacter);
	Character* getCharacter(const String& pCharacterName);
};