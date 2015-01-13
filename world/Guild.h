#pragma once

#include "Types.h"
#include <list>

class Character;

struct GuildMember {
	Guild* mGuild = nullptr;
	String mName = "";
	u8 mLevel = 0;
	u8 mClass = 0;
	u8 mRank = 0;
	bool mBanker = false;
	bool mTributeEnabled = false;
	bool mAlt = false;
	u32 mTimeLastOn = 0;
	u32 mTotalTribute = 0;
	u32 mLastTribute = 0;
	String mPublicNote = "";
	u16 mZoneID = 0;
	u16 mInstanceID = 0;
	inline bool isOnline() { return mCharacter != nullptr; }
	Character* mCharacter = nullptr; // Only valid when the guild member is online.
};

class Guild {
	friend class GuildManager;
	u32 mID = 0;
	String mName = "";
	String mMOTD = "";
	String mMOTDSetter = ""; // The Character name of who set the current MOTD.
	String mURL = "";
	String mChannel = "";
	std::list<Character*> mOnlineMembers;
	std::list<GuildMember*> mMembers;

	GuildMember* getMember(const String& pCharacterName) {
		for (auto i : mMembers) {
			if (i->mName == pCharacterName)
				return i;
		}

		return nullptr;
	}
	Character* getOnlineMember(const String& pCharacterName) {
		for (auto i : mOnlineMembers) {
			if (i->getName() == pCharacterName)
				return i;
		}

		return nullptr;
	}
};