#pragma once

#include "Constants.h"
#include "Utility.h"
#include "Vector3.h"
#include "../common/EQPacket.h"

namespace Data {
	struct Account;
	struct Title;
}
struct ZonePoint;
class Guild;
class Group;
class Character;

#define SIZE_CHECK(pCondition) if(!(pCondition))  { StringStream ss; ss << "[SIZE_CHECK] ("<< ARG_STR(pCondition) << ") Failed in " << __FUNCTION__; mLog->error(ss.str()); mSizeError = true; return false; }
#define STRING_CHECK(pCString, pMaxSize) if(!Utility::isSafe(pCString, pMaxSize)) { StringStream ss; ss << "[STRING_CHECK] Failed in " << __FUNCTION__; mLog->error(ss.str()); mStringError = true; return false; }

#pragma pack(1)

namespace Payload {

	template <typename T>
	struct Fixed {
		template <typename P>
		inline static T* convert(const P* pPacket) { return reinterpret_cast<T*>(pPacket->pBuffer); }
		inline static T* convert(unsigned char* pData) { return reinterpret_cast<T*>(pData); }
		inline static const bool sizeCheck(const std::size_t pSize) { return pSize == sizeof(T); }
		template <typename P>
		inline static const bool sizeCheck(const P* pPacket) { return pPacket->size == size(); }
		inline static const std::size_t size() { return sizeof(T); }
		inline static EQApplicationPacket* make(EmuOpcode pOpCode) { return new EQApplicationPacket(pOpCode, size()); }
	};

	template <typename T, EmuOpcode OpCode>
	struct FixedT {
		inline static T* convert(const EQApplicationPacket* pPacket) { return reinterpret_cast<T*>(pPacket->pBuffer); }
		inline static const bool sizeCheck(const EQApplicationPacket* pPacket) { return pPacket->size == size(); }
		inline static const std::size_t size() { return sizeof(T); }
		inline static EQApplicationPacket* create() { return new EQApplicationPacket(OpCode, reinterpret_cast<const unsigned char*>(new T()), size()); }
		inline static EQApplicationPacket* create(T pPayload) { return new EQApplicationPacket(OpCode, (unsigned char*)&pPayload, size()); }
	};

	template <typename T>
	struct VariableLengthPayload {
		inline static T* convert(unsigned char* pData) { return reinterpret_cast<T*>(pData); }
	};

	enum ItemPacketType
	{
		ItemPacketViewLink = 0x00,
		ItemPacketTradeView = 0x65, // When trading with Character
		ItemPacketLoot = 0x66,
		ItemPacketTrade = 0x67, // When buying from a shop or looting a corpse.
		ItemPacketCharInventory = 0x69,
		ItemPacketSummonItem = 0x6A,
		ItemPacketTributeItem = 0x6C,
		ItemPacketMerchant = 0x64,
		ItemPacketWorldContainer = 0x6B,
		ItemPacketCharmUpdate = 0x6E
	};

	// More complex and variable length packets.

	EQApplicationPacket* makeCharacterProfile(Character* pCharacter);
	EQApplicationPacket* makeCharacterSelection(Data::Account* pData);
	EQApplicationPacket* makeZonePoints(const std::list<ZonePoint*>& pZonePoints);
	EQApplicationPacket* makeGuildNameList(const std::list<::Guild*>& pGuilds);

	EQApplicationPacket* makeGuildMemberList(::Guild* pGuild);
	EQApplicationPacket* makeGroupMemberList(::Group* pGroup);

	EQApplicationPacket* makeTitleList(const std::list<Data::Title*>& pTitles);

	namespace Zone {

		// C->S
		struct ZoneData : public FixedT<ZoneData, OP_NewZone> {
			struct Fog {
				u8 mRed[4];
				u8 mGreen[4];
				u8 mBlue[4];
				u8 mUnknown = 0;
				float mMinimumClip[4];
				float mMaximumClip[4];
			};

			struct Weather {
				u8 mRainChance[4];
				u8 mRainDuration[4];
				u8 mSnowChance[4];
				u8 mSnowDuration[4];
			};

			ZoneData() {
				memset(mCharacterName, 0, sizeof(mCharacterName));
				memset(mShortName, 0, sizeof(mShortName));
				memset(unknown0096, 0, sizeof(unknown0096));
				memset(mLongName, 0, sizeof(mLongName));
				memset(unknown537, 0xFF, sizeof(unknown537));
				memset(unknown571, 0, sizeof(unknown571));
				memset(unknown620, 0, sizeof(unknown620));
				memset(mShortName2, 0, sizeof(mShortName2));
				memset(unknown804, 0, sizeof(unknown804));
				memset(unknown856, 0, sizeof(unknown856));
				memset(unknown932, 0, sizeof(unknown932));
				memset(&mFog, 0, sizeof(Fog));
				memset(&mWeather, 0, sizeof(Weather));
			}

			char mCharacterName[64];
			char mShortName[32];
			char unknown0096[96];
			char mLongName[278];
			u8 mZoneType = 0xFF; // TODO: Enum?
			Fog mFog;
			float mGravity = 0.4f;
			u8 mTimeType = 0; // TODO: Enum?
			Weather mWeather;
			u8 unknown537[33];
			u8 mSkyType = 0; // TODO: Enum? 0 = blank.
			u8 unknown571[13];
			float mExperienceModifier = 1.0f;
			float mSafeY = 0.0f;
			float mSafeX = 0.0f;
			float mSafeZ = 0.0f;
			float mMinimumZ = 0.0f;
			float mMaximumZ = 0.0f;
			float underworld = 0.0f;
			float mMinimumClip = 0.0f;
			float mMaximumClip = 0.0f;
			u8 unknown620[84];
			char mShortName2[96];
			i32 unknown800 = -1; // Copied.
			char unknown804[40];
			i32 unknown844 = 600; // Copied.
			i32 unknown848 = 0;
			u16 mZoneID = 0;
			u16 mInstanceID = 0;
			char unknown856[20];
			u32 mSuspendBuffs = 0;
			u32 unknown880 = 50; // Copied.
			u32 unknown884 = 10; // Copied.
			u8 unknown888 = 1; // Copied.
			u8 unknown889 = 0; // Copied.
			u8 unknown890 = 1; // Copied.
			u8 unknown891 = 0; // Copied.
			u8 unknown892 = 0; // Copied.
			u8 unknown893 = 0; // Copied.
			u8 mFallDamage = 0;
			u8 unknown895 = 0; // Copied.
			u32 unknown896 = 180; // Copied.
			u32 unknown900 = 180; // Copied.
			u32 unknown904 = 180; // Copied.
			u32 unknown908 = 2; // Copied.
			u32 unknown912 = 2; // Copied.
			float mFogDensity = 0.0f;
			u32 unknown920 = 0;
			u32 unknown924 = 0;
			u32 unknown928 = 0;
			u8 unknown932[12];
		};

		// Note: ZoneEntry is C->S fixed and S->C variable.
		struct ZoneEntry : public Fixed<ZoneEntry> {
			u32 mUnknown = 0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
		};

		// Used: CharacterProfile
		struct PotionBeltItem {
			u32 mItemID = 0;
			u32 mItemIcon = 0;
			char mItemName[64];
		};

		// Used: CharacterProfile
		struct BandolierItem {
			char mName[32];
			u32 mItemID = 0;
			u32 mItemIcon = 0;
			char mItemName[64];
		};

		// Used: CharacterProfile
		struct Bandolier {
			char mName[32];
			PotionBeltItem mItems[4];
		};

		// Used: CharacterProfile
		struct Tribute {
			u32 mID = 0;
			u32 mTier = 0;
		};

		// Used: CharacterProfile
		struct GroupLeaderAA {
			u32 mRanks[16];
		};

		// Used: CharacterProfile
		struct RaidLeaderAA {
			u32 mRanks[16];
		};

		struct AA {
			u32 mID = 0;
			u32 mValue = 0;
			u32 mUnknown = 0;
		};

		// Used: CharacterProfile
		struct PVPEvent {
			char mCharacterName[64];
			u32 mLevel = 0;
			u32 mRace = 0;
			u32 mClass = 0;
			u32 mZoneID = 0; // Possible u16 with instance.
			u32 mTime = 0;
			u32 mPoints = 0;
		};

		// Used: CharacterProfile
		struct PVPStats {
			u32 mTotalKills = 0;
			u32 mTotalDeaths = 0;
			u32 mPoints = 0;
			u32 mCareerPoints = 0;
			u32 mBestKillStreak = 0;
			u32 mWorstDeathStreak = 0;
			u32 mCurrentKillStreak = 0;
			PVPEvent mLastKill;
			PVPEvent mLastDeath;
			u32 mKills24 = 0; // Number of kills in the last 24 hours.
			PVPEvent mRecentKills[50];
		};

		// Used: CharacterProfile
		struct Buff {
			u8 mType = 0; // Note sure yet.
			u8 mLevel = 0;
			u8 mBardModifier = 0;
			u8 mUnknown0 = 0; // Unknown.
			u32 mUnknown1 = 0; // Unknown.
			u32 mSpellID = 0;
			u32 mDuration = 0;
			u32 mUnknown2 = 0; // Unknown.
			u32 mCasterSpawnID = 0;
			u32 mCounters = 0;
			u8 mUnknown3[48]; // Unknown.
		};

		// Used: CharacterProfile
		struct BindLocation {
			u32 mZoneID = 0; // Possible u16 with instance.
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			float heading = 0.0f;
		};

		// Used: CharacterProfile
		struct Disciplines {
			u32 mValues[200];
		};

		// Used: CharacterProfile
		struct Equipment {
			u32 equip0;
			u32 equip1;
			u32 mItemID = 0;
		};

		// Drakkin stuff.
		struct CharacterProfile : public FixedT<CharacterProfile, OP_PlayerProfile> {
			u32 mCheckSum = 0;
			u32 mGender = 0;
			u32 mRace = 0;
			u32 mClass = 0;
			u8 mUnknown0[40]; // Unknown. Tested
			u8 mLevel0 = 0;
			u8 mLevel1 = 0;
			u8 mUnknown1[2]; // Unknown. Tested.
			BindLocation mBindLocations[5];
			u32 mDeity = 0;
			u32 mIntoxication = 0;
			u32 mSpellBarTimers[10];
			u8 mUnknown2[14]; // Unknown. Tested.
			u32 abilitySlotRefresh = 0; // Not sure what this is yet.
			u8 mHairColour = 0;
			u8 mBeardColour = 0;
			u8 mLeftEyeColour = 0;
			u8 mRightEyeColour = 0;
			u8 mHairStyle = 0;
			u8 mBeardStyle = 0;
			u8 mUnknown3[4]; // Unknown. Tested.
			Equipment mEquipment[9];
			u8 mUnknown4[168]; // Unknown. Tested.
			u32 mEquipmentColours[9];
			AA mAAs[300];
			u32 mTrainingPoints = 0;
			u32 mMana = 0;
			u32 mHealth = 0;
			u32 mStrength = 0;
			u32 mStamina = 0;
			u32 mCharisma = 0;
			u32 mDexterity = 0;
			u32 mIntelligence = 0;
			u32 mAgility = 0;
			u32 mWisdom = 0;
			u8 mUnknown5[28]; // Unknown. Tested.
			u8 mFaceStyle = 0;
			u8 mUnknown6[147]; // Unknown. Tested.
			u32 mSpellBook[720];
			u32 mSpellBar[10];
			u8 mUnknown7[28]; // Unknown. Tested.
			u32 mPlatinum = 0;
			u32 mGold = 0;
			u32 mSilver = 0;
			u32 mCopper = 0;
			u32 mPlatinumCursor = 0;
			u32 mGoldCursor = 0;
			u32 mSilverCursor = 0;
			u32 mCopperCursor = 0;
			u32 mSkills[100];
			u8 mUnknown8[136]; // Unknown. Tested.
			u32 mToxicity = 0;
			u32 mThirst = 0;
			u32 mHunger = 0;
			Buff mBuffs[30];
			Disciplines mDisciplines;
			u32 mRecastTimers[20];
			u8 unknown11052[160]; // Unknown. Tested.
			u32 mEndurance = 0;
			u8 mUnknown9[20]; // Unknown. Tested.
			u32 mSpentAAPoints = 0;
			u32 mAAPoints = 0;
			u8 mUnknown10[4]; // Unknown. Something to do with being hidden.
			Bandolier mBandoliers[20];
			PotionBeltItem mPotionBelt[5];
			u32 mProfileType = 0; // Non-zero gives 0 mp and end.
			u32 mTemplateType = 2;
			u32 mAvailableSlots = 0xFFFFFFFF; // Appears to disable inventory slots.
			u8 mUnknown0X[12];
			i32 mColdResist = 0;
			i32 mFireResist = 0;
			u32 mUnknown1X = 0;
			u32 mUnknown2X = 0;
			i32 mPoisonResist = 0;
			u32 mUnknown3X = 0;
			i32 mCorruptionResist = 0;
			i32 mU0 = 0;
			i32 mU1 = 0;
			i32 mU2 = 0;
			i32 mU3 = 0;
			i32 mCombatManaRegen = 0; // Seems to cap at 65.
			i32 mU5 = 0;
			i32 mU6 = 0; // This affects AC
			i32 mU7 = 0;
			i32 mU8 = 0;
			i32 mU9 = 0;
			char mName[64];
			char mLastName[32];
			u8 mUnknown13[8]; // Unknown. Tested.
			u32 mGuildID = 0;
			u32 mCharacterCreated = 0;
			u32 mLastLogin = 0;
			u32 mAccountCreated = 0;
			u32 mTimePlayed = 0;
			u8 mPVP = 0;
			u8 mAnonymous = 0;
			u8 mGM = 0;
			u8 mGuildRank = 0;
			u32 mGuildFlags = 0;
			u8 mUnknown14[4]; // Unknown.
			u32 mExperience = 0;
			u8 mUnknown15[8]; // Unknown.
			u32 mAccountEntitledTime = 0; // Not sure yet.
			u8 mLanguages[32];
			float mY = 0;
			float mX = 0;
			float mZ = 0;
			float mHeading = 0;
			u8 mUnknown17[4]; // Unknown.
			u32 mPlatinumBank = 0;
			u32 mGoldBank = 0;
			u32 mSilverBank = 0;
			u32 mCopperBank = 0;
			u32 mPlatinumShared = 0;
			u8 mUnknown18[1036]; // Unknown. Possible bank contents?
			u32 mExpansions = 0;
			u8 mUnknown19[12]; // Unknown. Tested.
			u32 mAutoSplit = 0;
			u8 mUnknown20[16]; // Unknown.
			u16 mZoneID = 0;
			u16 mInstanceID = 0;
			char mGroupMemberNames[6][64];
			char mGroupLeaderName[64];
			u8 mUnknown21[540]; // Unknown. Possible group/raid members. Tested.
			u32 mEntityID = 0; // Not sure yet.
			u32 mLeadershipExperienceOn = 0;
			u8 mUnknown22[4]; // Unknown.
			i32 mGUKEarned = 0;
			i32 mMIREarned = 0;
			i32 mMMCEarned = 0;
			i32 mRUJEarned = 0;
			i32 mTAKEarned = 0;
			i32 mLDONPoints = 0;
			u32 mUnknown23[7]; // Unknown. Possible spent LDON points?
			u32 mUnknown24 = 0; // Unknown.
			u32 mUnknown25 = 0; // Unknown.
			u8 mUnknown26[4]; // Unknown.
			u32 mUnknown27[6]; // Unknown.
			u8 mUnknown28[72]; // Unknown.
			float mTributeTimeRemaining = 0.0f;
			u32 mCareerTributeFavor = 0;
			u32 mUnknown29 = 0; // Unknown. Possible 'Active Cost'
			u32 mCurrentFavor = 0;
			u32 mUnknown30 = 0; // Unknown.
			u32 mTribiteOn = 0;
			Tribute tributes[5]; // Not sure yet.
			u8 mUnknown31[4]; // Unknown.
			double mGroupExperience = 0.0f;
			double mRaidExperience = 0.0f;
			u32 mGroupPoints = 0;
			u32 mRaidPoints = 0;
			GroupLeaderAA mGroupLeaderAA;
			RaidLeaderAA mRaidLeaderAA;
			u8 mUnknown32[128]; // Unknown. Possible current group / raid leader AA. (if grouped)
			u32 mAirRemaining = 0;
			PVPStats mPVPStats;
			u32 mAAExperience = 0;
			u8 mUnknown33[40]; // Unknown.
			u32 mRadiantCrystals = 0;
			u32 mTotalRadiantCrystals = 0;
			u32 mEbonCrystals = 0;
			u32 mTotalEbonCrystals = 0;
			u8 mAutoConsentGroup = 0;
			u8 mAutoConsentRaid = 0;
			u8 mAutoConsentGuild = 0;
			u8 mUnknown34 = 0; // Unknown.
			u32 mLevel3 = 0;
			u32 mShowHelm = 0;
			u32 mRestTimer = 0;
			u8 mUnknown35[1036]; // Unknown. Tested.
		};

		// C->S
		// TODO: Investigate whether this goes inside CharacterProfile, there is room.
		struct TributeD {
			TributeD() {
				memset(mTributes, 0, sizeof(mTributes));
				memset(mTiers, 0, sizeof(mTiers));
			}
			u32 mActive = 0;
			u32	mTributes[5];
			u32 mTiers[5];
			u32 mTributeMasterID = 0; // SpawnID?
		};
		typedef FixedT<TributeD, OP_TributeUpdate> TributeUpdate;

		// C->S
		struct AddNimbus : public FixedT<AddNimbus, OP_SpellEffect> {
			u32 mNimbusID = 0;
			u32 mSpawnID = 0;
			u32 mSpawnID2 = 0;
			u32 mStartDelay = 1;
			u32 mStartDelay2 = 1;
			u32 Unknown020 = 1;	// Seen 3000
			u8 Unknown024 = 1;	// Seen 1 for SoD
			u8 Unknown025 = 1;	// Seen 1 for Live
			u16 Unknown026 = 1157;	// Seen 1157 and 1177 - varies per char
		};

		// C->S
		struct RemoveNimbus : public FixedT<RemoveNimbus, OP_RemoveNimbusEffect> {
			static EQApplicationPacket* construct(const u32 pSpawnID, const u32 pNimbusID) {
				auto packet = create();
				auto payload = convert(packet);

				payload->mSpawnID = pSpawnID;
				payload->mNimbusID = pNimbusID;
				return packet;
			}

			u32 mSpawnID = 0;
			u32 mNimbusID = 0;
		};

		// C->S
		namespace SetTitleAction { enum : u32 { Title = 0, Suffix = 1, }; }
		struct SetTitle : public Fixed<SetTitle> {
			u32 mAction = 0;
			u32 mID = 0;
		};

		// S->C
		namespace TitleUpdateAction { enum : u32 { Title = 0, Suffix = 1, }; }
		struct TitleUpdate : public FixedT<TitleUpdate, OP_SetTitleReply> {
			static EQApplicationPacket* construct(const u32 pAction, const u32 pSpawnID, const String& pText) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mAction = pAction;
				payload->mSpawnID = pSpawnID;
				strcpy(payload->mText, pText.c_str());

				return packet;
			}
			u32 mAction = 0; // See TitleUpdateAction
			char mText[32];
			u32 mSpawnID = 0;
		};

		// C->S
		struct FaceChange : public Fixed<FaceChange> {
			u8 mHairColour = 0;
			u8 mBeardColour = 0;
			u8 mLeftEyeColour = 0;
			u8 mRightEyeColour = 0;
			u8 mHairStyle = 0;
			u8 mBeardStyle = 0;
			u8 mFaceStyle = 0;
			u8 mUnknown0 = 0;
			u32 mDrakkinHeritage = 0;
			u32 mDrakkinTattoo = 0;
			u32 mDrakkinDetails = 0;
			u32 mUnknown1 = 0;
			String _debug() const {
				StringStream ss;
				ss << "[FaceChange] ";
				PRINT_MEMBER((i32)mHairColour);
				PRINT_MEMBER((i32)mBeardColour);
				PRINT_MEMBER((i32)mLeftEyeColour);
				PRINT_MEMBER((i32)mRightEyeColour);
				PRINT_MEMBER((i32)mHairStyle);
				PRINT_MEMBER((i32)mBeardStyle);
				PRINT_MEMBER((i32)mFaceStyle);
				PRINT_MEMBER((i32)mUnknown0);
				PRINT_MEMBER(mDrakkinHeritage);
				PRINT_MEMBER(mDrakkinTattoo);
				PRINT_MEMBER(mDrakkinDetails);
				PRINT_MEMBER(mUnknown1);
				return ss.str();
			}
		};

		// S->C
		struct WearChange : public FixedT<WearChange, OP_WearChange> {
			u16 mSpawnID = 0;
			u32 mMaterialID = 0;
			u32 mUnknown = 0;
			u32 mEliteMaterialID = 0; // Not sure what this is.
			u32 mColour = 0;
			u8 mSlot = 0;
			static EQApplicationPacket* construct(const u16 pSpawnID, const u32 pMaterialID, const u32 pEliteMaterialID, const u32 pColour, const u8 pSlot) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mMaterialID = pMaterialID;
				payload->mEliteMaterialID = pEliteMaterialID;
				payload->mColour = pColour;
				payload->mSlot = pSlot;

				return packet;
			}
			String _debug() const {
				StringStream ss;
				ss << "[WearChange] ";
				PRINT_MEMBER(mSpawnID);
				PRINT_MEMBER(mMaterialID);
				PRINT_MEMBER(mUnknown);
				PRINT_MEMBER(mEliteMaterialID);
				PRINT_MEMBER(mColour);
				PRINT_MEMBER((i32)mColour);
				return ss.str();
			}
		};
		
		// C->S
		struct AutoAttack : public Fixed<AutoAttack> {
			bool mAttacking = false;
			char mUnknown[3];
		};

		// C<->S
		namespace MemoriseSpellAction { enum : u32 { Scribe = 0, Memorise = 1, Unmemorise = 2, SpellBarRefresh = 3, }; }
		struct MemoriseSpell : public FixedT<MemoriseSpell, OP_MemorizeSpell> {
			static EQApplicationPacket* construct(const u32 pSlot, const u32 pSpellID, const u32 pAction) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSlot = pSlot;
				payload->mSpellID = pSpellID;
				payload->mAction = pAction;

				return packet;
			}
			u32 mSlot = 0;
			u32 mSpellID = 0;
			u32 mAction = 0;
			u32 mUnknown0 = 0;
		};

		// C<->S
		struct DeleteSpell : public FixedT<DeleteSpell, OP_DeleteSpell> {
			static EQApplicationPacket* construct(const u32 pSlot, const u32 pResponse) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSlot = pSlot;
				payload->mResponse = pResponse;

				return packet;
			}
			u32 mSlot = 0;
			u32 mResponse = 0; // 1 = Success, 0 = Failure.
		};

		// C->S
		struct LoadSpellSet : public Fixed<LoadSpellSet> {
			u32 mSpellIDs[Limits::Character::MAX_SPELLS_MEMED];
			u32 mUnknown0 = 0;
		};

		// C->S
		struct SwapSpell : public Fixed<SwapSpell> {
			u32 mFrom = 0;
			u32 mTo = 0;
		};

		// C->S
		struct CastSpell : public Fixed<CastSpell> {
			u32 mSlot = 0;
			u32 mSpellID = 0;
			u32 mInventorySlot = 0; // slot for clicky item, 0xFFFF = normal cast
			u32 mTargetID; // SpawnID?
			u32 mUnknown0[5];
		};

		// S->C
		struct BeginCast : public FixedT<BeginCast, OP_BeginCast> {
			static EQApplicationPacket* construct(const u16 pSpawnID, const u16 pSpellID, const u32 pCastTime) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mSpellID = pSpellID;
				payload->mCastTime = pCastTime;

				return packet;
			}
			u16 mSpawnID = 0; // Caster
			u16 mSpellID = 0;
			u32 mCastTime = 0; // MS
		};

		// S->C
		// Based on: InterruptCast_Struct
		// NOTE: This is not actually fixed size. TODO
		struct InterruptCast : public Fixed<InterruptCast> {
			u32 mSpawnID = 0;
			u32 mMessageID = 0;
			char mMessage[0];
		};

		// S->C
		struct ManaChange : public FixedT<ManaChange, OP_ManaChange> {
			static EQApplicationPacket* construct(const u32 pCurrentMana, const u32 pCurrentEndurance, const u32 pSpellID) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mMana = pCurrentMana;
				payload->mEndurance = pCurrentEndurance;
				payload->mSpellID = pSpellID;

				return packet;
			}
			u32 mMana = 0;
			u32 mEndurance = 0;
			u32 mSpellID = 0;
			u32 mUnknown0 = 0;
			u32 mUnknown1 = 0;
		};

		// S->C
		struct ManaUpdate : public FixedT<ManaUpdate, OP_ManaUpdate> {
			static EQApplicationPacket* construct(const u16 pSpawnID, const u32 pCurrentMana, const u32 pMaximumMana) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mCurrentMana = pCurrentMana;
				payload->mMaximumMana = pMaximumMana;
				payload->mSpawnID = pSpawnID;

				return packet;
			}
			u32 mCurrentMana = 0;
			u32 mMaximumMana = 0;
			u16 mSpawnID = 0;
		};

		// S->C
		struct EnduranceUpdate : FixedT<EnduranceUpdate, OP_EnduranceUpdate> {
			static EQApplicationPacket* construct(const u16 pSpawnID, const u32 pCurrentEndurance, const u32 pMaximumEndurance) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mCurrentEndurance = pCurrentEndurance;
				payload->mMaximumEndurance = pMaximumEndurance;
				payload->mSpawnID = pSpawnID;

				return packet;
			}
			u32 mCurrentEndurance = 0;
			u32 mMaximumEndurance = 0;
			u16 mSpawnID = 0;
		};

		// S->C
		// Based on: Action_Struct
		struct Action : public Fixed<Action> {
			u16 mTargetSpawnID = 0;
			u16 mSourceSpawnID = 0;
			u16 mCasterLevel = 1;
			u16 instrument_mod = 0;
			u32 bard_focus_id = 0;
			u16 mUnknown0 = 0;
			u32 mSequence = 0;
			u32 mUnknown1 = 0;
			u8 mType = 0;		// 231 (0xE7) for spells
			u32 mUnknown2 = 0;
			u16 mSpellID = 0; // u16?? hmm
			u8 mUnknown3 = 0;
			u8 buff_unknown = 0;	// if this is 4, a buff icon is made
		};

		// C->S
		struct CombatAbility : public Fixed<CombatAbility> {
			u32 mTargetID = 0;
			u32 mAttack = 0;
			u32 mSkill = 0;
		};

		// C->S
		struct Taunt : Fixed<Taunt> {
			u32 mSpawnID = 0;
		};

		// C<->S
		struct Consider : FixedT<Consider, OP_Consider> {
			enum Message { Ally = 1, Warmly = 2, Kindly = 3, Amiably = 4, Indifferent = 5, Scowls = 6, Threateningly = 7, Dubiously = 8, Apprehensively = 9  };
			static EQApplicationPacket* construct(const u32 pSpawnID, const u32 pTargetSpawnID, const u32 pMessage) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mTargetSpawnID = pTargetSpawnID;
				payload->mMessage = pMessage;

				return packet;
			}
			String _debug() const {
				StringStream ss;
				ss << "[Consider] ";
				PRINT_MEMBER(mSpawnID);
				PRINT_MEMBER(mTargetSpawnID);
				PRINT_MEMBER(mMessage);
				PRINT_MEMBER(mTargetLevel);
				PRINT_MEMBER((i32)mIsPVP);
				PRINT_MEMBER((i32)mUnknown0);
				PRINT_MEMBER((i32)mUnknown1);
				PRINT_MEMBER((i32)mUnknown2);
				return ss.str();
			}
			u32 mSpawnID = 0;
			u32 mTargetSpawnID = 0;
			u32 mMessage = Message::Indifferent;
			u32 mTargetLevel = 0;
			u8 mIsPVP = 0;
			u8 mUnknown0 = 0;
			u8 mUnknown1 = 0;
			u8 mUnknown2 = 0;
		};

		// C<->S
		struct Surname : public FixedT<Surname, OP_Surname> {
			static EQApplicationPacket* construct(const u32 pApproved, const String& pCharacterName, const String& pSurname) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mApproved = pApproved;
				strcpy(payload->mCharacterName, pCharacterName.c_str());
				strcpy(payload->mSurname, pSurname.c_str());

				return packet;
			}
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			u32 mApproved = 0;
			char mSurname[Limits::Character::MAX_LAST_NAME_LENGTH];
		};

		// S->C
		struct SurnameUpdate : public FixedT<SurnameUpdate, OP_GMLastName> {
			SurnameUpdate() {
				memset(mActorName, 0, sizeof(mActorName));
				memset(mGMName, 0, sizeof(mGMName));
				memset(mSurname, 0, sizeof(mSurname));
				memset(mUnknown, 1, sizeof(mUnknown));
			}
			static EQApplicationPacket* construct(const String& pActorName, const String& pSurname) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mActorName, pActorName.c_str());
				strcpy(payload->mGMName, pActorName.c_str());
				strcpy(payload->mSurname, pSurname.c_str());

				return packet;
			}
			char mActorName[Limits::Character::MAX_NAME_LENGTH];
			char mGMName[Limits::Character::MAX_NAME_LENGTH];
			char mSurname[Limits::Character::MAX_NAME_LENGTH];
			u16 mUnknown[4];
		};

		// C<->S
		struct SpawnAppearance : public FixedT<SpawnAppearance, OP_SpawnAppearance> {
			static EQApplicationPacket* construct(const u16 pSpawnID, const u16 pType, const u32 pParameter) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mType = pType;
				payload->mParameter = pParameter;

				return packet;
			}
			u16 mSpawnID = 0;
			u16 mType = 0;
			u32 mParameter = 0;
		};
		
		// S->C
		struct ActorSize : public FixedT<ActorSize, OP_ChangeSize> {
			static EQApplicationPacket* construct(const u32 pSpawnID, const float pSize) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mSize = pSize;

				return packet;
			}
			u32 mSpawnID = 0;
			float mSize = 1.0f;
			u32 mUnknown0 = 0;
			float mUnknown1 = 1.0f;
		};

		// S->C
		struct ActorAnimation : public FixedT<ActorAnimation, OP_Animation> {
			static EQApplicationPacket* construct(const u16 pSpawnID, const u8 pAnimation, const u8 pSpeed = 10) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mSpeed = pSpeed;
				payload->mAnimation = pAnimation;

				return packet;
			}
			u16 mSpawnID = 0;
			u8 mSpeed = 0;
			u8 mAnimation = 0;
		};

		// S->C
		struct SkillUpdate : public FixedT<SkillUpdate, OP_SkillUpdate> {
			static EQApplicationPacket* construct(const u32 pSkillID, const u32 pValue) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSkillID = pSkillID;
				payload->mValue = pValue;

				return packet;
			}
			u32 mSkillID = 0;
			u32 mValue = 0;
		};

		// S->C
		struct Death : public FixedT<Death, OP_Death> {
			static EQApplicationPacket* construct(const u32 pSpawnID, const u32 pKillerSpawnID, const u32 pDamage, const u32 pSkill) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mKillerSpawnID = pKillerSpawnID;
				payload->mDamage = pDamage;
				payload->mSkillID = pSkill;

				return packet;
			}
			u32 mSpawnID = 0;
			u32 mKillerSpawnID = 0;
			u32 mCorpseID = 0; // ??
			u32 mBindZoneID = 0;
			u32 mSpellID = 0xFFFFFFFF;
			u32 mSkillID = 0;
			u32 mDamage = 0;
			u32 mUnknown0 = 0;
		};

		// S->C
		struct Damage : public FixedT<Damage, OP_Damage> {
			Damage() { memset(__Unknown1, 0, sizeof(__Unknown1)); }
			static EQApplicationPacket* construct(const u16 pTarget, const u16 pSource, const u32 pAmount, const u8 pType, const u32 pSequence, const u16 pSpellID = 0) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mTargetSpawnID = pTarget;
				payload->mSourceSpawnID = pSource;
				payload->mAmount = pAmount;
				payload->mType = pType;
				payload->mSpellID = pSpellID;
				payload->mSequence = pSequence;
				return packet;
			}
			u16 mTargetSpawnID = 0;
			u16 mSourceSpawnID = 0;
			u8 mType = 0; // See Constants, DamageType
			u16 mSpellID = 0;
			i32 mAmount = 0;
			float __Unknown0 = 0.0f;
			u32 mSequence = 0;
			u8 __Unknown1[9];
		};

		// C->S
		struct LootBeginRequest : public Fixed<LootBeginRequest> {
			u32 mSpawnID = 0;
		};
		// C->S
		struct LootEndRequest : public Fixed<LootEndRequest> {
			u32 mSpawnID = 0;
		};

		// S->C
		struct LootResponse : public Fixed<LootResponse> {
			enum Response : u8 { 
				ALREADY = 0,
				LOOT = 1,
				DENY = 2,
				TOO_FAR = 3,
				HOSTILE = 7, // "You cannot loot while a hostile create is aware of your presence"
			};
			u8 mResponse = DENY;
			u8 mUnknown0 = 0x5a;
			u8 mUnknown1 = 0x40;
			u8 mUnknown2 = 0;
			u32 mPlatinum = 0;
			u32 mGold = 0;
			u32 mSilver = 0;
			u32 mCopper = 0;
		};

		struct LootItem : Fixed<LootItem> {
			u32 mCorpseSpawnID = 0;
			u32 mLooterSpawnID = 0;
			u32 mSlotID = 0;
			u32 mAutoLoot = 0;
			u32 mUnknown0 = 0; // Right0, 
			String _debug() const {
				StringStream ss;
				ss << "[LootItem] ";
				PRINT_MEMBER(mCorpseSpawnID);
				PRINT_MEMBER(mLooterSpawnID);
				PRINT_MEMBER(mSlotID);
				PRINT_MEMBER(mAutoLoot);
				PRINT_MEMBER(mUnknown0);
				return ss.str();
			}
			/*
			C->S OP_LootItem
			S->C OP_LootItem (echo first packet).
			*/
		};
		
		// C->S
		struct Target : public Fixed<Target> {
			u32 mSpawnID = 0;
		};

		// S->C
		// THIS DOES NOT WORK.
		struct RejectTarget : public FixedT<RejectTarget, OP_TargetReject> {
			//RejectTarget() { memset(mUnknown, 0, sizeof(mUnknown)); }
			static EQApplicationPacket* construct() {
				auto packet = create();
				auto payload = convert(packet);

				// Copied. I have no idea what they do yet.
				payload->mUnknown[0] = 0x2f;
				payload->mUnknown[1] = 0x01;
				payload->mUnknown[4] = 0x0d;

				return packet;
			}
			u8 mUnknown[12];
		};

		// C->S
		namespace TGBAction { enum : u32 { Off = 0, On = 1, }; }
		struct TGB : public Fixed<TGB> {
			u32 mAction = 0;
		};

		// S->C
		struct RequestZoneChange : public FixedT<RequestZoneChange, OP_RequestClientZoneChange> {
			static EQApplicationPacket* construct(const u16 pZoneID, const u16 pInstanceID, const Vector3& pPosition) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mZoneID = pZoneID;
				payload->mInstanceID = pInstanceID;
				payload->mX = pPosition.x;
				payload->mY = pPosition.y;
				payload->mZ = pPosition.z;

				return packet;
			}
			u16 mZoneID = 0;
			u16 mInstanceID = 0;
			float mY = 0.0f;
			float mX = 0.0f;
			float mZ = 0.0f;
			float mHeading = 0.0f;
			u32 mType = 0; // NOTE: I tried various values for mType and found they did nothing I noticed.
		};

		// C<->S
		struct ZoneChange : public FixedT<ZoneChange, OP_ZoneChange> {
			ZoneChange() { memset(mCharacterName, 0, sizeof(mCharacterName)); }
			static EQApplicationPacket* construct(const String& pCharacterName, const u16 pZoneID, const u16 pInstanceID, const Vector3& pPosition, const i32 pSuccess) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mCharacterName, pCharacterName.c_str());
				payload->mZoneID = pZoneID;
				payload->mInstanceID = pInstanceID;
				payload->mX = pPosition.x;
				payload->mY = pPosition.y;
				payload->mZ = pPosition.z;
				payload->mSuccess = pSuccess;

				return packet;
			}
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			u16 mZoneID = 0;
			u16 mInstanceID = 0;
			float mY = 0.0f;
			float mX = 0.0f;
			float mZ = 0.0f;
			u32 mReason = 0;
			i32 mSuccess = 0; // =0 client->server, =1 server->client, -X=specific error
		};

		// S->C
		struct Weather : public FixedT<Weather, OP_Weather> {
			static EQApplicationPacket* construct(const u32 pA, const u32 pB, const u32 pC) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mUnknown0 = pA;
				payload->mType = pB;
				payload->mIntensity = pC;

				return packet;
			}
			enum Type : u32 { NORMAL = 0, SNOW = 2, RAIN = 3 };
			u32 mUnknown0 = 0;
			u32 mType = NORMAL;
			u32 mIntensity = 0;
		};

		// C->S
		// [UF] MoveItem causes a client crash when using trade slots.
		struct MoveItem : public FixedT<MoveItem, OP_MoveItem> {
			static EQApplicationPacket* construct(const u32 pFromSlot, const u32 pToSlot = 0xFFFFFFFF, const u32 pStacks = 0xFFFFFFFF) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mFromSlot = pFromSlot;
				payload->mToSlot = pToSlot;
				payload->mStacks = pStacks;

				return packet;
			}
			u32 mFromSlot = 0;
			u32 mToSlot = 0;
			u32 mStacks = 0;
		};

		// S->C
		// [UF] DeleteItem allows delete from trade slots.
		struct DeleteItem : public FixedT<DeleteItem, OP_DeleteItem> {
			static EQApplicationPacket* construct(const u32 pFromSlot, const u32 pToSlot = 0xFFFFFFFF, const u32 pStacks = 0xFFFFFFFF) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mFromSlot = pFromSlot;
				payload->mToSlot = pToSlot;
				payload->mStacks = pStacks;

				return packet;
			}
			u32 mFromSlot = 0;
			u32 mToSlot = 0;
			u32 mStacks = 0;
		};

		// S->C
		// Have not managed to get this to do anything except trigger "MOVE ITEM FAILED IN CLIENT APPLICATION" in the client.
		struct DeleteItemCharges : public FixedT<DeleteItemCharges, OP_DeleteCharge> {
			static EQApplicationPacket* construct(const u32 pFromSlot, /*const u32 pToSlot = 0xFFFFFFFF, */const u32 pStacks = 0xFFFFFFFF) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mFromSlot = pFromSlot;
				payload->mToSlot = 0xFFFFFFFF;
				payload->mStacks = pStacks;

				return packet;
			}
			u32 mFromSlot = 0;
			u32 mToSlot = 0;
			u32 mStacks = 0;
		};

		// C->S
		struct Consume : public Fixed<Consume> {
			enum ConsumeAction : u32{ AUTO = 0xFFFFFF, MANUAL = 999 };
			enum ConsumeType : u8 { FOOD = 1, DRINK = 2 };
			u32 mSlot = 0;
			u32 mConsumeAction = AUTO; // ConsumeAction
			u8 c_unknown1[4];
			u8 mType = 0; // ConsumeType
			u8 unknown13[3];
		};

		// S->C
		// Based on: Stamina_Struct
		struct Stamina : public FixedT<Stamina, OP_Stamina> {
			u32 mHunger = 0;
			u32 mThirst = 0;
		};

		// C->S
		struct ItemRightClick : public Fixed<ItemRightClick> {
			i32 mSlot = 0;
			u32 mTargetSpawnID = 0;
		};

		// S->C
		struct ItemRightClickResponse : public FixedT<ItemRightClickResponse, OP_ItemVerifyReply> {
			i32 mSlot = 0;
			u32 mSpellID = 0;
			u32 mTargetSpawnID = 0;
		};

		// C->S
		/*
		2/11/2014 - Live shows current level, max level but does not seem to show current exp
		*/
		struct ItemLink : public Fixed<ItemLink> {
			ItemLink() { 
				memset(mAugments, 0, sizeof(mAugments));
				memset(unknown046, 0, sizeof(unknown046));
			}
			u32 mItemID = 0;
			u32 mAugments[5];
			u32 mHash = 0;
			u32 mUnknown0 = 0; // Always 4 it appears.
			i32 mUnknown1 = 0; // Seen: 1 for evolving, 0 non-evolving.
			i32 mUnknown2 = 0; // Seen: 5 for evolving, 0 non-evolving.
			i32 mCurrentEvolvingLevel = 0;
			u16 mOrnamentationIcon = 0;
			char unknown046[2];
		};

		// C->S
		struct OpenContainer : public Fixed<OpenContainer> {
			u32 mSlot = 0;
		};

		// C<->S
		struct TradeRequest : public FixedT<TradeRequest, OP_TradeRequest> {
			u32 mToSpawnID = 0;
			u32 mFromSpawnID = 0;
		};

		// S->C
		struct TradeRequestAcknowledge : public FixedT<TradeRequestAcknowledge, OP_TradeRequestAck> {
			u32 mToSpawnID = 0;
			u32 mFromSpawnID = 0;

			static EQApplicationPacket* construct(const u32 pToSpawnID, const u32 pFromSpawnID) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mToSpawnID = pToSpawnID;
				payload->mFromSpawnID = pFromSpawnID;

				return packet;
			}
		};

		// C->S
		struct TradeCancel : public FixedT<TradeCancel, OP_CancelTrade> {
			u32 mToSpawnID = 0;
			u32 mFromSpawnID = 0;

			static EQApplicationPacket* construct(const u32 pToSpawnID, const u32 pFromSpawnID) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mToSpawnID = pToSpawnID;
				payload->mFromSpawnID = pFromSpawnID;

				return packet;
			}

			String _debug() const {
				StringStream ss;
				ss << "[TradeCancel] ";
				PRINT_MEMBER(mToSpawnID);
				PRINT_MEMBER(mFromSpawnID);
				return ss.str();
			}
		};

		// C<->S
		struct TradeAccept : public FixedT<TradeAccept, OP_TradeAcceptClick> {
			u32 mFromSpawnID = 0;
			u32 mUnknown = 1;

			static EQApplicationPacket* construct(const u32 pFromSpawnID) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mFromSpawnID = pFromSpawnID;

				return packet;
			}
			String _debug() const {
				StringStream ss;
				ss << "[TradeAccept] ";
				PRINT_MEMBER(mFromSpawnID);
				PRINT_MEMBER(mUnknown);
				return ss.str();
			}
		};

		// C<->S
		struct TradeBusy : public FixedT<TradeBusy, OP_TradeBusy> {
			u32 mToSpawnID = 0;
			u32 mFromSpawnID = 0;
			u32 mReason = 0;

			static EQApplicationPacket* construct(const u32 pToSpawnID, const u32 pFromSpawnID, const u32 pReason) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mToSpawnID = pToSpawnID;
				payload->mFromSpawnID = pFromSpawnID;
				payload->mReason = pReason;

				return packet;
			}
		};

		// C->S
		struct ServerFilter : public Fixed<ServerFilter> {
			Filters mFilters;
		};

		// C->S
		struct MoveCurrency : public Fixed<MoveCurrency> {
			u32 mFromSlot = 0;
			u32 mToSlot = 0;
			u32 mFromType = 0;
			u32 mToType = 0;
			i32 mAmount = 0;
		};

		// S->C
		struct TradeCurrencyUpdate : FixedT<TradeCurrencyUpdate, OP_TradeCoins> {
			static EQApplicationPacket* construct(const u32 pSpawnID, const u32 pCurrencyType, const i32 pAmount) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mTraderSpawnID = pSpawnID;
				payload->mType = pCurrencyType;
				payload->mAmount = pAmount;

				return packet;
			}
			u32 mTraderSpawnID = 0;
			u32 mType = 0;
			i32 mAmount = 0;
		};

		// C<-S
		struct MoneyUpdate : public FixedT<MoneyUpdate, OP_MoneyUpdate> {
			static EQApplicationPacket* construct(const i32 pPlatinum, const i32 pGold, const i32 pSilver, const i32 pCopper) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mPlatinum = pPlatinum;
				payload->mGold = pGold;
				payload->mSilver = pSilver;
				payload->mCopper = pCopper;

				return packet;
			}
			i32 mPlatinum = 0;
			i32 mGold = 0;
			i32 mSilver = 0;
			i32 mCopper = 0;
		};

		// C->S
		namespace CrystalCreateType { enum : u32 { Radiant = 4, Ebon = 5, }; }
		struct CrystalCreate : public Fixed<CrystalCreate>  {
			u32 mType = 0;
			u32 mAmount = 0;
			String _debug() const {
				StringStream ss;
				ss << "[CrystalCreate] ";
				PRINT_MEMBER(mType);
				PRINT_MEMBER(mAmount);
				return ss.str();
			}
		};

		// S->C
		struct CrystalUpdate : public FixedT<CrystalUpdate, OP_CrystalCountUpdate> {
			u32 mRadiantCrystals = 0;
			u32 mEbonCrystals = 0;
			u32 mTotalRadiantCrystals = 0;
			u32 mTotalEbonCrystals = 0;
		};

		// S->C
		struct AppearanceUpdate : public FixedT<AppearanceUpdate, OP_Illusion> {
			AppearanceUpdate() {
				memset(mActorName, 0, sizeof(mActorName));
				memset(mUnknowns0, 0, sizeof(mUnknowns0));
				memset(mUnknowns1, 0, sizeof(mUnknowns1));
				memset(mUnknowns2, 0, sizeof(mUnknowns2));
			}
			// Where is eye colours?!
			u32 mSpawnID = 0; // Tested 4 bytes.
			char mActorName[64];
			u16 mRace = 0;
			u16 mIsIllusion = 0; // 0 = OFF, 1 = ON.
			u8 mGender = 0; // Tested.
			u8 mTexture = 0; // Tested.
			u8 mUnknown1 = 0;
			u8 mUnknown2 = 0;
			u8 mHelmTexture = 0;
			u8 mUnknown3 = 0;
			u8 mUnknown4 = 0;
			u8 mUnknown5 = 0;
			u8 mFaceStyle = 0; // Tested.
			u8 mUnknowns0[3];
			u8 mHairStyle = 0; // Tested.
			u8 mHairColour = 0;
			u8 mBeardStyle = 0;
			u8 mBeardColour = 0;
			float mSize = 1.0f; // Tested.
			u8 mUnknowns1[148];
			u8 mUnknowns2[4];
			u32 mDrakkinHeritage = 0; // Drakkin stuff appears wrong.
			u32 mDrakkinTattoo = 0;
			u32 mDrakkinDetails = 0;
		};

		// S->C
		struct HealthUpdate : public FixedT<HealthUpdate, OP_HPUpdate> {
			static EQApplicationPacket* construct(const u16 pSpawnID, const i32 pCurrentHP, const i32 pMaximumHP) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mCurrentHP = pCurrentHP;
				payload->mMaximumHP = pMaximumHP;
				payload->mSpawnID = pSpawnID;

				return packet;
			}
			i32 mCurrentHP = 0;
			i32 mMaximumHP = 0;
			u16 mSpawnID = 0;
		};

		// S->C
		struct ActorHPUpdate : public FixedT<ActorHPUpdate, OP_MobHealth> {
			static EQApplicationPacket* construct(const u16 pSpawnID, const u8 pHPPercent) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mHPPercent = pHPPercent;

				return packet;
			}
			i16 mSpawnID = 0;
			u8 mHPPercent = 0;
		};

		// C->S
		struct MarkNPC : public FixedT<MarkNPC, OP_MarkNPC> {
			MarkNPC() { memset(mName, 0, sizeof(mName)); }
			static EQApplicationPacket* construct(const u32 pSpawnID, const u32 pNumber, const String& pName) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mNumber = pNumber;
				strcpy(payload->mName, pName.c_str());

				return packet;
			}
			u32 mSpawnID = 0;
			u32 mNumber = 1;
			char mName[64];
		};

		// S->C
		struct PopupWindow : public FixedT<PopupWindow, OP_OnLevelMessage> {
			PopupWindow() {
				memset(mTitle, 0, sizeof(mTitle));
				memset(mText, 0, sizeof(mText));
				memset(mLeftButtonName, 0, sizeof(mLeftButtonName));
				memset(mRightButtonName, 0, sizeof(mRightButtonName));
			}
			static EQApplicationPacket* construct(const String& pTitle, const String& pText) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mTitle, pTitle.c_str());
				strcpy(payload->mText, pText.c_str());

				return packet;

			}
			char mTitle[128];
			char mText[4096];
			char mLeftButtonName[25];
			char mRightButtonName[25];
			u8 mShowButtons = 0;
			u8 __Unknown0 = 0;
			u32 mDuration = 0;
			u32 mLeftButtonResponse = 1;
			u32 mRightButtonResponse = 2;
			u32 __Unknown1 = 0;
		};

		// C->S
		struct PopupResponse : public Fixed<PopupResponse> {
			u32	mUnknown = 0;
			u32 mPopupID = 0;
		};

		// C->S
		struct AugmentItem : public Fixed<AugmentItem> {
			AugmentItem() { memset(mUnknown, 0, sizeof(mUnknown)); }
			i16 mContainerSlot = 0;
			char mUnknown[2];
			i32 mAugmentSlot = 0; // -1 = Insert

			String _debug() const {
				StringStream ss;
				ss << "{AugmentItem} ";
				PRINT_MEMBER(mContainerSlot);
				PRINT_MEMBER((int)mUnknown[0]);
				PRINT_MEMBER((int)mUnknown[1]);
				PRINT_MEMBER(mAugmentSlot);
				return ss.str();
			}
		};

		// C->S
		struct AugmentInformation : public Fixed<AugmentInformation> {
			AugmentInformation() { memset(mUnknown, 0, sizeof(mUnknown)); }
			u32 mID = 0;
			u8 mWindow = 0;
			u8 mUnknown[71];

			String _debug() const {
				StringStream ss;
				ss << "{AugmentInformation} ";
				PRINT_MEMBER(mID);
				PRINT_MEMBER((int)mWindow);
				return ss.str();
			}
		};

		// C->S
		struct BookRequest : Fixed<BookRequest> {
			BookRequest() { memset(mText, 0, sizeof(mText)); }
			u32 mWindow = 0;		// where to display the text (0xFFFFFFFF means new window).
			u32 mSlot = 0;		// The inventory slot the book is in. Not used, but echoed in the response packet.
			u32 mType = 0;		// 0 = Scroll, 1 = Book, 2 = Item Info. Possibly others
			u32 mUnknown0 = 0;
			u16 mUnknown1 = 0;
			char mText[8194];

			String _debug() const {
				StringStream ss;
				ss << "{BookRequest} ";
				PRINT_MEMBER(mWindow);
				PRINT_MEMBER(mSlot);
				PRINT_MEMBER(mType);
				PRINT_MEMBER(mUnknown0);
				PRINT_MEMBER(mUnknown1);
				return ss.str();
			}
		};

		// C->S
		struct Combine : Fixed<Combine> {
			u32 mContainerSlotID = 0;
			String _debug() const {
				StringStream ss;
				ss << "{Combine} ";
				PRINT_MEMBER(mContainerSlotID);
				return ss.str();
			}
		};

		// C<->S
		struct ShopRequest : FixedT<ShopRequest, OP_ShopRequest> {
			static EQApplicationPacket* construct(const u32 pNPCSpawnID, const u32 pCharacterSpawnID, const u32 pAction, const float pRate) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mNPCSpawnID = pNPCSpawnID;
				payload->mCharacterSpawnID = pCharacterSpawnID;
				payload->mAction = pAction;
				payload->mRate = pRate;

				return packet;
			}

			u32 mNPCSpawnID = 0;
			u32 mCharacterSpawnID = 0;
			u32 mAction = 0;
			float mRate = 0.0F;

			String _debug() const {
				StringStream ss;
				ss << "{ShopRequest} ";
				PRINT_MEMBER(mNPCSpawnID);
				PRINT_MEMBER(mCharacterSpawnID);
				PRINT_MEMBER(mAction);
				PRINT_MEMBER(mRate);
				return ss.str();
			}
		};

		// C->S
		struct ShopEnd : Fixed<ShopEnd> {
			u32 mNPCSpawnID = 0;
			u32 mCharacterSpawnID = 0;
		};

		// C<->S
		struct ShopSell : FixedT<ShopSell, OP_ShopPlayerSell>{
			static EQApplicationPacket* construct(const u32 pNPCSpawnID, const u32 pSlotID, const u32 pStacks, const u32 pPrice) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mNPCSpawnID = pNPCSpawnID;
				payload->mSlotID = pSlotID;
				payload->mStacks = pStacks;
				payload->mPrice = pPrice;

				return packet;
			}

			u32 mNPCSpawnID = 0;
			u32 mSlotID = 0;
			u32 mStacks = 0;
			u32 mPrice = 0; // When UF sends this, mPrice is undefined. Ignore it.

			String _debug() const {
				StringStream ss;
				ss << "{ShopSell} ";
				PRINT_MEMBER(mNPCSpawnID);
				PRINT_MEMBER(mSlotID);
				PRINT_MEMBER(mStacks);
				PRINT_MEMBER(mPrice);
				return ss.str();
			}
		};

		// C<->S
		struct ShopBuy : public FixedT<ShopBuy, OP_ShopPlayerBuy> {
			static EQApplicationPacket* construct(const u32 pCharacterSpawnID, const u32 pNPCSpawnID, const u32 pItemInstanceID, const u32 pStacks, const uint64 pPrice, const u32 pResponse = 0) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mNPCSpawnID = pNPCSpawnID;
				payload->mCharacterSpawnID = pCharacterSpawnID;
				payload->mItemInstanceID = pItemInstanceID;
				payload->mResponse = pResponse;
				payload->mStacks = pStacks;
				payload->mPrice = pPrice;

				return packet;
			}

			u32 mNPCSpawnID = 0;
			u32 mCharacterSpawnID = 0;
			u32 mItemInstanceID = 0;
			u32 mResponse = 0; // -1 = silent failure, 0 = success.
			u32 mStacks = 0;
			u32 mUnknown4 = 0;
			uint64 mPrice = 0;

			String _debug() const {
				StringStream ss;
				ss << "{ShopBuy} ";
				PRINT_MEMBER(mNPCSpawnID);
				PRINT_MEMBER(mCharacterSpawnID);
				PRINT_MEMBER(mItemInstanceID);
				PRINT_MEMBER(mResponse);
				PRINT_MEMBER(mStacks);
				PRINT_MEMBER(mUnknown4);
				PRINT_MEMBER(mPrice);
				return ss.str();
			}
		};

		// C->S
		// Not sure how this works yet.
		struct ShopDeleteItem : public FixedT<ShopDeleteItem, OP_ShopDelItem> {
			static EQApplicationPacket* construct(const u32 pCharacterSpawnID, const u32 pNPCSpawnID, const u32 pItemInstanceID, const u32 pUnknown) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mNPCSpawnID = pNPCSpawnID;
				payload->mCharacterSpawnID = pCharacterSpawnID;
				payload->mItemInstanceID = pItemInstanceID;
				payload->mUnknown = pUnknown;

				return packet;
			}

			u32 mNPCSpawnID = 0;
			u32 mCharacterSpawnID = 0;
			u32 mItemInstanceID = 0;
			u32 mUnknown = 0;
		};

		// S->C
		struct UpdateAlternateCurrency : FixedT<UpdateAlternateCurrency, OP_AltCurrency> {
			UpdateAlternateCurrency() {
				memset(mCharacterName, 0, sizeof(mCharacterName));
			}
			static EQApplicationPacket* construct(const String& pCharacterName, const u32 pCurrencyID, const u32 pQuantity) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mCharacterName, pCharacterName.c_str());
				payload->mCurrencyID = pCurrencyID;
				payload->mQuantity = pQuantity;

				return packet;
			}
			u32 mAction = 7;
			char mCharacterName[64];
			u32 mCurrencyID = 0;
			u32 mUnknown0 = 1; // Copied.
			u32 mQuantity = 0;
			u32 mUnknown1 = 0; // Copied.
			u32 mUnknown2 = 0; // Copied.
		};

		// C->S
		struct AlternateCurrencyReclaim : Fixed<AlternateCurrencyReclaim> {
			enum Action : u32 { Create = 0, Reclaim = 1 };
			u32 mCurrencyID = 0;
			u32 mUnknown0 = 0;
			u32 mStacks = 0;
			u32 mAction = 0; //1 = this is reclaim

			String _debug() const {
				StringStream ss;
				ss << "{AlternateCurrencyReclaim} ";
				PRINT_MEMBER(mCurrencyID);
				PRINT_MEMBER(mUnknown0);
				PRINT_MEMBER(mStacks);
				PRINT_MEMBER(mAction);
				return ss.str();
			}
		};

		// C->S
		struct RandomRequest : Fixed<RandomRequest> {
			u32 mLow = 0;
			u32 mHigh = 0;
		};

		// S->C
		struct RandomReply : FixedT<RandomReply, OP_RandomReply> {
			RandomReply() { memset(mCharacterName, 0, sizeof(mCharacterName)); }
			static EQApplicationPacket* construct(const String& pCharacterName, const u32 pLow, const u32 pHigh, const u32 pResult) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mCharacterName, pCharacterName.c_str());
				payload->mLow = pLow;
				payload->mHigh= pHigh;
				payload->mResult = pResult;

				return packet;
			}
			u32 mLow = 0;
			u32 mHigh = 0;
			u32 mResult = 0;
			char mCharacterName[64];
		};

		//struct Object : FixedT<Object, OP_GroundSpawn> {
		//	Object() {
		//		memset(mAsset, 0, sizeof(mAsset));
		//	}
		//	u32	linked_list_addr[2];
		//	u32	unknown008 = 0;
		//	u32	drop_id = 0;
		//	u16	mZoneID = 0;
		//	u16 mInstanceID = 0;
		//	u32	unknown020 = 0;
		//	u32	unknown024 = 0;
		//	float mHeading = 0.0f;
		//	u8	unknown032[8];
		//	float mSize = 1.0f;
		//	float mZ = 0.0f;
		//	float mX = 0.0f;
		//	float mY = 0.0f;
		//	char mAsset[32];
		//	u32	unknown088;
		//	u32 mType = 0;
		//	u8	unknown096[4];
		//	u32 mSpawnID = 0;
		//};

		// S->C
		struct ExperienceUpdate : public FixedT<ExperienceUpdate, OP_ExpUpdate> {
			static EQApplicationPacket* construct(const u32 pExperience, const u32 pAAExperience) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mExperience = pExperience;
				payload->mAAExperience = pAAExperience;

				return packet;
			}
			u32 mExperience = 0;
			u32 mAAExperience = 0;
		};

		// S->C
		struct AAExperienceUpdate : public FixedT<AAExperienceUpdate, OP_AAExpUpdate> {
			static EQApplicationPacket* construct(const u32 pAAExperience, const u32 pUnspentAA, const u32 pExperienceToAA) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mExperienceRatio = pAAExperience;
				payload->mUnspentAA = pUnspentAA;
				payload->mExperienceToAA = pExperienceToAA;

				return packet;
			}
			u32 mExperienceRatio = 0;
			u32 mUnspentAA = 0;
			u32 mExperienceToAA = 0;
		};

		// S->C
		struct LeadershipExperienceUpdate : public FixedT<LeadershipExperienceUpdate, OP_LeadershipExpUpdate> {
			static EQApplicationPacket* construct(const double pGroupExperience, const u32 pGroupPoints, const double pRaidExperience, const u32 pRaidPoints) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mGroupExperience = pGroupExperience;
				payload->mGroupPoints = pGroupPoints;
				payload->mRaidExperience = pRaidExperience;
				payload->mRaidPoints = pRaidPoints;

				return packet;
			}
			double mGroupExperience = 0.0f; // 0 - 1000
			u32 mGroupPoints = 0;
			u32 mUnknown = 0;
			double mRaidExperience = 0.0f; // 0 - 2000
			u32 mRaidPoints = 0;
		};

		// S->C
		struct LevelUpdate : public FixedT<LevelUpdate, OP_LevelUpdate> {
			static EQApplicationPacket* construct(const u32 pPreviousLevel, const u32 pCurrentLevel, const u32 pExperience) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mLevel = pCurrentLevel;
				payload->mPreviousLevel = pPreviousLevel;
				payload->mExperience = pExperience;

				return packet;
			}
			u32 mLevel = 0;
			u32 mPreviousLevel = 0;
			u32 mExperience = 0;
		};

		// S->C
		struct LevelAppearance : public FixedT<LevelAppearance, OP_LevelAppearance> {
			u32 mSpawnID = 0;
			u32 mParameter1 = 0;
			u32 mValue1A = 0;
			u32 mValue1B = 0;
			u32 mParameter2 = 0;
			u32 mValue2A = 0;
			u32 mValue2B = 0;
			u32 mParameter3 = 0;
			u32 mValue3A = 0;
			u32 mValue3B = 0;
			u32 mParameter4 = 0;
			u32 mValue4A = 0;
			u32 mValue4B = 0;
			u32 mParameter5 = 0;
			u32 mValue5A = 0;
			u32 mValue5B = 0;
		};

		// C->S
		namespace EmoteLimits { static const auto MAX_MESSAGE = 1024; }
		struct Emote : public FixedT <Emote, OP_Emote> {
			static EQApplicationPacket* construct(const String& pMessage) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mMessage, pMessage.c_str());

				return packet;
			}
			u32 mUnknown = 0;
			char mMessage[EmoteLimits::MAX_MESSAGE];
		};
		
		// S->C
		struct SimpleMessage : FixedT<SimpleMessage, OP_SimpleMessage> {
			static EQApplicationPacket* construct(const u32 pType, const u32 pStringID) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mStringID = pStringID;
				payload->mType = pType;

				return packet;
			}
			u32 mStringID = 0;
			u32 mType = 0;
			u32 mUnknown = 0; // Tried SpawnID and numbers as a parameter for the String ID but nothing worked.
		};

		namespace DoorLimits { static const auto MAX_TEXT = 32; }
		struct Door : public FixedT<Door, OP_SpawnDoor> {
			Door() {
				memset(mUnknowns0, 0, sizeof(mUnknowns0));
			}
			char mAsset[DoorLimits::MAX_TEXT];
			float mY = 0.0f;
			float mX = 0.0f;
			float mZ = 0.0f;
			float mHeading = 0.0f;
			u32 mInline = 0;
			u32 mSize = 100;
			u8 mUnknowns0[4];
			u8 mID = 0;
			u8 mOpenType = 0;
			u8 mState = 0;
			u8 mInvertState = 0; // if this is 1, the door is normally open
			u32 door_param; // normally ff ff ff ff (-1)
			u32 unknown0068; // 00 00 00 00
			u32 unknown0072; // 00 00 00 00
			u8 unknown0076; // seen 1 or 0
			u8 unknown0077; // seen 1 (always?)
			u8 unknown0078; // seen 0 (always?)
			u8 unknown0079; // seen 1 (always?)
			u8 unknown0080; // seen 0 (always?)
			u8 unknown0081; // seen 1 or 0 or rarely 2C or 90 or ED or 2D or A1
			u8 unknown0082; // seen 0 or rarely FF or FE or 10 or 5A or 82
			u8 unknown0083; // seen 0 or rarely 02 or 7C
			u8 unknown0084[8]; // mostly 0s, the last 3 bytes are something tho
		};

		// C->S
		struct RespawnWindowSelect : Fixed<RespawnWindowSelect> {
			u32 mSelection = 0;
			
			String _debug() const {
				StringStream ss;
				ss << "{RespawnWindowSelect} ";
				PRINT_MEMBER(mSelection);
				return ss.str();
			}
		};

		namespace ExtendedTarget {
			struct AutoAddHaters : public Fixed<AutoAddHaters> {
				u8 mAction = 0;
			};
		}

		// C->S
		struct WhoRequest : public Fixed<WhoRequest> {
			char mCharacterName[64];
			i32 mRace = -1; // -1 = Not set.
			i32 mClass = -1; // -1 = Not set.
			i32 mLevelMinimum = -1; // -1 = Not set.
			i32 mLevelMaximum = -1;	// -1 = Not set.
			i32 mGM = -1; // -1 = Not set, 1 = /who (all) gm
			i32 mFlag = 0;	// -3 = LFG, -4 = Trader, -5 = Buyer
			u8 mUnknown[64];
			u32 mType;		// 0 = /who 3 = /who all

			String _debug() const {
				StringStream ss;
				ss << "{WhoRequest} ";
				PRINT_MEMBER(mCharacterName);
				PRINT_MEMBER(mRace);
				PRINT_MEMBER(mClass);
				PRINT_MEMBER(mLevelMinimum);
				PRINT_MEMBER(mLevelMaximum);
				PRINT_MEMBER(mGM);
				PRINT_MEMBER(mFlag);
				PRINT_MEMBER(mUnknown);
				PRINT_MEMBER(mType);
				return ss.str();
			}
		};

		// C->S
		struct AAAction : public Fixed<AAAction> {
			u32 mAction = 0;
			u32 mAbility = 0;
			u32 mUnknown = 0;
			u32 mExperienceToAA = 0;

			String _debug() const {
				StringStream ss;
				ss << "{AAAction} ";
				PRINT_MEMBER(mAction);
				PRINT_MEMBER(mAbility);
				PRINT_MEMBER(mUnknown);
				PRINT_MEMBER(mExperienceToAA);
				return ss.str();
			}
		};

		// C->S
		struct LeadershipExperienceToggle : public FixedT<LeadershipExperienceToggle, OP_LeadershipExpToggle> {
			u8 mValue = 0; // 0 = OFF, 1 = ON
		};

		// C->S
		struct EnvironmentDamage : public Fixed<EnvironmentDamage> {
			uint32 id;
			uint16 unknown4;
			uint32 damage;
			float unknown10;	// New to Underfoot - Seen 1
			uint8 unknown14[12];
			uint8 dmgtype;		// FA = Lava; FC = Falling
			uint8 unknown27[4];
			uint16 unknown31;	// New to Underfoot - Seen 66
			uint16 constant;		// Always FFFF
			uint16 unknown35;
		};

		// C->S
		struct ClaimRequest : public Fixed<ClaimRequest> {
			ClaimRequest() { memset(mName, 0, sizeof(mName)); }
			char mName[64];
			u32 mClaimID = 0;
			u32 mUnknown = 0;
		};

		// C->S
		struct Camp : public Fixed <Camp> {
			u32 mUnknown = 0;
		};

		// C->S
		struct PotionBelt : public Fixed<PotionBelt> {
			u32	mAction = 0;
			u32	mSlotID = 0;
			u32 mItemID = 0;
		};

		// C->S
		struct DeleteSpawn : public Fixed<DeleteSpawn> {
			u32 mUnknown = 0;
		};

		// C->S
		struct PetCommand : Fixed<PetCommand> {
			u32 mCommand = 0;
			u32 mUnknown = 0;
		};

		// C->S
		struct SaveRequest : public Fixed<SaveRequest> {
			u8 mUnknown0[192];
			u8 mUnknown1[176];
		};

		// C->S
		struct ApplyPoison : FixedT<ApplyPoison, OP_ApplyPoison> {
			u32 mSlotID = 0;
			u32 mSuccess = 0;
		};

		// C->S
		struct FindPersonRequest : public Fixed<FindPersonRequest> {
			u32 mUnknown0 = 0;
			u32 mSpawnID = 0;
			float mY = 0.0f;
			float mX = 0.0f;
			float mZ = 0.0f;
			u32 mUnknown1 = 0;
		};
		
		// C->S
		struct InspectRequest : public Fixed<InspectRequest> {
			u32 mTargetSpawnID = 0;
			u32 mCharacterSpawnID = 0;
		};

		// S->C
		struct InspectResponse : public FixedT<InspectResponse, OP_InspectAnswer> {
			InspectResponse() {
				memset(mItemNames, 0, sizeof(mItemNames[0][0]) * 23 * 64);
				memset(mItemIcons, 0, sizeof(mItemIcons));
				memset(mText, 0, sizeof(mText));
			}
			u32 mTargetSpawnID = 0;
			u32 mCharacterSpawnID = 0;
			char mItemNames[23][64];
			u32 mItemIcons[23];
			char mText[288];
		};

		// C->S
		struct InspectMessage : public Fixed<InspectMessage> {
			char mMessage[256];
		};

		// C->S
		struct RemoveBuffRequest : public Fixed<RemoveBuffRequest> {
			u32 mSlotID = 0;
			u32 mSpawnID = 0;
		};

		// S->C
		struct Time : public FixedT<Time, OP_TimeOfDay> {
			static EQApplicationPacket* construct(const u8 pHour, const u8 pMinute, const u8 pDay, const u8 pMonth, const u32 pYear) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mHour = pHour;
				payload->mMinute = pMinute;
				payload->mDay = pDay;
				payload->mMonth = pMonth;
				payload->mYear = pYear;

				return packet;
			}
			u8 mHour = 0;
			u8 mMinute = 0;
			u8 mDay = 0;
			u8 mMonth = 0;
			u32 mYear = 0;
		};

		// S->C
		struct DespawnActor : public FixedT<DespawnActor, OP_DeleteSpawn> {
			static EQApplicationPacket* construct(const u32 pSpawnID) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;

				return packet;
			}
			u32 mSpawnID = 0;
		};
	}

	namespace Raid {

		struct Action {
			u32 mAction = 0;
			char mCharacterName[64];
			u32 mUnknown = 0;
			char mLeaderName[64];
			u32 mParameter = 0;
		};

		typedef Fixed<Action> Invite;
	}

	namespace World {

		// C->S
		struct Connect : public Fixed<Connect> {
			char mInformation[64]; // Account ID and Account Key
			u8 mUnknown0[124];
			u8 mZoning;
			u8 mUnknown1[275];
		};

		// C->S
		struct EnterWorld : public Fixed<EnterWorld> {
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			u32 mTutorial = 0;
			u32 mReturnHome = 0;
		};

		// C->S
		struct DeleteCharacter : public VariableLengthPayload<DeleteCharacter> {
			char* mCharacterName = 0;
			static const bool sizeCheck(const std::size_t pSize) {
				return pSize >= Limits::Character::MIN_INPUT_LENGTH && pSize <= Limits::Character::MAX_INPUT_LENGTH;
			}
		};

		// C->S
		struct CreateCharacter : public Fixed<CreateCharacter> {
			u32 mClass = 0;
			u32 mHairColour = 0;
			u32 mBeardStyle = 0;
			u32 mBeardColour = 0;
			u32 mGender = 0;
			u32 mRace = 0;
			u32 mStartZoneID = 0;
			u32 mHairStyle = 0;
			u32 mDeity = 0;
			u32 mStrength = 0;
			u32 mStamina = 0;
			u32 mAgility = 0;
			u32 mDexterity = 0;
			u32 mWisdom = 0;
			u32 mIntelligence = 0;
			u32 mCharisma = 0;
			u32 mFaceStyle = 0;
			u32 mEyeColour1 = 0;
			u32 mEyeColour2 = 0;
			u32 mTutorial = 0;
			u32 mDrakkinHeritage = 0;
			u32 mDrakkinTattoo = 0;
			u32 mDrakkinDetails = 0;
		};

		// S->C
		// Based on: LogServer_Struct
		struct LogServer : public Fixed<LogServer> {
			u32 mUknown0 = 0;
			u8 mPVPEnabled = 0;
			u8 mUnknown1 = 0;
			u8 mUnknown2 = 0;
			u8 mUnknown3 = 0;
			u8 mFVEnabled = 0;
			u8 mUnknown4 = 0;
			u8 mUnknown5 = 0;
			u8 mUnknown6 = 0;
			u32 mUnknown7 = 0;
			u32 mUnknown8 = 0;
			u8 mUnknown9 = 0;
			char mWorldShortName[32];
			u8 mUnknown10[32];
			char mUnknown11[16];
			char mUnknown12[16];
			u8 mUnknown13[48];
			u32 mUnknown14;
			char mUnknown15[80];
			u8 mEnablePetitionWindow = 0;
			u8 mEnabledVoiceMacros = 0;
			u8 mEnableEmail = 0;
			u8 mEnableTutorial = 0;
		};

		// S->C
		struct ExpansionInfo : public FixedT<ExpansionInfo, OP_ExpansionInfo> {
			static EQApplicationPacket* construct(const u32 pExpansions) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mExpansions = pExpansions;

				return packet;
			}
			u32 mExpansions = 0;
		};

		// S<->C
		struct NameGeneration : public FixedT<NameGeneration, OP_RandomNameGenerator> {
			NameGeneration() { memset(mName, 0, sizeof(mName)); }
			static EQApplicationPacket* construct(const u32 pRace, const u32 pGender, const String& pName) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mRace = pRace;
				payload->mGender = pGender;
				strcpy(payload->mName, pName.c_str());

				return packet;
			}
			u32 mRace = 0;
			u32 mGender = 0;
			char mName[64];
		};

		// S->C
		struct ZoneServerInfo : public FixedT<ZoneServerInfo, OP_ZoneServerInfo> {
			static EQApplicationPacket* construct(const String& pIP, const u16 pPort) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mIP, pIP.c_str());
				payload->mPort = pPort;

				return packet;
			}
			char mIP[128];
			u16 mPort = 0;
		};

		// S->C
		struct ZoneUnavailable : public FixedT<ZoneUnavailable, OP_ZoneUnavail> {
			ZoneUnavailable() { memset(mZoneName, 0, sizeof(mZoneName)); }
			static EQApplicationPacket* construct(const String& pZoneName) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mZoneName, pZoneName.c_str());

				return packet;
			}
			char mZoneName[16];
			i32 mUnknown = 0;
		};

		// S->C
		struct ApproveWorld : public Fixed<ApproveWorld> {
			u8 mUnknown0[544];
		};

		// C->S
		struct ApproveName : public Fixed<ApproveName> {
			ApproveName() { memset(mName, 0, sizeof(mName)); }
			char mName[64];
			u32 mRace = 0;
			u32 mClass = 0;
			u32 mUnknown = 0;
			String _debug() const {
				StringStream ss;
				ss << "{ApproveName} ";
				PRINT_MEMBER(mName);
				PRINT_MEMBER(mRace);
				PRINT_MEMBER(mClass);
				PRINT_MEMBER(mUnknown);
				return ss.str();
			}
		};

		// S->C
		struct ApproveNameResponse : public FixedT<ApproveNameResponse, OP_ApproveName> {
			static EQApplicationPacket* construct(const u8 pResponse) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mResponse = pResponse;

				return packet;
			}
			u8 mResponse = 0; // 0 = Fail, 1 = Success.
		};
	}

	namespace LoginServer {
		namespace OpCode {
			enum : u16 {
				WorldInformation = 0x1008,
				WorldStatus = 0x1001,
				ConnectRequest = 0xAB00,
				ConnectResponse = 0xAB01,
				Authentication = 0x1002,
			};
		}

		// LS->W
		struct ConnectRequest : public Fixed<ConnectRequest> {
			u32 mAccountID = 0;
			u32 mWorldID = 0;
			u32 mFromID = 0;
			u32 mToID = 0;
		};

		// W->LS
		struct ConnectResponse : public Fixed<ConnectResponse> {
			u32 mAccountID = 0;
			u32 mWorldID = 0;
			u8 mResponse = ResponseID::Allowed;
			u32 mFromID = 0; // Ignored in LS.
			u32 mToID = 0; // Ignored in LS.
		};

		// LS->W
		namespace ClientAuthenticationLimits {
			static const auto MAX_ACCOUNT = 30;
			static const auto MAX_KEY = 30;
		}
		struct Authentication : public Fixed<Authentication> {
			u32 mLoginAccountID = 0;
			char mLoginAccountName[ClientAuthenticationLimits::MAX_ACCOUNT];
			char mLoginKey[ClientAuthenticationLimits::MAX_KEY];
			u8 mLoginServerAdmin = 0; // Ignored.
			i16 mWorldAdmin = 0; // Ignored.
			u32 mIP = 0;
			u8 mLocal = 0;
		};

		// W->LS
		struct WorldStatus : public Fixed<WorldStatus> {
			i32 mStatus = 0;
			i32 mPlayers = 0;
			i32 mZones = 0;
		};

		// W->LS
		struct WorldInformation : public Fixed<WorldInformation> {
			char mLongName[201];
			char mShortName[50];
			char	remote_address[125];			// DNS address of the server
			char	local_address[125];			// DNS address of the server
			char mAccount[31];
			char mPassword[31];
			char	protocolversion[25];	// Major protocol version number
			char	serverversion[64];		// minor server software version number
			uint8 mServerType = 0;
		};
	}

	namespace Group {
		// S->C
		struct Invite : public FixedT<Invite, OP_GroupInvite> {
			Invite() {
				memset(mFrom, 0, sizeof(mFrom));
				memset(mTo, 0, sizeof(mTo));
			}
			static EQApplicationPacket* construct(const String& pFrom, const String& pTo) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mFrom, pFrom.c_str());
				strcpy(payload->mTo, pTo.c_str());

				return packet;
			}

			char mTo[64];
			char mFrom[64];
			u32 mUnknown0 = 0;
			u32 mUnknown1 = 0;
			u32	mUnknown2 = 0;
			u32	mUnknown3 = 0;
			u32	mUnknown4 = 0;
		};

		// C->S
		struct AcceptInvite : public Fixed<AcceptInvite> {
			AcceptInvite() {
				memset(mName1, 0, sizeof(mName1));
				memset(mName2, 0, sizeof(mName2));
			}
			char mName1[64];
			char mName2[64];
			u32 mUnknown0 = 0;
			u32 mUnknown1 = 0;
			u32	mUnknown2 = 0;
			u32	mUnknown3 = 0;
			u32	mUnknown4 = 0;
			u32	mUnknown5 = 0;
		};

		// C->S
		struct DeclineInvite : public Fixed<DeclineInvite> {
			DeclineInvite() {
				memset(mName1, 0, sizeof(mName1));
				memset(mName2, 0, sizeof(mName2));
			}
			char mName1[64];
			char mName2[64];
			u32 mUnknown0 = 0;
			u32 mUnknown1 = 0;
			u32	mUnknown2 = 0;
			u32	mUnknown3 = 0;
			u32	mUnknown4 = 0;
			u32 mToggle = 0;
		};

		// C->S
		struct Disband : public Fixed<Disband> {
			char mName1[64];
			char mName2[64];
			u32 mUnknown0 = 0;
			u32 mUnknown1 = 0;
			u32	mUnknown2 = 0;
			u32	mUnknown3 = 0;
			u32	mUnknown4 = 0;
		};

		// S->C
		//typedef FixedT<Disband, OP_GroupDisbandYou> DisbandYou;
		struct DisbandYou : public FixedT<Disband, OP_GroupDisbandYou> {
			static EQApplicationPacket* construct(const String& pCharacterName, const String& pRemoverNamer) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mName1, pCharacterName.c_str());
				strcpy(payload->mName2, pRemoverNamer.c_str());

				return packet;
			}
		};

		// S->C
		//typedef FixedT<Disband, OP_GroupDisbandOther> DisbandOther;
		struct DisbandOther : public FixedT<Disband, OP_GroupDisbandOther> {
			static EQApplicationPacket* construct(const String& pCharacterName) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mName1, pCharacterName.c_str());
				strcpy(payload->mName2, pCharacterName.c_str());

				return packet;
			}
		};

		// S->C
		struct LeadershipAbilities : public FixedT<LeadershipAbilities, OP_GroupLeadershipAAUpdate> {
			LeadershipAbilities() {
				memset(mRanks, 0, sizeof(mRanks));
				memset(Unknown1, 0, sizeof(Unknown1));
				memset(mUnknown2, 0, sizeof(mUnknown2));
			}
			static EQApplicationPacket* construct() {
				auto packet = create();
				auto payload = convert(packet);
				payload->mRanks[0] = 1;
				//for (auto i = 0; i < 49; i++) {
				//	payload->Unknown1[i] = 1;
				//}

				return packet;
			}
			u32 mUnknown0 = 0;
			u32 mRanks[16];
			u32 Unknown1[49];
			u32 mNPCMarkerID = 0;
			u32 mUnknown2[13];
		};

		// C->S
		struct MakeLeader : public Fixed<MakeLeader> {
			MakeLeader() {
				memset(mCurrentLeader, 0, sizeof(mCurrentLeader));
				memset(mNewLeader, 0, sizeof(mNewLeader));
				memset(mUnknown1, 0, sizeof(mUnknown1));
			}
			u32 mUnknown0 = 0;
			char mCurrentLeader[64];
			char mNewLeader[64];
			char mUnknown1[324]; // LeadershipAbilityUpdate
		};

		// S->C
		struct LeaderUpdate : public FixedT<LeaderUpdate, OP_GroupLeaderChange> {
			LeaderUpdate() {
				memset(mUnknown0, 0, sizeof(mUnknown0));
				memset(mLeaderName, 0, sizeof(mLeaderName));
				memset(mUnknown1, 0, sizeof(mUnknown1));
			}
			static EQApplicationPacket* construct(const String& pLeaderName) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mLeaderName, pLeaderName.c_str());

				return packet;
			}
			char mUnknown0[64];
			char mLeaderName[64];
			char mUnknown1[20];
		};

		// C<->S
		struct Roles : FixedT<Roles,  OP_GroupRoles> {
			Roles() {
				memset(mTargetName, 0, sizeof(mTargetName));
				memset(mSetterName, 0, sizeof(mSetterName));
			}
			static EQApplicationPacket* construct(const String& pTargetName, const String& pSetterName, const u32 pRoleID, const u8 pToggle) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mTargetName, pTargetName.c_str());
				strcpy(payload->mSetterName, pSetterName.c_str());
				payload->mRoleID = pRoleID;
				payload->mToggle = pToggle;

				return packet;
			}
			char mTargetName[64]; // Character name of whoever is getting changed.
			char mSetterName[64]; // Character name of whoever is setting the role.
			u32 mUnknown0 = 0;
			u32 mUnknown1 = 0;
			u32 mUnknown2 = 0;
			u32 mRoleID = 0;
			u8 mToggle = 0; // 0 = OFF, 1 = ON.
			u8 mUnknown3 = 0;
			u8 mUnknown4 = 0;
			u8 mUnknown5 = 0;
		};
	}

	namespace Guild {

		// C->S
		struct Create : public Fixed<Create> {
			Create() { memset(mName, 0, sizeof(mName)); }
			char mName[64];
		};

		// C->S
		struct Delete : public Fixed<Delete> {
			u32 mGuildID = 0;
			char mCharacterName[64];
		};

		// C->S
		namespace InviteResponseType {
			enum : u32 { Accept = 0, Decline = 5, };
		}
		struct InviteResponse : public Fixed<InviteResponse> {
			InviteResponse() {
				memset(mInviter, 0, sizeof(mInviter));
				memset(mNewMember, 0, sizeof(mNewMember));
			}
			char mInviter[64];
			char mNewMember[64];
			u32 mResponse = 0;
			u32 mGuildID = 0;
		};

		// S->C
		struct RankUpdate : public FixedT<RankUpdate, OP_SetGuildRank> {
			RankUpdate() { memset(mCharacterName, 0, sizeof(mCharacterName)); }
			static EQApplicationPacket* construct(const u32 pRank, const String& pCharacterName) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mRank = pRank;
				strcpy(payload->mCharacterName, pCharacterName.c_str());

				return packet;
			}
			u32 mUnknown0 = 0;
			u32 mUnknown1 = 0;
			u32 mRank = 0;
			char mCharacterName[64];
			u32 mBanker = 0;
		};

		namespace CommandAction {
			enum : u32 { Invite = 0, Promote = 1, };
		}
		struct Command {
			Command() {
				memset(mToCharacter, 0, sizeof(mToCharacter));
				memset(mFromCharacter, 0, sizeof(mFromCharacter));
			}
			char mToCharacter[64];
			char mFromCharacter[64];
			u32 mGuildID = 0;
			//u8 unknown[2]; // for guildinvite all 0's, for remove 0=0x56, 2=0x02
			u32 mAction = 0;
		};

		// S<->C
		typedef FixedT<Command, OP_GuildInvite> Invite;

		typedef Fixed<Command> Remove;

		// C->S
		struct MakeLeader : public Fixed<MakeLeader> {
			MakeLeader() {
				memset(mCharacterName, 0, sizeof(mCharacterName));
				memset(mLeaderName, 0, sizeof(mLeaderName));
			}
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			char mLeaderName[Limits::Character::MAX_NAME_LENGTH];
		};

		// C->S
		struct Demote : public Fixed<Demote> {
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // Character doing the demoting
			char mDemoteName[Limits::Character::MAX_NAME_LENGTH]; // Character being demoted.
		};

		// C->S
		struct FlagsUpdate : public Fixed<FlagsUpdate> {
			u32 mUnknown = 0; // Probably Guild ID.
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // NOTE: UF does not send this
			char mOtherName[Limits::Character::MAX_NAME_LENGTH]; // Character whose status is being changed.
			u32 mFlags = 0;
		};

		// S->C
		// THIS DOES NOT WORK.
		struct MemberJoin : public FixedT<MemberJoin, OP_GuildManageAdd> {
			MemberJoin() { memset(mCharacterName, 0, sizeof(mCharacterName)); }
			static EQApplicationPacket* construct(const u32 pGuildID, const String& pCharacterName, const u32 pLevel, const u32 pClass, const u32 pZoneID) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mCharacterName, pCharacterName.c_str());
				payload->mGuildID = pGuildID;
				payload->mLevel = pLevel;
				payload->mClass = pClass;
				payload->mZoneID = pZoneID;

				return packet;
			}
			u32 mGuildID = 0;
			u32 mUnknown0 = 1;
			u32 mLevel = 0;
			u32 mClass = 0;
			u32 mRank = 0;
			u32 mZoneID = 0; // Probably instance in there too.
			u32 mUnknown1 = 0;
			char mCharacterName[64];
		};

		// S->C
		// THIS DOES NOT WORK.
		struct MemberRemove : public FixedT<MemberRemove, OP_GuildManageRemove> {
			MemberRemove() { memset(mCharacterName, 0, sizeof(mCharacterName)); }
			static EQApplicationPacket* construct(const u32 pGuildID, const String& pCharacterName) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mCharacterName, pCharacterName.c_str());
				payload->mGuildID = pGuildID;

				return packet;
			}
			u32 mGuildID = 0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
		};

		// S<->C
		struct MOTD : public FixedT<MOTD, OP_GuildMOTD> {
			MOTD() {
				memset(mCharacterName, 0, sizeof(mCharacterName));
				memset(mSetByName, 0, sizeof(mSetByName));
				memset(mMOTD, 0, sizeof(mMOTD));
			}
			static EQApplicationPacket* construct(const String& pSetByName, const String& pMOTD) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mSetByName, pSetByName.c_str());
				strcpy(payload->mMOTD, pMOTD.c_str());

				return packet;
			}
			u32 mUnknown0 = 0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // Receiver name.. why.
			char mSetByName[Limits::Character::MAX_NAME_LENGTH];
			u32 mUnknown1 = 0;
			char mMOTD[Limits::Guild::MAX_MOTD_LENGTH];
		};

		// S->C
		struct MOTDResponse : public FixedT<MOTD, OP_GetGuildMOTDReply> {
			MOTDResponse() {
				memset(mCharacterName, 0, sizeof(mCharacterName));
				memset(mSetByName, 0, sizeof(mSetByName));
				memset(mMOTD, 0, sizeof(mMOTD));
			}
			static EQApplicationPacket* construct(const String& pSetByName, const String& pMOTD) {
				auto packet = create();
				auto payload = convert(packet);
				strcpy(payload->mSetByName, pSetByName.c_str());
				strcpy(payload->mMOTD, pMOTD.c_str());

				return packet;
			}
			u32 mUnknown0 = 0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // Receiver name.. why.
			char mSetByName[Limits::Character::MAX_NAME_LENGTH];
			u32 mUnknown1 = 0;
			char mMOTD[Limits::Guild::MAX_MOTD_LENGTH];
		};

		// C->S
		typedef MOTDResponse MOTDRequest;

		// S<->C
		namespace GuildUpdateAction {
			enum : u32 { URL = 0, Channel = 1, };
		}
		struct GuildUpdate : public FixedT<GuildUpdate, OP_GuildUpdateURLAndChannel> {
			GuildUpdate() {
				memset(mUnknown0, 0, sizeof(mUnknown0));
				memset(mText, 0, sizeof(mText));
				memset(mUnknown1, 0, sizeof(mUnknown1));
			}
			static EQApplicationPacket* construct(const u32 pAction, const String& pText) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mAction = pAction;
				strcpy(payload->mText, pText.c_str());

				return packet;
			}
			u32 mAction = 0;
			char mUnknown0[76];
			char mText[512];
			char mUnknown1[3584];
		};

		// C->S
		struct PublicNote : public Fixed<PublicNote> {
			PublicNote() {
				memset(mSenderName, 0, sizeof(mSenderName));
				memset(mTargetName, 0, sizeof(mTargetName));
				memset(mNote, 0, sizeof(mNote));
			}
			u32 mUnknown = 0; // Probably Guild ID.
			char mSenderName[Limits::Character::MAX_NAME_LENGTH];
			char mTargetName[Limits::Character::MAX_NAME_LENGTH];
			char mNote[256];
		};

		// S->C
		struct MemberZoneUpdate : public FixedT<MemberZoneUpdate, OP_GuildMemberUpdate> {
			MemberZoneUpdate() { memset(mMemberName, 0, sizeof(mMemberName)); }
			static EQApplicationPacket* construct(const u32 pGuildID, const String& pMemberName, const u32 pZoneID, const u16 pInstanceID, const u32 pLastSeen) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mGuildID = pGuildID;
				strcpy(payload->mMemberName, pMemberName.c_str());
				payload->mZoneID = pZoneID;
				payload->mInstanceID = pInstanceID;
				payload->mLastSeen = pLastSeen;

				return packet;
			}
			u32 mGuildID = 0;
			char mMemberName[64];
			u32 mZoneID;
			u16 mInstanceID;
			u32 mLastSeen;
		};

		// S->C
		// THIS DOES NOT WORK.
		struct LevelUpdate : public FixedT<LevelUpdate, OP_GuildMemberLevelUpdate> {
			LevelUpdate() { memset(mMemberName, 0, sizeof(mMemberName)); }
			static EQApplicationPacket* construct(const u32 pGuildID, const String& pMemberName, const u32 pLevel) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mGuildID = pGuildID;
				strcpy(payload->mMemberName, pMemberName.c_str());
				payload->mLevel = pLevel;

				return packet;
			}
			u32 mGuildID = 0;
			char mMemberName[64];
			u32 mLevel = 0;
		};

		// C->S
		struct StatusRequest : public Fixed<StatusRequest> {
			StatusRequest() {
				memset(mName, 0, sizeof(mName));
				memset(mUnknown, 0, sizeof(mUnknown));
			}
			char mName[64];
			u8 mUnknown[72];
		};
	}

	// C->S
	struct PositionUpdate : public Fixed<PositionUpdate> {
		u16 mSpawnID = 0;
		u16 mSequence = 0;
		/*0004*/ u8		unknown0004[4];		// ***Placeholder
		float mX = 0.0f;
		float mY = 0.0f;
		/*0016*/ signed		delta_heading : 10;	// change in heading
		unsigned	padding0036 : 10;		// animation
		unsigned	padding0016 : 12;		// ***Placeholder
		float mDeltaX = 0.0f;
		float mDeltaY = 0.0f;
		float mZ = 0.0f;
		float mDeltaZ = 0.0f;
		/*0036*/ unsigned	animation : 10;		// ***Placeholder
		unsigned	heading : 12;			// Directional heading
		unsigned	padding0037 : 10;		// ***Placeholder
		/*0040*/
	};

	// S->C
	struct ActorPosition : public FixedT<ActorPosition, OP_ClientUpdate> {
		u16 mSpawnID = 0;
		signed padding0000 : 12;
		signed mDeltaX : 13;
		signed padding0005 : 7;
		signed mDeltaHeading : 10;
		signed mDeltaY : 13;
		signed padding0006 : 9;
		signed mY : 19;
		signed mAnimation : 13;
		unsigned mHeading : 12;
		signed mX : 19;
		signed padding0014 : 1;
		signed mZ : 19;
		signed mDeltaZ : 13;      
	};

	struct ActorData {
		ActorData() {
			memset(mName, 0, sizeof(mName));
			memset(mSurname, 0, sizeof(mSurname));
			memset(mTitle, 0, sizeof(mTitle));
			memset(mSuffix, 0, sizeof(mSuffix));
			memset(mUnknowns, 0, sizeof(mUnknowns));
			memset(&mFlags, 0, sizeof(mFlags));
			memset(&mPosition, 0, sizeof(mPosition));
			memset(mColours, 0, sizeof(mColours));

			mFlags.mTargetable = 1;
			mFlags.mTargetableWithHotkey = 1;
			mFlags.mShowName = 1;
			mFlags.mShowHelm = 0;
		}
		char mName[100]; // Variable
		u32 mSpawnID = 0;
		u8 mLevel = 1;
		// Destructible Object writes 4 bytes, float = 10.
		// Else writes 4 bytes, float, SpawnSize - 0.7 comment = eye height?
		float mViewOffset = 0.7f;
		u8 mActorType = 0;
		struct ActorFlags {
			unsigned mIsPet : 1;		// Could be 'is summoned pet' rather than just is pet.
			unsigned mIsAFK : 1;		// 0=no, 1=afk
			unsigned mIsSneaking : 1;
			unsigned mLFG : 1;
			unsigned __Unknown0 : 1; // No observed effects.
			// Invisible Start
			unsigned mIsInvisible : 1; // Normal
			unsigned __Unknown1 : 1; // Undead
			unsigned __Unknown2 : 1; // Animals
			unsigned __Unknown3 : 1; // Others? AA invis types?
			unsigned __Unknown4 : 1;
			unsigned __Unknown5 : 1;
			unsigned __Unknown6 : 1;
			unsigned __Unknown7 : 1;
			unsigned __Unknown8 : 1;
			unsigned __Unknown9 : 1;
			unsigned __Unknown10 : 1;
			unsigned __Unknown11 : 1;
			// Invisible End.
			unsigned mIsGM : 1;
			unsigned mAnonymous : 2; // See Constants AnonType
			unsigned mGender : 2; // See Constants Gender
			unsigned mIsLD : 1;
			unsigned betabuffed : 1;
			unsigned mShowHelm : 1; // Works for NPC
			unsigned padding26 : 1;
			unsigned mTargetable : 1;
			unsigned mTargetableWithHotkey : 1;
			unsigned mShowName : 1;
			unsigned statue : 1; // No observed effect on NPCs. Added <Unknown Guild> below PC name.
			unsigned mIsTrader : 1; // No observed effect on NPCs. Added "Trader" to PC name.
			unsigned mIsBuyer : 1; // No observed effect on NPCs. Added "Buyer" to PC name + <Unknown Guild> + removed title.
		};
		ActorFlags mFlags;
		u8 mOtherFlags = 0;
		float __Unknown0 = -1; // Destructible writes 0, else writes -1 float.
		float __Unknown1 = 0; // HC as 0

		/*
		struct Destructible {
		Destructible() {
		memset(mModel, 0, sizeof(mModel));
		memset(mName, 0, sizeof(mName));
		memset(mUnknown, 0, sizeof(mUnknown));
		}
		char mModel[100]; // Variable
		char mName[100]; // Variable
		char mUnknown[100]; // Variable, Unknown

		u32 mAppearance = 0;
		u32 mUnknown0 = 0;

		u32 mID0 = 0;
		u32 mID1 = 0;
		u32 mID2 = 0;
		u32 mID3 = 0;

		u32 mUnknown1 = 0;
		u32 mUnknown2 = 0;
		u32 mUnknown3 = 0;
		u32 mUnknown4 = 0;
		u32 mUnknown5 = 0;
		u32 mUnknown6 = 0;
		u8 mUnknown7
		u32 mUnknown8 = 0;
		};
		Destructible mDestructible;
		*/

		float mSize = 0.0f;
		u8 mFaceStyle = 0;
		float mWalkSpeed = 3.0f;
		float mRunSpeed = 6.0f;
		u32 mRace = 0;
		u8 mPropertyCount = 1; // HC 1. Add 4 bytes per count after.
		u32 mBodyType = 0;
		u8 mHPPercent = 100; // Percentage: 0-100
		u8 mHairColour = 0;
		u8 mBeardColour = 0;
		u8 mLeftEyeColour = 0;
		u8 mRightEyeColour = 0;
		u8 mHairStyle = 0;
		u8 mBeardStyle = 0;
		u32 mDrakkinHeritage = 0;
		u32 mDrakkinTattoo = 0;
		u32 mDrakkinDetails = 0;
		u8 __Unknown3 = 0;
		u32 mDeity = 0;
		u32 mGuildID = GuildID::None;
		u32 mGuildRank = 0;
		u8 mClass = 0;
		u8 mPVP = 0;
		u8 mStandState = 100;
		u8 mLight = 0;
		u8 mFlyMode = 0;
		u8 mTexture = 0; // Does not affect Characters.
		u8 __Unknown5 = 0;
		u8 __Unknown6 = 0;
		u8 mShowHelm = 0; // TODO: Figure out how this works.

		char mSurname[100];

		u32 mAAtitle = 0; // TODO: Figure out how this works.
		u8 __Unknown8 = 0;
		u32 mOwnerSpawnID = 0; // Pet Owner.
		u8 __Unknown9 = 0;
		u32 __Unknown10 = 0; // - Stance 64 = normal 4 = aggressive 40 = stun/mezzed
		u32 __Unknown11 = 0;
		u32 __Unknown12 = 0;
		u32 __Unknown13 = 0;
		u32 __Unknown14 = 0xFFFFFFFF; // Copied HC
		u16 __Unknown15 = 0xFFFF; // Copied HC

		ActorPosition mPosition;

		u32 mColours[9];

		struct Equipment {
			u32 mMaterial = 0;
			u32 mEquip1 = 0;
			u32 mItemID = 0;
		};
		Equipment mEquipment[9];

		char mTitle[100]; // Variable
		char mSuffix[100]; // Variable

		u32 __Unknown16 = 0;
		u32 __Unknown17 = 0;
		u8 mIsMercenary = 0;

		char mUnknowns[28];
	};
}
#pragma pack()