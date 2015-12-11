#include "Payload.h"
#include "Account.h"
#include "Data.h"
#include "Zone.h"
#include "Guild.h"
#include "Group.h"
#include "Character.h"
#include "BuffController.h"
#include "Inventory.h"
#include "ExperienceController.h"
#include "Task.h"
#include "TitleManager.h"

#include "../common/crc32.h"
#include "../common/packet_dump_file.h"

#pragma pack(1)

struct Equipment {
	u32 mMaterial = 0;
	u32 mUnknown = 0; // Elite material maybe?
	u32 itemid = 0;
	u32 mColour = 0;
};
struct CharacterSelectionHeader {
	u32 mCharacterCount = 0;
	u32 mAdditionalSlots = 0;
};
struct CharacterSelectionPartA {
	u8 mLevel = 0;
	u8 mHairStyle = 0;
	u8 mGender = 0;
};

struct CharacterSelectionPartB {
	u8 mBeardStyle = 0;
	u8 mHairColour = 0;
	u8 mFaceStyle = 0;
	Equipment mEquipment[9];
	u32 mPrimary = 0;
	u32 mSecondary = 0;
	u8 mUnknown0 = 0; // Was 0xFF
	u32 mDeity = 0;
	u16 mZoneID = 0;
	u16 mInstanceID = 0;
	u8 mReturnHome = 0; // 0 = Unavailable, 1 = Available.
	u8 mUnknown1 = 0; // Was 0xFF
	u32 mRace = 0;
	u8 mEnterTutorial = 0; // 0 = Unavailable, 1 = Available.
	u8 mClass = 0;
	u8 mLeftEyeColour = 0;
	u8 mBeardColour = 0;
	u8 mRightEyeColour = 0;
	u32 mDrakkinHeritage = 0;
	u32 mDrakkinTattoo = 0;
	u32 mDrakkinDetails = 0;
	u8 mUnknown2 = 0;
};

#pragma pack()

EQApplicationPacket* Payload::makeCharacterSelection(SharedPtrList<AccountCharacter> pCharacters) {
	const u32 numAccountCharacters = pCharacters.size();
	u32 size = 0;

	// Add name lengths.
	for (auto i : pCharacters) {
		size += i->mName.length() + 1;
	}

	size += sizeof(CharacterSelectionHeader);
	size += sizeof(CharacterSelectionPartA) * numAccountCharacters;
	size += sizeof(CharacterSelectionPartB) * numAccountCharacters;

	unsigned char * data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	// Header.
	CharacterSelectionHeader header;
	header.mCharacterCount = numAccountCharacters;
	header.mAdditionalSlots = 0; // Total Slots = 8 + additional slots.
	writer.write<CharacterSelectionHeader>(header);

	for (auto i : pCharacters) {
		// Part A.
		CharacterSelectionPartA pA;
		pA.mLevel = i->mLevel;
		pA.mHairStyle = i->mHairStyle;
		pA.mGender = i->mGender;
		writer.write<CharacterSelectionPartA>(pA);

		// Name.
		writer.writeString(i->mName);

		// Part B.
		CharacterSelectionPartB pB;
		pB.mBeardStyle = i->mBeardStyle;
		pB.mHairColour = i->mHairColour;
		pB.mFaceStyle = i->mFaceStyle;

		for (auto j = 0; j < 9; j++) {
			pB.mEquipment[j].mMaterial = i->mEquipment[j].mMaterial;
			pB.mEquipment[j].mColour = i->mEquipment[j].mColour;
		}

		pB.mPrimary = i->mPrimary;
		pB.mSecondary = i->mSecondary;
		pB.mUnknown0 = 0;
		pB.mDeity = i->mDeity;
		pB.mZoneID = i->mZoneID;
		pB.mInstanceID = 0;
		pB.mReturnHome = i->mCanReturnHome ? 1 : 0;
		pB.mUnknown1 = 0;
		pB.mRace = i->mRace;
		pB.mEnterTutorial = i->mCanEnterTutorial ? 1 : 0;
		pB.mClass = i->mClass;
		pB.mLeftEyeColour = i->mEyeColourLeft;
		pB.mBeardColour = i->mBeardColour;
		pB.mRightEyeColour = i->mEyeColourRight;
		pB.mDrakkinHeritage = i->mDrakkinHeritage;
		pB.mDrakkinTattoo = i->mDrakkinTattoo;
		pB.mDrakkinDetails = i->mDrakkinDetails;
		pB.mUnknown2 = 0;
		writer.write<CharacterSelectionPartB>(pB);
	}

	auto packet = new EQApplicationPacket(OP_SendCharInfo, data, size);
	return packet;
}

#pragma pack(1)
namespace Payload {
	struct ZonePointData {
		u32 mID = 0;
		float mY = 0.0f;
		float mX = 0.0f;
		float mZ = 0.0f;
		float mHeading = 0.0f;
		u16 mZoneID = 0;
		u16 mInstanceID = 0;
		u32 mUnknown = 0;
	};
}
#pragma pack()

EQApplicationPacket* Payload::makeZonePoints(const std::list<ZonePoint*>& pZonePoints) {
	const u32 numZonePoints = pZonePoints.size();
	u32 size = 0;

	size += sizeof(u32); // Count.
	size += sizeof(ZonePointData) * numZonePoints;
	
	unsigned char * data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	// Count.
	writer.write<u32>(numZonePoints);

	for (auto i : pZonePoints) {
		ZonePointData p;
		p.mID = i->mID;
		p.mY = i->mPosition.y;
		p.mX = i->mPosition.x;
		p.mZ = i->mPosition.z;
		p.mHeading = i->mHeading;
		p.mZoneID = i->mDestinationZoneID;
		p.mInstanceID = i->mDestinationInstanceID;
		p.mUnknown = 0;

		writer.write<ZonePointData>(p);
	}

	return new EQApplicationPacket(OP_SendZonepoints, data, size);
}

EQApplicationPacket* Payload::makeGuildNameList(const std::list<::Guild*>& pGuilds) {
	const u32 numGuilds = pGuilds.size();

	// Calculate size.
	u32 size = 0;
	size += 64; // Unknown 64 bytes at the start.
	size += 1; // Unknown single byte at the end.
	size += sizeof(u32); // Count.

	// Add name/ID lengths.
	for (auto i : pGuilds) {
		size += i->getName().length() + 1; // Name.
		size += sizeof(u32); // ID.
	}

	auto data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	// Unknown.
	writer._memset(0, 64);
	
	// Count.
	writer.write<u32>(numGuilds);

	// Guilds.
	for (auto i : pGuilds) {
		writer.write<u32>(i->getID());
		writer.writeString(i->getName());
	}

	// Unknown.
	writer.write<u8>(0);

	return new EQApplicationPacket(OP_GuildsList, data, size);
}

EQApplicationPacket* Payload::makeGuildMemberList(::Guild* pGuild) {
	static const auto fixedSize = 40; // 40 bytes per member.

	std::list<GuildMember*> members;
	u32 numMembers = 0;

	if (pGuild) {
		members = pGuild->getMembers();
		numMembers = members.size();
	}
	
	// Calculate size.
	u32 size = 0;
	size += 1; // Empty Character name.
	size += sizeof(u32); // Count.
	size += fixedSize * numMembers;

	// Add name lengths.
	for (auto i : members) {
		size += i->getName().length() + 1;
	}

	// Add public note lengths.
	for (auto i : members) {
		size += i->getPublicNote().length() + 1;
	}

	unsigned char * data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	// Count.
	writer.write<u8>(0); // Empty Character name.
	writer.write<u32>(htonl(numMembers));

	// Members.
	for (auto i : members) {
		writer.writeString(i->getName());
		writer.write<u32>(htonl(i->getLevel()));
		writer.write<u32>(htonl(i->getFlags()));
		writer.write<u32>(htonl(i->getClass()));
		writer.write<u32>(htonl(i->getRank()));
		writer.write<u32>(htonl(i->getLastSeen()));
		writer.write<u32>(htonl(i->isTributeEnabled() ? 1 : 0));
		writer.write<u32>(htonl(i->getTotalTribute()));
		writer.write<u32>(htonl(i->getLastTribute()));
		writer.write<u32>(htonl(1)); // Unknown.
		writer.writeString(i->getPublicNote());
		writer.write<u16>(htons(i->getInstanceID()));
		writer.write<u16>(htons(i->getZoneID()));	
	}

	// NOTE: Sending this is currently wiping out the guild name at the top of the 'Guild Management' window. (only when it sent when the window is open)
	return new EQApplicationPacket(OP_GuildMemberList, data, size);
}

EQApplicationPacket* Payload::makeGroupMemberList(::Group* pGroup) {
	static const auto fixedSize = 31; // 31 bytes per member.
	const auto numMembers = pGroup->numMembers();
	auto& members = pGroup->getMembers();
	auto leader = pGroup->getLeader();

	// Calculate size.
	u32 size = 0;
	size += sizeof(u32); // Unknown.
	size += sizeof(u32); // Count.
	size += fixedSize * numMembers;
	size += leader->getName().size() + 1;

	// Member names.
	for (auto i : members)
		size += i->getName().length() + 1;

	unsigned char * data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	// Header.
	writer.write<u32>(0); // Probably Group ID.
	writer.write<u32>(numMembers);
	writer.writeString(leader->getName()); // Leader name.

	// Members.
	auto c = 0;
	for (auto i : members) {
		writer.write<u32>(c); // Member index.
		writer.writeString(i->getName()); // Member name.
		writer.write<u8>(0); // Unknown.
		writer.write<u8>(0); // Unknown.
		writer.write<u8>(0); // String: Unknown purpose. Pet Name?
		writer.write<u32>(i->getLevel()); // Level. (maybe)
		writer.write<u8>(i->isGroupMainTank() ? 1 : 0); // Main Tank
		writer.write<u8>(i->isGroupMainAssist() ? 1 : 0); // Main Assist
		writer.write<u8>(i->isGroupPuller() ? 1 : 0); // Puller
		writer.write<u32>(0); // Unknown. Possible 'in zone' flag. 0 name = white, 1 = light gray
		writer.write<u8>(0); // Unknown.
		writer.write<u8>(0); // Unknown.
		writer.write<u8>(0); // Unknown.
		writer.write<u8>(0); // Unknown.
		c++;
	}

	// NOTE: This packet fills the group window with names.
	return new EQApplicationPacket(OP_GroupUpdateB, data, size);
}

EQApplicationPacket* Payload::makeCharacterProfile(Character* pCharacter) {
	auto packet = Payload::Zone::CharacterProfile::create();
	auto payload = Payload::Zone::CharacterProfile::convert(packet);
	memset(payload, 0, sizeof(Payload::Zone::CharacterProfile));

	auto inventory = pCharacter->getInventory();
	auto experience = pCharacter->getExperienceController();
	auto group = pCharacter->getGroup();

	//payload->mCheckSum = 0;
	payload->mGender = pCharacter->getGender();
	payload->mRace = pCharacter->getRace();
	payload->mClass = pCharacter->getClass();
	//payload->mUnknown0[40]; // Unknown.
	payload->mLevel0 = pCharacter->getLevel();
	payload->mLevel1 = pCharacter->getLevel();
	//payload->mUnknown1[2]; // Unknown.
	//BindLocation mBindLocations[5];
	payload->mDeity = pCharacter->getDeity();
	payload->mIntoxication = pCharacter->getIntoxication();
	//payload->mSpellBarTimers[10];
	//payload->mUnknown2[14]; // Unknown.
	//payload->abilitySlotRefresh = 0; // Not sure what this is yet.
	payload->mHairColour = pCharacter->getHairColour();
	payload->mBeardColour = pCharacter->getBeardColour();
	payload->mLeftEyeColour = pCharacter->getLeftEyeColour();
	payload->mRightEyeColour = pCharacter->getRightEyeColour();
	payload->mHairStyle = pCharacter->getHairStyle();
	payload->mBeardStyle = pCharacter->getBeardStyle();
	//payload->mUnknown3[4]; // Unknown.
	//Equipment mEquipment[9];
	//payload->mUnknown4[168]; // Unknown.
	//payload->mEquipmentColours[9];
	//AA mAAs[300];
	//payload->mTrainingPoints = 0;
	payload->mMana = pCharacter->getCurrentMana();
	payload->mHealth = pCharacter->getCurrentHP();
	auto bonuses = pCharacter->getBaseBonuses();
	payload->mStrength = bonuses->getStrength();
	payload->mStamina = bonuses->getStamina();
	payload->mCharisma = bonuses->getCharisma();
	payload->mDexterity = bonuses->getDexterity();
	payload->mIntelligence = bonuses->getIntelligence();
	payload->mAgility = bonuses->getAgility();
	payload->mWisdom = bonuses->getWisdom();
	//payload->mUnknown5[28]; // Unknown.
	payload->mUnknown5[0] = 2;
	payload->mUnknown5[1] = 4;
	payload->mUnknown5[2] = 6;
	payload->mUnknown5[3] = 8;
	payload->mUnknown5[4] = 10;
	payload->mUnknown5[5] = 12;
	payload->mUnknown5[6] = 14;
	payload->mFaceStyle = pCharacter->getFaceStyle();
	//payload->mUnknown6[147]; // Unknown.
	//payload->mSpellBook[720];
	//payload->mSpellBar[10];
	payload->mPlatinum = inventory->getPersonalPlatinum();
	payload->mGold = inventory->getPersonalGold();
	payload->mSilver = inventory->getPersonalSilver();
	payload->mCopper = inventory->getPersonalCopper();
	payload->mPlatinumCursor = inventory->getCursorPlatinum();
	payload->mGoldCursor = inventory->getCursorGold();
	payload->mSilverCursor = inventory->getCursorSilver();
	payload->mCopperCursor = inventory->getCursorCopper();
	//payload->mSkills[100];
	//payload->mUnknown8[136];
	//payload->mToxicity = 0;
	payload->mThirst = 0;
	payload->mHunger = 0;
	auto& buffs = pCharacter->getBuffController()->getData();
	for (auto i = 0; i < MaxBuffs; i++) {
		payload->mBuffs[i].mType = buffs[i]->inUse() ? 2 : 0;
		payload->mBuffs[i].mLevel = 0; // TODO:
		payload->mBuffs[i].mInstrumentModifier = 0; // TODO:
		payload->mBuffs[i].mUnknown1 = buffs[i]->inUse() ? 0x3f800000 : 0;
		payload->mBuffs[i].mSpellID = buffs[i]->mSpellID;
		payload->mBuffs[i].mDuration = buffs[i]->mTicksRemaining;
		// Hits remaining?
	}
	//memcpy(payload->mBuffs, &buffs, sizeof(buffs));
	//Disciplines mDisciplines;
	//payload->mRecastTimers[20];
	//payload->unknown11052[160]; // Some type of Timers
	payload->mEndurance = pCharacter->getCurrentEndurance();
	//payload->mUnknown9[20]; // Unknown.
	payload->mSpentAAPoints = experience->getSpentAA();
	payload->mAAPoints = experience->getUnspentAA();
	//payload->mUnknown10[4]; // Unknown.
	//Bandolier mBandoliers[20];
	//PotionBeltItem mPotionBelt[5];
	payload->mAvailableSlots = 0xFFFFFFFF;
	//memset(payload->mUnknown0X, 1, sizeof(payload->mUnknown0X));
	//payload->mColdResist = 150;
	//payload->mFireResist = 250;
	//payload->mPoisonResist = 69;
	//payload->mCorruptionResist = 500;
	//payload->mCombatManaRegen = 50;
	//payload->mUnknown0X[0] = 15;
	//payload->mUnknown0X[4] = 15;
	//payload->mUnknown0X[8] = 15;
	//payload->mUnknown1X = 15;
	//payload->mUnknown2X = 15;
	//payload->mUnknown3X = 15;
	//payload->mU0 = 15;
	//payload->mU1 = 15;
	//payload->mU2 = 15;
	//payload->mU3 = 15;
	//payload->mU5 = 15;
	//payload->mU6 = 15; // This affects AC
	//payload->mU7 = 15;
	//payload->mU8 = 15;
	//payload->mU9 = 15;
	strcpy(payload->mName, pCharacter->getName().c_str());
	strcpy(payload->mLastName, pCharacter->getSurname().c_str());
	//payload->mUnknown13[8]; // Unknown.
	payload->mGuildID = pCharacter->getGuildID();
	payload->mCharacterCreated = 0;
	payload->mLastLogin = 0;
	payload->mAccountCreated = 0;
	payload->mTimePlayed = 0;
	payload->mPVP = pCharacter->isPVP() ? 1 : 0;
	payload->mAnonymous = pCharacter->getAnonymous();
	payload->mGM = pCharacter->isGM() ? 1 : 0;
	payload->mGuildRank = pCharacter->getGuildRank();
	payload->mGuildFlags = 0; // pCharacter->getGuildFlags(); // TODO
	//payload->mUnknown14[4];
	payload->mExperience = experience->getExperience();
	//payload->mUnknown15[8]; // Unknown.
	//payload->mAccountEntitledTime = 0; // Not sure yet.
	//payload->mLanguages;
	payload->mY = pCharacter->getPosition().y;
	payload->mX = pCharacter->getPosition().x;
	payload->mZ = pCharacter->getPosition().z;
	payload->mHeading = pCharacter->getHeading();
	//payload->mUnknown17[4]; // Unknown.
	payload->mPlatinumBank = inventory->getBankPlatinum();
	payload->mGoldBank = inventory->getBankGold();
	payload->mSilverBank = inventory->getBankSilver();
	payload->mCopperBank = inventory->getBankCopper();
	//payload->mPlatinumShared = 0; // TODO!
	//payload->mUnknown18[1036]; // Unknown. Possible bank contents?
	payload->mExpansions = 0xFFFFFFFF;
	//payload->mUnknown19[12]; // Unknown.
	//payload->mAutoSplit = 0;
	//payload->mUnknown20[16]; // Unknown.
	payload->mZoneID = pCharacter->getZone()->getID();
	payload->mInstanceID = pCharacter->getZone()->getInstanceID();

	// Character is in a group.
	if (group) {
		auto leader = group->getLeader();
		auto members = group->getMembers();
		
		auto c = 0;
		for (auto i : members) {
			strcpy(payload->mGroupMemberNames[c], i->getName().c_str());
			c++;
		}

		strcpy(payload->mGroupLeaderName, leader->getName().c_str());
	}

	//char mGroupMemberNames[6][64];
	//char mGroupLeaderName[64];
	//payload->mUnknown21[540]; // Unknown. Possible group/raid members
	payload->mEntityID = 0; // Not sure yet.
	payload->mLeadershipExperienceOn = 0; // Not implemented.
	//payload->mUnknown22[4]; // Unknown.
	payload->mGUKEarned = 0;
	payload->mMIREarned = 0;
	payload->mMMCEarned = 0;
	payload->mRUJEarned = 0;
	payload->mTAKEarned = 0;
	payload->mLDONPoints = 0;
	//payload->mUnknown23[7]; // Unknown. Possible spent LDON points?
	payload->mUnknown24 = 0; // Unknown.
	payload->mUnknown25 = 0; // Unknown.
	//payload->mUnknown26[4]; // Unknown.
	//payload->mUnknown27[6]; // Unknown.
	//payload->mUnknown28[72]; // Unknown.
	payload->mTributeTimeRemaining = 0.0f;
	payload->mCareerTributeFavor = 0;
	payload->mUnknown29 = 0; // Unknown. Possible 'Active Cost'
	payload->mCurrentFavor = 0;
	payload->mUnknown30 = 0; // Unknown.
	payload->mTribiteOn = 0;
	//Tribute tributes[5]; // Not sure yet.
	//payload->mUnknown31[4]; // Unknown.
	payload->mGroupExperience = 0; // Not implemented.
	payload->mRaidExperience = 0; // Not implemented.
	payload->mGroupPoints = 0; // Not implemented.
	payload->mRaidPoints = 0; // Not implemented.
	//GroupLeaderAA mGroupLeaderAA;
	for (auto i = 0; i < 16; i++)
		payload->mGroupLeaderAA.mRanks[i] = 1;
	//RaidLeaderAA mRaidLeaderAA;
	//payload->mUnknown32[128]; // Unknown. Possible current group / raid leader AA. (if grouped)
	payload->mAirRemaining = 0;
	//PVPStats mPVPStats;
	payload->mAAExperience = experience->getAAExperience();
	//payload->mUnknown33[40]; // Unknown.
	payload->mRadiantCrystals = inventory->getRadiantCrystals();
	payload->mTotalRadiantCrystals = inventory->getTotalRadiantCrystals();
	payload->mEbonCrystals = inventory->getEbonCrystals();
	payload->mTotalEbonCrystals = inventory->getTotalEbonCrystals();
	payload->mAutoConsentGroup = pCharacter->getAutoConsentGroup() ? 1 : 0;
	payload->mAutoConsentRaid = pCharacter->getAutoConsentRaid() ? 1 : 0;
	payload->mAutoConsentGuild = pCharacter->getAutoConsentGuild() ? 1 : 0;
	//payload->mUnknown34 = 0; // Unknown.
	payload->mLevel3 = pCharacter->getLevel();
	payload->mShowHelm = 0;
	payload->mRestTimer = 0;
	//payload->mUnknown35[1036]

	CRC32::SetEQChecksum((uchar*)payload, sizeof(Payload::Zone::CharacterProfile) - 4);
	return packet;
}

EQApplicationPacket* Payload::makeTitleList(const std::list<Data::Title*>& pTitles) {
	const u32 numTitles = pTitles.size();
	
	// Calculate size.
	u32 size = 0;
	size += sizeof(u32); // Count
	size += sizeof(u32) * numTitles; // Title ID

	for (auto i : pTitles) {
		size += i->mPrefix.length() + 1;
		size += i->mSuffix.length() + 1;
	}

	unsigned char * data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	writer.write<u32>(numTitles); // Count.

	// Titles.
	for (auto i : pTitles) {
		writer.write<u32>(i->mID); // ID
		writer.writeString(i->mPrefix);
		writer.writeString(i->mSuffix);
	}

	return new EQApplicationPacket(OP_SendTitleList, data, size);
}

unsigned char* Payload::updateBuffIcon(const u32 pActorID, Buff* pBuff, u32& pSize) {
	// Calculate size.
	u32 size = 0;

	size += 12; // Header.
	size += 17; // Buff.

	pSize = size;

	unsigned char* data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	// Header.
	writer.write<u32>(pActorID);
	writer.write<u32>(0); // Unknown.
	writer.write<u8>(0); // 1 = All buffs. 0 = 1 buff.
	writer.write<u16>(1); // Count.

	// Buff.
	writer.write<u32>(pBuff->mSlotIndex); // Slot.
	writer.write<u32>(pBuff->mSpellID); // Spell ID.
	writer.write<u32>(pBuff->mTicksRemaining); // Tics remaining.
	writer.write<u32>(pBuff->mHitsRemaining); // Hits remaining.
	writer.write<u8>(0); // Unknown. String?

	// Footer.
	writer.write<u8>(0); // Unknown.

	return data;
}

unsigned char* Payload::updateBuffIcons(const u32 pActorID, std::array<Buff*, MaxBuffs>& pBuffs, u32& pSize) {
	u16 count = 0;
	// Calculate count.
	for (auto i : pBuffs)
		if (i->inUse()) count++;

	// Calculate size.
	u32 size = 0;

	size += 12; // Header.
	size += 17 * count; // Buff.

	pSize = size;

	unsigned char* data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	// Header.
	writer.write<u32>(pActorID);
	writer.write<u32>(0); // Unknown.
	writer.write<u8>(1); // 1 = All buffs. 0 = 1 buff.
	writer.write<u16>(count); // Count.

	// Buffs.
	u32 slot = 0;
	for (auto i : pBuffs) {
	//for (auto i = 0; i < MaxBuffs; i++) {
		//auto buff = pBuffs[i];
		if (i->inUse() == false) continue; // Skip empty buff slots.

		writer.write<u32>(i->mSlotIndex); // Slot.
		writer.write<u32>(i->mSpellID); // Spell ID.
		writer.write<u32>(i->mTicksRemaining); // Tics remaining.
		writer.write<u32>(i->mHitsRemaining); // Hits remaining.
		writer.write<u8>(0); // Unknown. String?
	}

	// Footer.
	writer.write<u8>(0); // Unknown.

	return data;
}

EQApplicationPacket* Payload::updateBuffIcon(const u32 pActorID, Buff* pBuff) {
	u32 size = 0;
	auto data = updateBuffIcon(pActorID, pBuff, size);
	return new EQApplicationPacket(OP_BuffCreate, data, size);
}

EQApplicationPacket* Payload::updateBuffIcons(const u32 pActorID, std::array<Buff*, MaxBuffs>& pBuffs) {
	u32 size = 0;
	auto data = updateBuffIcons(pActorID, pBuffs, size);
	return new EQApplicationPacket(OP_BuffCreate, data, size);
}

EQApplicationPacket* Payload::updateTargetBuffIcon(const u32 pActorID, Buff* pBuff) {
	u32 size = 0;
	auto data = updateBuffIcon(pActorID, pBuff, size);
	return new EQApplicationPacket(OP_TargetBuffs, data, size);
}

EQApplicationPacket* Payload::updateTargetBuffIcons(const u32 pActorID, std::array<Buff*, MaxBuffs>& pBuffs) {
	u32 size = 0;
	auto data = updateBuffIcons(pActorID, pBuffs, size);
	return new EQApplicationPacket(OP_TargetBuffs, data, size);
}

EQApplicationPacket* Payload::updatePetBuffIcon(const u32 pActorID, Buff* pBuff) {
	u32 size = 0;
	auto data = updateBuffIcon(pActorID, pBuff, size);
	return new EQApplicationPacket(OP_PetBuffWindow, data, size);
}

EQApplicationPacket* Payload::updatePetBuffIcons(const u32 pActorID, std::array<Buff*, MaxBuffs>& pBuffs) {
	u32 size = 0;
	auto data = updateBuffIcons(pActorID, pBuffs, size);
	return new EQApplicationPacket(OP_PetBuffWindow, data, size);

}

EQApplicationPacket* Payload::makeAvailableTasks(AvailableTasks& pTasks) {

	// Calculate size.
	u32 size = 0;
	size += 12; // Header.
	size += 64 * pTasks.size(); // Fixed per task.

	// Strings.
	for (auto i : pTasks) {
		size += i->mTitle.length() + 1;
		size += i->mDescription.length() + 1;
	}

	unsigned char* data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	// Header.
	writer.write<u32>(pTasks.size());
	writer.write<u32>(2); // Unknown.
	writer.write<u32>(50); // Actor ID?

	// Tasks.
	for (auto i : pTasks) {
		writer.write<u32>(i->mID); // Task ID.
		writer.write<u32>(0x3f800000); // Unknown. HC copy.
		writer.write<u32>(1000); // Time limit in seconds.
		writer.write<u32>(0); // Unknown.
		writer.writeString(i->mTitle); // Task name.
		writer.writeString(i->mDescription); // Task description.
		writer.write<u8>(0); // Unknown String.
		writer.write<u32>(1); // Unknown. HC copy.
		writer.write<u32>(0); // Unknown.
		writer.write<u32>(1); // Unknown. HC copy.
		writer.write<u32>(0); // Unknown.
		writer.write<u8>(0); // Unknown String.
		writer.write<u8>(0); // Unknown String.
		writer.write<u32>(1); // Item count? HC copy.
		writer.write<u32>(0xFFFFFFFF); // Unknown. HC copy.
		writer.write<u32>(0xFFFFFFFF); // Unknown. HC copy.
		writer.write<u32>(29); // Start zone?
		writer.write<u8>(0); // Unknown String.
	}

	return new EQApplicationPacket(OP_OpenNewTasksWindow, data, size);
}

EQApplicationPacket* Payload::updateTask(TaskSystem::Task* pTask) {
	// Calculate size.
	u32 size = 0;

	// Fixed.
	size += 47;

	if (pTask->getRewardType() == TaskRewardType::Simple) {

	}

	// Strings.
	size += pTask->getTitle().length() + 1;
	size += pTask->getDescription().length() + 1;
	size += pTask->getRewardText().length() + 1;

	unsigned char* data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	writer.write<u32>(pTask->getIndex()); // Index
	writer.write<u32>(pTask->getID()); // ID
	writer.write<u8>(1); // Unknown. 1 = open 'Quest Journal', 0 = do not open 'Quest Journal'
	writer.write<u32>(pTask->getType()); // Type. 0 = 'T' (Task), 1 = 'S' (Shared), 2 = 'Q' (Quest).
	writer.write<u32>(0); // Unknown. Possibly Max Players for Shared Tasks.
	writer.writeString(pTask->getTitle()); // Title.
	writer.write<u32>(pTask->getDuration()); // Duration.
	writer.write<u32>(0); // Unknown.
	writer.write<u32>(pTask->getStartTime()); // Start time.
	writer.writeString(pTask->getDescription()); // Description.
	//writer.write<u8>(pTask->getRewardType()); // TaskRewardType.
	writer.write<u8>(0); // TaskRewardType.
	writer.write<u32>(3); // Coin reward. Non-zero adds 'X platinum, X gold, X silver, X copper' to the Reward(s) section.
	writer.write<u8>(0); // Unknown.
	writer.write<u8>(0); // Unknown.
	writer.write<u8>(0); // Unknown.
	writer.write<u8>(1); // 1 = adds 'You gain experience!!' to the Reward(s) section.
	writer.write<u8>(0); // Unknown.
	writer.write<u8>(0); // Unknown.
	writer.write<u8>(0); // Unknown.
	writer.write<u8>(0); // Unknown.
	writer.write<u8>(0); // Unknown. Non-zero here appears to change the expected size client side.
	writer.writeString(pTask->getRewardText());
	writer.write<i32>(pTask->getRewardPoints()); // Points reward. Non-zero adds 'X points' to the Reward(s) section.

	return new EQApplicationPacket(OP_TaskDescription, data, size);
}

EQApplicationPacket* Payload::updateTaskObjective(TaskSystem::Objective* pObjective) {
	auto parentTask = pObjective->getParentTask();
	auto parentStage = pObjective->getParentStage();

	// Calculate size.
	u32 size = 0;

	// Fixed.
	size += 56;

	// Strings.
	size += pObjective->getTextA().length() + 1;
	size += pObjective->getTextB().length() + 1;
	size += pObjective->getTextC().length() + 1;

	unsigned char* data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	writer.write<u32>(parentTask->getIndex()); // Task Index.
	writer.write<u32>(parentTask->getType()); // Task Type
	writer.write<u32>(parentTask->getID()); // Task ID.
	writer.write<u32>(pObjective->getIndex()); // Objective Index.
	writer.write<u32>(parentStage->getIndex()); // Stage Index.
	writer.write<u32>(pObjective->getType()); // Objective type.
	writer.write<u32>(pObjective->isOptional() ? 1 : 0); // Optional flag.
	writer.write<u32>(0); // Unknown.
	writer.writeString(pObjective->getTextA()); // Text A.
	writer.writeString(pObjective->getTextB()); // Text B.
	writer.write<i32>(pObjective->getRequired()); // Objective required count.
	writer.write<u32>(0); // Unknown.
	writer.write<u32>(0); // Unknown.
	writer.write<u32>(pObjective->getZoneID()); // Zone ID.
	writer.write<u32>(0); // Unknown.
	writer.writeString(pObjective->getTextC()); // Text override.
	writer.write<i32>(pObjective->getValue());
	// Emu had 3 extra bytes after mValue, but I can not determine whether they are needed are not.
	//writer.write<u8>(0); // Unknown.
	//writer.write<u8>(0); // Unknown.
	//writer.write<u8>(0); // Unknown.

	return new EQApplicationPacket(OP_TaskActivity, data, size);
}

EQApplicationPacket* Payload::updateTaskHistory(CompletedTasks& pTasks) {
	// Calculate size.
	u32 size = 0;

	u32 count = pTasks.size();

	size += 4; // Count.
	size += 8 * count; // Fixed

	// Strings
	for (auto i : pTasks)
		size += i.mTaskData->mTitle.length() + 1;

	unsigned char* data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	// Count.
	writer.write<u32>(count);

	// Tasks.
	for (auto i : pTasks) {
		writer.write<u32>(i.mTaskData->mID); // Task ID.
		writer.writeString(i.mTaskData->mTitle); // Task title.
		writer.write<u32>(i.mTimeCompleted); // Completed time stamp.
	}

	return new EQApplicationPacket(OP_CompletedTasks, data, size);
}

EQApplicationPacket* Payload::updateTaskObjectiveHistory(const u32 pTaskIndex, const Data::Task* pTaskData) {
	if (!pTaskData) return nullptr;

	// Calculate size.
	u32 size = 0;

	u32 objectiveCount = 0;
	for (auto i : pTaskData->mStages) {
		objectiveCount += i->mObjectives.size();
	}

	size += 4; // Task index.
	size += 4; // Objective count.
	size += 24 * objectiveCount; // Fixed.

	// Strings.
	for (auto i : pTaskData->mStages) {
		for (auto j : i->mObjectives) {
			size += j->mTextA.length() + 1;
			size += j->mTextB.length() + 1;
			size += j->mTextC.length() + 1;
		}
	}

	unsigned char* data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	writer.write<u32>(pTaskIndex); // Task index.
	writer.write<u32>(objectiveCount); // Objective count.

	for (auto i : pTaskData->mStages) {
		for (auto j : i->mObjectives) {
			writer.write<u32>(j->mType); // Objective type.
			writer.writeString(j->mTextA);
			writer.writeString(j->mTextB);
			writer.write<u32>(j->mRequired); // Required value.
			writer.write<u32>(0); // Unknown.
			writer.write<u32>(0); // Unknown.
			writer.write<u32>(j->mZoneID); // Zone ID.
			writer.write<u32>(0); // Unknown.
			writer.writeString(j->mTextC);
		}
	}

	return new EQApplicationPacket(OP_TaskHistoryReply, data, size);
}