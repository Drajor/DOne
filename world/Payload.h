#pragma once

#include "Constants.h"

enum GuildUpdateAction : uint32 {
	GUILD_URL = 0,
	GUILD_CHANNEL = 1
};

struct Colour
{
	union
	{
		struct
		{
			uint8 mBlue;
			uint8 mGreen;
			uint8 mRed;
			uint8 mUseTint;	// if there's a tint this is FF
		} mRGB;
		uint32 mColour;
	};
};

#pragma pack(1)
namespace Payload {

	template <typename T>
	struct FixedSizedPayload {
		inline static T* convert(unsigned char* pData) { return reinterpret_cast<T*>(pData); }
		inline static const bool sizeCheck(const std::size_t pSize) { return pSize == sizeof(T); }
	};

	template <typename T>
	struct VariableLengthPayload {
		inline static T* convert(unsigned char* pData) { return reinterpret_cast<T*>(pData); }
	};

	template <typename T>
	class ServerToClient {
	public: inline static const std::size_t size() { return sizeof(T); }
	};

	template <typename T>
	struct FixedLength_And_ServerToClient {
		inline static T* convert(unsigned char* pData) { return reinterpret_cast<T*>(pData); }
		inline static const bool sizeCheck(const std::size_t pSize) { return pSize == sizeof(T); }
		inline static const std::size_t size() { return sizeof(T); }
	};

	namespace Zone {

		struct ZoneEntry : public FixedSizedPayload<ZoneEntry> {
			uint32 mUnknown = 0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
		};

		// C->S
		// Based on: SetTitle_Struct
		struct SetTitle : public FixedSizedPayload<SetTitle> {
			enum : uint32 { SET_PREFIX = 0, SET_SUFFIX = 1 };
			uint32 mOption = SET_PREFIX;
			uint32 mTitleID = 0;
		};

		// S->C
		// Based on: SetTitleReply_Struct
		struct TitleUpdate : public FixedLength_And_ServerToClient<TitleUpdate> {
			enum : uint32 { UPDATE_PREFIX = 0, UPDATE_SUFFIX = 1 };
			uint32 mOption = UPDATE_PREFIX;
			char mTitle[32];
			uint32 mSpawnID = 0;
		};

		// C->S
		struct FaceChange : public FixedSizedPayload<FaceChange> {
			uint8 mHairColour = 0;
			uint8 mBeardColour = 0;
			uint8 mLeftEyeColour = 0;
			uint8 mRightEyeColour = 0;
			uint8 mHairStyle = 0;
			uint8 mBeardStyle = 0;
			uint8 mFaceStyle = 0;
			uint32 mDrakkinHeritage = 0;
			uint32 mDrakkinTattoo = 0;
			uint32 mDrakkinDetails = 0;
		};

		// S->C
		struct WearChange : public FixedSizedPayload<WearChange>, public ServerToClient<WearChange> {
			uint16 mSpawnID = 0;
			uint32 mMaterialID = 0;
			uint32 mUnused0 = 0;
			uint32 mUnused1 = 0;
			uint32 mUnused2 = 0;
			uint32 mUnused3 = 0;
			Colour mColour;
			uint8 mSlotID = 0;
		};
		
		// C->S
		struct AutoAttack : public FixedSizedPayload<AutoAttack> {
			bool mAttacking = false;
			char mUnknown[3];
		};

		// C<->S
		// Based on: MemorizeSpell_Struct
		struct MemoriseSpell : public FixedLength_And_ServerToClient<MemoriseSpell> {
			enum Action : uint32 { SCRIBE = 0, MEMORISE = 1, UNMEMORISE = 2, SPELLBAR_REFRESH = 3 };
			uint32 mSlot = 0;
			uint32 mSpellID = 0;
			Action mAction = SCRIBE;
			uint32 mUnknown0 = 0;
		};

		// C<->S
		struct DeleteSpell : public FixedLength_And_ServerToClient<DeleteSpell> {
			int16 mSpellBookSlot = 0;
			uint8 mUnknown0[2];
			uint8 mSuccess = 0;
			uint8 mUnknown1[3];
		};

		// C->S
		struct LoadSpellSet : public FixedSizedPayload<LoadSpellSet> {
			uint32 mSpellIDs[Limits::Character::MAX_SPELLS_MEMED];	// 0xFFFFFFFF if no action, slot number if to unmem starting at 0
			uint32 mUnknown0 = 0;
		};

		// C->S
		struct SwapSpell : public FixedSizedPayload<SwapSpell> {
			uint32 mFrom = 0;
			uint32 mTo = 0;
		};

		// C->S
		struct CastSpell : public FixedSizedPayload<CastSpell> {
			uint32 mSlot = 0;
			uint32 mSpellID = 0;
			uint32 mInventorySlot = 0; // slot for clicky item, 0xFFFF = normal cast
			uint32 mTargetID; // SpawnID?
			uint8 mUnknown0[4];
		};

		// S->C
		// Based on: BeginCast_Struct
		struct BeginCast : public FixedLength_And_ServerToClient<BeginCast> {
			uint16 mSpawnID = 0; // Caster
			uint16 mSpellID = 0;
			uint32 mCastTime = 0; // MS
		};

		// S->C
		// Based on: InterruptCast_Struct
		struct InterruptCast : public FixedLength_And_ServerToClient<InterruptCast> {
			uint32 mSpawnID = 0;
			uint32 mMessageID = 0;
			char mMessage[0];
		};

		// S->C
		// Based on: ManaChange_Struct
		struct ManaChange : public FixedLength_And_ServerToClient<ManaChange> {
			uint32 mMana = 0;
			uint32 mEndurance = 0;
			uint32 mSpellID = 0;
			uint32 mUnknown0 = 0;
		};

		// S->C
		// Based on: Action_Struct
		struct Action : public FixedLength_And_ServerToClient<Action> {
			uint16 mTargetSpawnID = 0;
			uint16 mSourceSpawnID = 0;
			uint16 mCasterLevel = 1;
			uint16 instrument_mod = 0;
			uint32 bard_focus_id = 0;
			uint16 mUnknown0 = 0;
			uint32 mSequence = 0;
			uint32 mUnknown1 = 0;
			uint8 mType = 0;		// 231 (0xE7) for spells
			uint32 mUnknown2 = 0;
			uint16 mSpellID = 0; // uint16?? hmm
			uint8 mUnknown3 = 0;
			uint8 buff_unknown = 0;	// if this is 4, a buff icon is made
		};

		// C->S
		// Based on: CombatAbility_Struct
		struct CombatAbility : public FixedSizedPayload<CombatAbility> {
			uint32 mTargetID = 0;
			uint32 mAttack = 0;
			uint32 mSkill = 0;
		};

		// C->S
		struct Taunt : FixedSizedPayload<Taunt> {
			uint32 mSpawnID = 0;
		};

		// C<->S
		// Yes the Client sends this for some magical reason.
		struct Consider : FixedLength_And_ServerToClient<Consider> {
			uint32 mSpawnID = 0;
			uint32 mTargetSpawnID = 0;
			uint32 mFaction = 0;
			uint32 mTargetLevel = 1;
			int32 cur_hp = 0; // Ignore.
			int32 max_hp = 0; // Ignore.
			uint8 mIsPVP; // 0/1
			uint8 mUnknown0[3];
		};

		// C<->S
		struct Surname : public FixedLength_And_ServerToClient<Surname> {
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			uint32 mApproved = 0;
			char mLastName[Limits::Character::MAX_LAST_NAME_LENGTH];
		};

		// S->C
		// Based on: GMLastName_Struct
		struct SurnameUpdate : public FixedLength_And_ServerToClient<SurnameUpdate> {
			char mCharaterName[Limits::Character::MAX_NAME_LENGTH];
			char mGMName[Limits::Character::MAX_NAME_LENGTH];
			char mLastName[Limits::Character::MAX_NAME_LENGTH];
			uint16 mUnknown0[4];
		};

		// Based on: SpawnAppearance_Struct
		struct SpawnAppearance : public FixedLength_And_ServerToClient<SpawnAppearance> {
			uint16 mSpawnID = 0;
			uint16 mType = 0;
			uint32 mParameter = 0;
		};

		// S->C
		// Based on: Animation_Struct
		struct Animation : public FixedLength_And_ServerToClient<Animation> {
			uint16 mSpawnID = 0;
			uint8 mAction = 0;
			uint8 mValue = 0;
		};

		// S->C
		// Based on: SkillUpdate_Struct
		struct SkillUpdate : public FixedLength_And_ServerToClient<SkillUpdate> {
			uint32 mID = 0;
			uint32 mValue = 0;
		};

		// S->C
		// Based on: Death_Struct
		struct Death : public FixedLength_And_ServerToClient<Death> {
			uint32 mSpawnID = 0;
			uint32 mKillerSpawnID = 0;
			uint32 mCorpseID = 0; // ??
			uint32 mBindZoneID = 0;
			uint32 mSpellID = 0xFFFFFFFF;
			uint32 mSkillID = 0;
			uint32 mDamage = 0;
			uint32 mUnknown0 = 0;
		};

		// C->S
		struct LootBeginRequest : public FixedSizedPayload<LootBeginRequest> {
			uint32 mSpawnID = 0;
		};
		// C->S
		struct LootEndRequest : public FixedSizedPayload<LootEndRequest> {
			uint32 mSpawnID = 0;
		};

		// S->C
		// Based on: moneyOnCorpseStruct
		struct LootResponse : public FixedLength_And_ServerToClient<LootResponse> {
			enum Response : uint8 { 
				ALREADY = 0,
				LOOT = 1,
				DENY = 2,
				TOO_FAR = 3,
				HOSTILE = 7, // "You cannot loot while a hostile create is aware of your presence"
			};
			uint8 mResponse = DENY;
			uint8 mUnknown0 = 0x5a;
			uint8 mUnknown1 = 0x40;
			uint8 mUnknown2 = 0;
			uint32 mPlatinum = 0;
			uint32 mGold = 0;
			uint32 mSilver = 0;
			uint32 mCopper = 0;
		};
		
		// C->S
		// Based on: ClientTarget_Struct
		struct Target : public FixedSizedPayload<Target> {
			uint32 mSpawnID = 0;
		};
	}

	namespace World {

		// C->S
		struct LoginInformation : public FixedSizedPayload<LoginInformation> {
			char mInformation[64]; // Account ID and Account Key
			uint8 mUnknown0[124];
			uint8 mZoning;
			uint8 mUnknown1[275];
		};

		// C->S
		struct EnterWorld : public FixedSizedPayload<EnterWorld> {
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			uint32 mTutorial = 0;
			uint32 mReturnHome = 0;
		};

		// C->S
		struct DeleteCharacter : public VariableLengthPayload<DeleteCharacter> {
			char* mCharacterName = 0;
			static const bool sizeCheck(const std::size_t pSize) {
				return pSize >= Limits::Character::MIN_NAME_LENGTH && pSize <= Limits::Character::MAX_NAME_LENGTH;
			}
		};

		struct CreateCharacter : public FixedSizedPayload<CreateCharacter> {
			uint32 mClass = 0;
			uint32 mHairColour = 0;
			uint32 mBeardColour = 0;
			uint32 mBeardStyle = 0;
			uint32 mGender = 0;
			uint32 mRace = 0;
			uint32 mStartZoneID = 0;
			uint32 mHairStyle = 0;
			uint32 mDeity = 0;
			uint32 mStrength = 0;
			uint32 mStamina = 0;
			uint32 mAgility = 0;
			uint32 mDexterity = 0;
			uint32 mWisdom = 0;
			uint32 mIntelligence = 0;
			uint32 mCharisma = 0;
			uint32 mFaceStyle = 0;
			uint32 mEyeColour1 = 0;
			uint32 mEyeColour2 = 0;
			uint32 mDrakkinHeritage = 0;
			uint32 mDrakkinTattoo = 0;
			uint32 mDrakkinDetails = 0;
		};
	}

	namespace LoginServer {

		struct ConnectRequest : public FixedSizedPayload<ConnectRequest> {
			uint32 mAccountID = 0;
			uint32 mWorldID = 0;
			uint32 mFromID = 0;
			uint32 mToID = 0;
		};

		struct ConnectResponse : public FixedSizedPayload<ConnectResponse> {
			uint32 mAccountID = 0;
			uint32 mWorldID = 0;
			ResponseID mResponse = ResponseID::ALLOWED;
			uint32 mFromID = 0;
			uint32 mToID = 0;
		};

		struct ClientAuthentication : public FixedSizedPayload<ClientAuthentication> {
			uint32 mAccountID = 0;
			char mAccountName[Limits::LoginServer::MAX_ACCOUNT_NAME_LENGTH];
			char mKey[Limits::LoginServer::MAX_KEY_LENGTH];
			uint8 mLoginServerAdmin = 0; // Ignored.
			int16 mWorldAdmin = 0; // Ignored.
			uint32 mIP = 0;
			uint8 mLocal = 0;
		};
	}

	namespace Guild {

		struct MakeLeader : public FixedSizedPayload<MakeLeader> {
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			char mLeaderName[Limits::Character::MAX_NAME_LENGTH];
		};

		struct Demote : public FixedSizedPayload<Demote> {
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // Character doing the demoting
			char mDemoteName[Limits::Character::MAX_NAME_LENGTH]; // Character being demoted.
		};

		// Used for changing both banker and alt status of a guild member.
		struct BankerAltStatus : public FixedSizedPayload<BankerAltStatus> {
			uint32 mUnknown = 0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // NOTE: UF does not send this
			char mOtherName[Limits::Character::MAX_NAME_LENGTH]; // Character whose status is being changed.
			uint32 mStatus = 0;
		};

		// S->C
		struct Remove : public FixedSizedPayload<Remove>, public ServerToClient<Remove> {
			GuildID mGuildID;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
		};
		
		struct LevelUpdate {
			GuildID mGuildID; // why?!
			char mMemberName[Limits::Character::MAX_NAME_LENGTH];
			uint32 mLevel;
		};

		struct MOTD {
			uint32 mUnknown0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // Receiver name.. why.
			char mSetByName[Limits::Character::MAX_NAME_LENGTH];
			uint32 mUnknown1;
			char mMOTD[Limits::Guild::MAX_MOTD_LENGTH];
		};

		struct GuildUpdate {
			enum Action : uint32 { GUILD_URL = 0, GUILD_CHANNEL = 1 };
			Action mAction;
			char mUnknown0[76];
			char mText[512];
			char mUnknown1[3584];
		};

		struct PublicNote {
			uint32 mUnknown;
			char mSenderName[Limits::Character::MAX_NAME_LENGTH];
			char mTargetName[Limits::Character::MAX_NAME_LENGTH];
			char mNote[1]; // NOTE: I believe this gets cut off to length 100 by underlying code.
		};

		struct MemberUpdate {
			GuildID mGuildID;
			char mMemberName[Limits::Character::MAX_NAME_LENGTH];
			ZoneID mZoneID;
			InstanceID mInstanceID;
			uint32 mLastSeen;
		};
	}

	struct SpawnData {
		uint8 mUnnknown0 = 0;
		GMStatus mGM = GM_OFF;
		uint8 mUnknown1 = 0;
		AATitle mAATitle = NONE;
		uint8 mUnknown2 = 0;
		AnonType mAnonymous = AT_None;
		uint8 mFaceStyle = 0;
		char mName[Limits::Character::MAX_NAME_LENGTH];
		DeityID mDeity = 0;
		uint16 unknown3 = 0;
		float mSize = 5.0f;
		uint32 mUnknown4 = 0;
		ActorType mActorType = AT_PLAYER;
		uint8 mIsInvisible = 0;
		uint8 mHairColour = 0;
		uint8 mCurrentHP = 100;
		uint8 max_hp = 100;				// (name prolly wrong)takes on the value 100 for players, 100 or 110 for NPCs and 120 for PC corpses...
		uint8 mIsFindable = 0;
		uint8 mUnknown5[3];
		uint16 mSpawnID2 = 0;
		/*0094*/ signed	deltaHeading : 10;	// change in heading
		/*????*/ signed	x : 19;				// x coord
		/*????*/ signed	padding0054 : 3;		// ***Placeholder
		/*0098*/ signed	y : 19;				// y coord
		/*????*/ signed	animation : 10;		// animation
		/*????*/ signed	padding0058 : 3;		// ***Placeholder
		/*0102*/ signed	z : 19;				// z coord
		/*????*/ signed	deltaY : 13;			// change in y
		/*0106*/ signed	deltaX : 13;			// change in x
		/*????*/ unsigned	heading : 12;		// heading
		/*????*/ signed	padding0066 : 7;		// ***Placeholder
		/*0110*/ signed	deltaZ : 13;			// change in z
		/*????*/ signed	padding0070 : 19;		// ***Placeholder
		uint8 mLeftEyeColour = 0;
		uint8 mUnknown6[11];
		uint8 mStandState = 100;	// stand state for SoF+ 0x64 for normal animation
		uint32 mDrakkinHeritage = 0;
		uint32 mDrakkinTattoo = 0;
		uint32 mDrakkinDetails = 0;
		uint8 mShowHelm = 0;
		uint8 mUnknown7[4];
		uint8 mIsNPC = 0;
		uint8 mHairStyle = 0;
		uint8 mBeardStyle = 0;
		uint8 mUnknown8[4];
		uint8 mLevel = 1;
		uint8 mUnknown9[4];
		uint8 mBeardColour = 0;
		char mSuffix[Limits::Character::MAX_SUFFIX_LENGTH];
		SpawnID mOwnerSpawnID = 0;
		GuildRank mGuildRank = GR_None;
		uint8 mUnknown10[3];
		uint32 mEquipmentMaterials[MAX_MATERIAL_SLOTS];
		float mRunSpeed = 1.0f;
		uint8 mIsAFK = 0;
		GuildID mGuildID = NO_GUILD;
		char mTitle[Limits::Character::MAX_TITLE_LENGTH];
		uint8 mUnknown11 = 0;
		uint8 set_to_0xFF[8];	// ***Placeholder (all ff)
		uint8 mHelmTexture = 0;
		RaceID mRace = 0;
		uint32 mUnknown12 = 0;
		char mLastName[Limits::Character::MAX_LAST_NAME_LENGTH];
		float mWalkSpeed = 0.5f;
		uint8 mUnknown13 = 0;
		uint8 mIsPet = 0;
		uint8 mLight = 0;
		ClassID mClass = Warrior;
		uint8 mRightEyeColour = 0;
		FlyMode mFlyMode = FM_NONE;
		GenderID mGender = G_FEMALE;
		BodyType mBodyType = BT_Humanoid;
		uint8 mUnknown14[3];
		union
		{
			/*0339*/ uint8 equip_chest2 = 0;	// Second place in packet for chest texture (usually 0xFF in live packets)
			// Not sure why there are 2 of them, but it effects chest texture!
			/*0339*/ uint8 mount_color;		// drogmor: 0=white, 1=black, 2=green, 3=red
			// horse: 0=brown, 1=white, 2=black, 3=tan
		};
		uint32 mSpawnID = 0;
		uint8 mUnknown15[3];
		uint8 mIsMercenary = 0;
		Colour mEquipmentColours[MAX_MATERIAL_SLOTS];
		uint8 mIsLFG = 0;
		bool mIsDestructibleObject = false;
		char DestructibleModel[64];	// Model of the Destructible Object - Required - Seen "DEST_TNT_G"
		char DestructibleName2[64];	// Secondary name - Not Required - Seen "a_tent"
		char DestructibleString[64];	// Unknown - Not Required - Seen "ZoneActor_01186"
		uint32 DestructibleAppearance = 0;	// Damage Appearance
		uint32 DestructibleUnk1 = 0;
		uint32 DestructibleID1 = 0;
		uint32 DestructibleID2 = 0;
		uint32 DestructibleID3 = 0;
		uint32 DestructibleID4 = 0;
		uint32 DestructibleUnk2 = 0;
		uint32 DestructibleUnk3 = 0;
		uint32 DestructibleUnk4 = 0;
		uint32 DestructibleUnk5 = 0;
		uint32 DestructibleUnk6 = 0;
		uint32 DestructibleUnk7 = 0;
		uint8 DestructibleUnk8 = 0;
		uint32 DestructibleUnk9 = 0;

	};
}
#pragma pack()