#pragma once

#include "Constants.h"
#include "Utility.h"
#include "../common/EQPacket.h"

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
	struct Fixed {
		inline static T* convert(const EQApplicationPacket* pPacket) { return reinterpret_cast<T*>(pPacket->pBuffer); }
		inline static T* convert(unsigned char* pData) { return reinterpret_cast<T*>(pData); }
		inline static const bool sizeCheck(const std::size_t pSize) { return pSize == sizeof(T); }
		inline static const bool sizeCheck(const EQApplicationPacket* pPacket) { return pPacket->size == size(); }
		inline static const std::size_t size() { return sizeof(T); }
		inline static EQApplicationPacket* make(EmuOpcode pOpCode) { return new EQApplicationPacket(pOpCode, size()); }
	};

	template <typename T, EmuOpcode OpCode>
	struct FixedT {
		inline static T* convert(const EQApplicationPacket* pPacket) { return reinterpret_cast<T*>(pPacket->pBuffer); }
		inline static const bool sizeCheck(const EQApplicationPacket* pPacket) { return pPacket->size == size(); }
		inline static const std::size_t size() { return sizeof(T); }
		inline static EQApplicationPacket* create() { return new EQApplicationPacket(OpCode, size()); }
		inline static EQApplicationPacket* create(T pPayload) { return new EQApplicationPacket(OpCode, (unsigned char*)&pPayload, size()); }
	};

	template <typename T>
	struct VariableLengthPayload {
		inline static T* convert(unsigned char* pData) { return reinterpret_cast<T*>(pData); }
	};

	enum ItemPacketType
	{
		ItemPacketViewLink = 0x00,
		ItemPacketTradeView = 0x65,
		ItemPacketLoot = 0x66,
		ItemPacketTrade = 0x67,
		ItemPacketCharInventory = 0x69,
		ItemPacketSummonItem = 0x6A,
		ItemPacketTributeItem = 0x6C,
		ItemPacketMerchant = 0x64,
		ItemPacketWorldContainer = 0x6B,
		ItemPacketCharmUpdate = 0x6E
	};

	namespace Zone {

		struct ZoneEntry : public Fixed<ZoneEntry> {
			uint32 mUnknown = 0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
		};

		// C->S
		// Based on: SetTitle_Struct
		struct SetTitle : public Fixed<SetTitle> {
			enum : uint32 { SET_PREFIX = 0, SET_SUFFIX = 1 };
			uint32 mOption = SET_PREFIX;
			uint32 mTitleID = 0;
		};

		// S->C
		// Based on: SetTitleReply_Struct
		struct TitleUpdate : public Fixed<TitleUpdate> {
			enum : uint32 { UPDATE_PREFIX = 0, UPDATE_SUFFIX = 1 };
			uint32 mOption = UPDATE_PREFIX;
			char mTitle[32];
			uint32 mSpawnID = 0;
		};

		// C->S
		struct FaceChange : public Fixed<FaceChange> {
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
		struct WearChange : public Fixed<WearChange> {
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
		struct AutoAttack : public Fixed<AutoAttack> {
			bool mAttacking = false;
			char mUnknown[3];
		};

		// C<->S
		// Based on: MemorizeSpell_Struct
		struct MemoriseSpell : public Fixed<MemoriseSpell> {
			enum Action : uint32 { SCRIBE = 0, MEMORISE = 1, UNMEMORISE = 2, SPELLBAR_REFRESH = 3 };
			uint32 mSlot = 0;
			uint32 mSpellID = 0;
			Action mAction = SCRIBE;
			uint32 mUnknown0 = 0;
		};

		// C<->S
		struct DeleteSpell : public Fixed<DeleteSpell> {
			int16 mSpellBookSlot = 0;
			uint8 mUnknown0[2];
			uint8 mSuccess = 0;
			uint8 mUnknown1[3];
		};

		// C->S
		struct LoadSpellSet : public Fixed<LoadSpellSet> {
			uint32 mSpellIDs[Limits::Character::MAX_SPELLS_MEMED];	// 0xFFFFFFFF if no action, slot number if to unmem starting at 0
			uint32 mUnknown0 = 0;
		};

		// C->S
		struct SwapSpell : public Fixed<SwapSpell> {
			uint32 mFrom = 0;
			uint32 mTo = 0;
		};

		// C->S
		struct CastSpell : public Fixed<CastSpell> {
			uint32 mSlot = 0;
			uint32 mSpellID = 0;
			uint32 mInventorySlot = 0; // slot for clicky item, 0xFFFF = normal cast
			uint32 mTargetID; // SpawnID?
			uint8 mUnknown0[4];
		};

		// S->C
		// Based on: BeginCast_Struct
		struct BeginCast : public Fixed<BeginCast> {
			uint16 mSpawnID = 0; // Caster
			uint16 mSpellID = 0;
			uint32 mCastTime = 0; // MS
		};

		// S->C
		// Based on: InterruptCast_Struct
		struct InterruptCast : public Fixed<InterruptCast> {
			uint32 mSpawnID = 0;
			uint32 mMessageID = 0;
			char mMessage[0];
		};

		// S->C
		// Based on: ManaChange_Struct
		struct ManaChange : public Fixed<ManaChange> {
			uint32 mMana = 0;
			uint32 mEndurance = 0;
			uint32 mSpellID = 0;
			uint32 mUnknown0 = 0;
		};

		// S->C
		// Based on: Action_Struct
		struct Action : public Fixed<Action> {
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
		struct CombatAbility : public Fixed<CombatAbility> {
			uint32 mTargetID = 0;
			uint32 mAttack = 0;
			uint32 mSkill = 0;
		};

		// C->S
		struct Taunt : Fixed<Taunt> {
			uint32 mSpawnID = 0;
		};

		// C<->S
		// Yes the Client sends this for some magical reason.
		struct Consider : Fixed<Consider> {
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
		struct Surname : public Fixed<Surname> {
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			uint32 mApproved = 0;
			char mLastName[Limits::Character::MAX_LAST_NAME_LENGTH];
		};

		// S->C
		// Based on: GMLastName_Struct
		struct SurnameUpdate : public Fixed<SurnameUpdate> {
			char mCharaterName[Limits::Character::MAX_NAME_LENGTH];
			char mGMName[Limits::Character::MAX_NAME_LENGTH];
			char mLastName[Limits::Character::MAX_NAME_LENGTH];
			uint16 mUnknown0[4];
		};

		// C<->S
		struct SpawnAppearance : public FixedT<SpawnAppearance, OP_SpawnAppearance> {
			uint16 mSpawnID = 0;
			uint16 mType = 0;
			uint32 mParameter = 0;
		};

		// S->C
		struct Animation : public Fixed<Animation> {
			uint16 mSpawnID = 0;
			uint8 mAction = 0;
			uint8 mValue = 0;
		};

		// S->C
		// Based on: SkillUpdate_Struct
		struct SkillUpdate : public Fixed<SkillUpdate> {
			uint32 mID = 0;
			uint32 mValue = 0;
		};

		// S->C
		// Based on: Death_Struct
		struct Death : public Fixed<Death> {
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
		struct LootBeginRequest : public Fixed<LootBeginRequest> {
			uint32 mSpawnID = 0;
		};
		// C->S
		struct LootEndRequest : public Fixed<LootEndRequest> {
			uint32 mSpawnID = 0;
		};

		// S->C
		// Based on: moneyOnCorpseStruct
		struct LootResponse : public Fixed<LootResponse> {
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
		struct Target : public Fixed<Target> {
			uint32 mSpawnID = 0;
		};

		// S->C
		// Based on: RequestClientZoneChange_Struct
		struct RequestZoneChange : public Fixed<RequestZoneChange> {
			uint16 mZoneID = 0;
			uint16 mInstanceID = 0;
			float mY = 0.0f;
			float mX = 0.0f;
			float mZ = 0.0f;
			float mHeading = 0.0f;
			uint32 mType = 0; // NOTE: I tried various values for mType and found they did nothing I noticed.
		};

		// C<->S
		struct ZoneChange : public FixedT<ZoneChange, OP_ZoneChange> {
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			uint16 mZoneID = 0;
			uint16 mInstanceID = 0;
			float mY = 0.0f;
			float mX = 0.0f;
			float mZ = 0.0f;
			uint32 mReason = 0;
			int32 mSuccess = 0; // =0 client->server, =1 server->client, -X=specific error
		};

		// S->C
		// Based on: Weather_Struct
		struct Weather : public Fixed<Weather> {
			enum Type : uint32 { NORMAL = 0, SNOW = 2, RAIN = 3 };
			uint32 mUnknown0 = 0;
			uint32 mType = NORMAL;
			uint32 mIntensity = 0;
		};

		// C->S
		// Based on: MoveItem_Struct
		struct MoveItem : public Fixed<MoveItem> {
			uint32 mFromSlot = 0;
			uint32 mToSlot = 0;
			uint32 mStackSize = 0;
		};

		// C->S
		// Based on: Consume_Struct
		struct Consume : public Fixed<Consume> {
			enum ConsumeAction : uint32{ AUTO = 0xFFFFFF, MANUAL = 999 };
			enum ConsumeType : uint8 { FOOD = 1, DRINK = 2 };
			uint32 mSlot = 0;
			uint32 mConsumeAction = AUTO; // ConsumeAction
			uint8 c_unknown1[4];
			uint8 mType = 0; // ConsumeType
			uint8 unknown13[3];
		};

		// S->C
		// Based on: Stamina_Struct
		struct Stamina : public FixedT<Stamina, OP_Stamina> {
			uint32 mHunger = 0;
			uint32 mThirst = 0;
		};

		// C->S
		// Based on: ItemVerifyRequest_Struct
		struct ItemRightClick : public Fixed<ItemRightClick> {
			int32 mSlot = 0;
			uint32 mTargetSpawnID = 0;
		};

		// S->C
		struct ItemRightClickResponse : public FixedT<ItemRightClickResponse, OP_ItemVerifyReply> {
			int32 mSlot = 0;
			uint32 mSpellID = 0;
			uint32 mTargetSpawnID = 0;
		};

		// C->S
		struct OpenContainer : public Fixed<OpenContainer> {
			uint32 mSlot = 0;
		};

		// C<->S
		struct TradeRequest : public FixedT<TradeRequest, OP_TradeRequest> {
			uint32 mToSpawnID = 0;
			uint32 mFromSpawnID = 0;
		};

		// C->S
		struct TradeCancel : public FixedT<TradeCancel, OP_CancelTrade> {
			uint32 mFromSpawnID = 0;
			uint32 mAction = 0;
		};

		// C->S
		struct TradeAccept : public Fixed<TradeAccept> {
			uint32 mFromSpawnID = 0;
			uint32 mUnknown0 = 0;
		};

		// C<->S
		struct TradeBusy : public Fixed<TradeBusy> {
			uint32 mToSpawnID = 0;
			uint32 mFromSpawnID = 0;
			uint8 mType = 0;
			uint8 mUnknown0 = 0;
			uint8 mUnknown1 = 0;
			uint8 mUnknown2 = 0;
		};

		// C->S
		struct ServerFilter : public Fixed<ServerFilter> {
			Filters mFilters;
		};

		// C->S
		struct MoveCoin : public Fixed<MoveCoin> {
			uint32 mFromSlot = 0;
			uint32 mToSlot = 0;
			uint32 mTypeOne = 0;
			uint32 mTypeTwo = 0;
			int32 mAmount = 0;
			String _debug() const {
				StringStream ss;
				ss << "[MoveCoin] ";
				PRINT_MEMBER(mFromSlot);
				PRINT_MEMBER(mToSlot);
				PRINT_MEMBER(mTypeOne);
				PRINT_MEMBER(mTypeTwo);
				PRINT_MEMBER(mAmount);
				return ss.str();
			}
		};

		// C<-S
		struct MoneyUpdate : public FixedT<MoneyUpdate, OP_MoneyUpdate> {
			int32 mPlatinum = 0;
			int32 mGold = 0;
			int32 mSilver = 0;
			int32 mCopper = 0;
		};

		// C->S
		struct CrystalCreate : public Fixed<CrystalCreate>  {
			enum Type : uint32 { RADIANT = 4, EBON = 5 };
			uint32 mType = 0;
			uint32 mAmount = 0;
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
			uint32 mRadiantCrystals = 0;
			uint32 mEbonCrystals = 0;
			uint32 mTotalRadiantCrystals = 0;
			uint32 mTotalEbonCrystals = 0;
		};
	}

	namespace World {

		// C->S
		struct LoginInformation : public Fixed<LoginInformation> {
			char mInformation[64]; // Account ID and Account Key
			uint8 mUnknown0[124];
			uint8 mZoning;
			uint8 mUnknown1[275];
		};

		// C->S
		struct EnterWorld : public Fixed<EnterWorld> {
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

		struct CreateCharacter : public Fixed<CreateCharacter> {
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

		// S->C
		// Based on: LogServer_Struct
		struct LogServer : public Fixed<LogServer> {
			uint32 mUknown0 = 0;
			uint8 mPVPEnabled = 0;
			uint8 mUnknown1 = 0;
			uint8 mUnknown2 = 0;
			uint8 mUnknown3 = 0;
			uint8 mFVEnabled = 0;
			uint8 mUnknown4 = 0;
			uint8 mUnknown5 = 0;
			uint8 mUnknown6 = 0;
			uint32 mUnknown7 = 0;
			uint32 mUnknown8 = 0;
			uint8 mUnknown9 = 0;
			char mWorldShortName[32];
			uint8 mUnknown10[32];
			char mUnknown11[16];
			char mUnknown12[16];
			uint8 mUnknown13[48];
			uint32 mUnknown14;
			char mUnknown15[80];
			uint8 mEnablePetitionWindow = 0;
			uint8 mEnabledVoiceMacros = 0;
			uint8 mEnableEmail = 0;
			uint8 mEnableTutorial = 0;
		};

		// S->C
		// Based on: ExpansionInfo_Struct
		struct ExpansionInfo : public Fixed<ExpansionInfo> {
			uint32 mExpansions = 0;
		};

		// S->C
		// Based on: CharacterSelect_Struct
		struct CharacterSelect : public Fixed<CharacterSelect> {
			uint32 mRaces[18];
			Colour mEquipmentColours[18][9];
			uint8 mBeardColours[18];
			uint8 mHairStyles[18];
			uint32 mEquipment[18][9];			// 0=helm, 1=chest, 2=arm, 3=bracer, 4=hand, 5=leg, 6=boot, 7=melee1, 8=melee2 (Might not be)
			uint32 mSecondaryItems[18];
			uint32 mDrakkinHeritages[18];
			uint32 mDrakkinTattoos[18];
			uint32 mDrakkinDetails[18];
			uint32 mDeities[18];
			uint8 mGoHomeAvailable[18];
			uint8 mTutorialAvailable[18];
			uint8 mBeardStyles[18];
			uint8	unknown902[18];			// 10x ff
			uint32 mPrimaryItems[18]; // ID File
			uint8 mHairColours[18];
			uint8	unknown0962[2];			// 2x 00
			uint32 mZoneIDs[18];
			uint8 mClasses[18];
			uint8 mFaceStyles[18];
			char mNames[18][64];
			uint8 mGenders[18];
			uint8	eyecolor1[18];			// Characters Eye Color
			uint8	eyecolor2[18];			// Characters Eye 2 Color
			uint8 mLevels[18];
		};

		// S->C
		// Based on: NameGeneration_Struct
		struct NameGeneration : public Fixed<NameGeneration> {
			uint32 mRace = 0;
			uint32 mGender = 0;
			char mName[64];
		};

		// S->C
		// Based on: ZoneServerInfo_Struct
		struct ZoneServerInfo : public Fixed<ZoneServerInfo> {
			char mIP[128];
			uint16 mPort = 0;
		};

		// S->C
		// Based on: ZoneUnavail_Struct
		struct ZoneUnavailable : public Fixed<ZoneUnavailable> {
			char mZoneName[16];
			int16 mUnknown0[4];
		};

		// S->C
		// Based on: ApproveWorld_Struct
		struct ApproveWorld : public Fixed<ApproveWorld> {
			uint8 mUnknown0[544];
		};
	}

	namespace LoginServer {

		struct ConnectRequest : public Fixed<ConnectRequest> {
			uint32 mAccountID = 0;
			uint32 mWorldID = 0;
			uint32 mFromID = 0;
			uint32 mToID = 0;
		};

		struct ConnectResponse : public Fixed<ConnectResponse> {
			uint32 mAccountID = 0;
			uint32 mWorldID = 0;
			ResponseID mResponse = ResponseID::ALLOWED;
			uint32 mFromID = 0;
			uint32 mToID = 0;
		};

		struct ClientAuthentication : public Fixed<ClientAuthentication> {
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

		struct MakeLeader : public Fixed<MakeLeader> {
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			char mLeaderName[Limits::Character::MAX_NAME_LENGTH];
		};

		struct Demote : public Fixed<Demote> {
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // Character doing the demoting
			char mDemoteName[Limits::Character::MAX_NAME_LENGTH]; // Character being demoted.
		};

		// Used for changing both banker and alt status of a guild member.
		struct BankerAltStatus : public Fixed<BankerAltStatus> {
			uint32 mUnknown = 0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // NOTE: UF does not send this
			char mOtherName[Limits::Character::MAX_NAME_LENGTH]; // Character whose status is being changed.
			uint32 mStatus = 0;
		};

		// S->C
		struct Remove : public Fixed<Remove> {
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