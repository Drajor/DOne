#pragma once

#include "Constants.h"
#include "Utility.h"
#include "Vector3.h"
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
		struct SetTitle : public Fixed<SetTitle> {
			enum : uint32 { SET_TITLE = 0, SET_SUFFIX = 1 };
			uint32 mOption = SET_TITLE;
			uint32 mTitleID = 0;
		};

		// S->C
		struct TitleUpdate : public FixedT<TitleUpdate, OP_SetTitleReply> {
			static EQApplicationPacket* construct(const uint32 pOption, const uint32 pSpawnID, const String& pText) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mOption = pOption;
				payload->mSpawnID = pSpawnID;
				strcpy(payload->mText, pText.c_str());

				return packet;
			}
			enum : uint32 { UPDATE_TITLE = 0, UPDATE_SUFFIX = 1 };
			uint32 mOption = UPDATE_TITLE;
			char mText[32];
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
		struct Consider : FixedT<Consider, OP_Consider> {
			enum Message { Ally = 1, Warmly = 2, Kindly = 3, Amiably = 4, Indifferent = 5, Scowls = 6, Threateningly = 7, Dubiously = 8, Apprehensively = 9  };
			static EQApplicationPacket* construct(const uint32 pSpawnID, const uint32 pTargetSpawnID, const uint32 pMessage) {
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
				PRINT_MEMBER((int32)mIsPVP);
				PRINT_MEMBER((int32)mUnknown0);
				PRINT_MEMBER((int32)mUnknown1);
				PRINT_MEMBER((int32)mUnknown2);
				return ss.str();
			}
			uint32 mSpawnID = 0;
			uint32 mTargetSpawnID = 0;
			uint32 mMessage = Message::Indifferent;
			uint32 mTargetLevel = 0;
			uint8 mIsPVP = 0;
			uint8 mUnknown0 = 0;
			uint8 mUnknown1 = 0;
			uint8 mUnknown2 = 0;
		};

		// C<->S
		struct Surname : public FixedT<Surname, OP_Surname> {
			static EQApplicationPacket* construct(const uint32 pApproved, const String& pName, const String& pLastName) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mApproved = pApproved;
				strcpy(payload->mCharacterName, pName.c_str());
				strcpy(payload->mLastName, pLastName.c_str());

				return packet;
			}
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
		struct ActorAnimation : public FixedT<ActorAnimation, OP_Animation> {
			static EQApplicationPacket* construct(const uint16 pSpawnID, const uint8 pAnimation, const uint8 pSpeed = 10) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mSpeed = pSpeed;
				payload->mAnimation = pAnimation;

				return packet;
			}
			uint16 mSpawnID = 0;
			uint8 mSpeed = 0;
			uint8 mAnimation = 0;
		};

		// S->C
		// Based on: SkillUpdate_Struct
		struct SkillUpdate : public Fixed<SkillUpdate> {
			uint32 mID = 0;
			uint32 mValue = 0;
		};

		// S->C
		struct Death : public FixedT<Death, OP_Death> {
			static EQApplicationPacket* construct(const uint32 pSpawnID, const uint32 pKillerSpawnID, const uint32 pDamage, const uint32 pSkill) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mKillerSpawnID = pKillerSpawnID;
				payload->mDamage = pDamage;
				payload->mSkillID = pSkill;

				return packet;
			}
			uint32 mSpawnID = 0;
			uint32 mKillerSpawnID = 0;
			uint32 mCorpseID = 0; // ??
			uint32 mBindZoneID = 0;
			uint32 mSpellID = 0xFFFFFFFF;
			uint32 mSkillID = 0;
			uint32 mDamage = 0;
			uint32 mUnknown0 = 0;
		};

		// S->C
		struct Damage : public FixedT<Damage, OP_Damage> {
			Damage() { memset(__Unknown1, 0, sizeof(__Unknown1)); }
			static EQApplicationPacket* construct(const uint16 pTarget, const uint16 pSource, const uint32 pAmount, const uint8 pType, const uint32 pSequence, const uint16 pSpellID = 0) {
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
			uint16 mTargetSpawnID = 0;
			uint16 mSourceSpawnID = 0;
			uint8 mType = 0; // See Constants, DamageType
			uint16 mSpellID = 0;
			int32 mAmount = 0;
			float __Unknown0 = 0.0f;
			uint32 mSequence = 0;
			uint8 __Unknown1[9];
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

		struct LootItem : Fixed<LootItem> {
			uint32 mCorpseSpawnID = 0;
			uint32 mLooterSpawnID = 0;
			uint32 mSlotID = 0;
			uint32 mAutoLoot = 0;
			uint32 mUnknown0 = 0; // Right0, 
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
		// Based on: ClientTarget_Struct
		struct Target : public Fixed<Target> {
			uint32 mSpawnID = 0;
		};

		// S->C
		struct RequestZoneChange : public FixedT<RequestZoneChange, OP_RequestClientZoneChange> {
			static EQApplicationPacket* construct(const uint16 pZoneID, const uint16 pInstanceID, const Vector3& pPosition) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mZoneID = pZoneID;
				payload->mInstanceID = pInstanceID;
				payload->mX = pPosition.x;
				payload->mY = pPosition.y;
				payload->mZ = pPosition.z;

				return packet;
			}
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
			ZoneChange() { memset(mCharacterName, 0, sizeof(mCharacterName)); }
			static EQApplicationPacket* construct(const String& pCharacterName, const uint16 pZoneID, const uint16 pInstanceID, const Vector3& pPosition, const int32 pSuccess) {
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
			uint32 mFromType = 0;
			uint32 mToType = 0;
			int32 mAmount = 0;
			String _debug() const {
				StringStream ss;
				ss << "[MoveCoin] ";
				PRINT_MEMBER(mFromSlot);
				PRINT_MEMBER(mToSlot);
				PRINT_MEMBER(mFromType);
				PRINT_MEMBER(mToType);
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

		// S->C
		struct Illusion : public FixedT<Illusion, OP_Illusion> {

		};

		// S->C
		struct HPUpdate : public FixedT<HPUpdate, OP_HPUpdate> {
			static EQApplicationPacket* construct(const int16 pSpawnID, const int32 pCurrentHP, const int32 pMaximumHP) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mCurrentHP = pCurrentHP;
				payload->mMaximumHP = pMaximumHP;
				payload->mSpawnID = pSpawnID;

				return packet;
			}
			int32 mCurrentHP = 0;
			int32 mMaximumHP = 0;
			int16 mSpawnID = 0;
		};

		// S->C
		struct ActorHPUpdate : public FixedT<ActorHPUpdate, OP_MobHealth> {
			static EQApplicationPacket* construct(const uint16 pSpawnID, const uint8 pHPPercent) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mHPPercent = pHPPercent;

				return packet;
			}
			int16 mSpawnID = 0;
			uint8 mHPPercent = 0;
		};

		// C->S
		struct MarkNPC : public FixedT<MarkNPC, OP_MarkNPC> {
			MarkNPC() { memset(mName, 0, sizeof(mName)); }
			static EQApplicationPacket* construct(const uint32 pSpawnID, const uint32 pNumber, const String& pName) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mSpawnID = pSpawnID;
				payload->mNumber = pNumber;
				strcpy(payload->mName, pName.c_str());

				return packet;
			}
			uint32 mSpawnID = 0;
			uint32 mNumber = 1;
			char mName[64];
		};

		// C->S
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
			uint8 mShowButtons = 0;
			uint8 __Unknown0 = 0;
			uint32 mDuration = 0;
			uint32 mLeftButtonResponse = 1;
			uint32 mRightButtonResponse = 2;
			uint32 __Unknown1 = 0;
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

	// C->S
	struct PositionUpdate : public Fixed<PositionUpdate> {
		uint16 mSpawnID = 0;
		uint16 mSequence = 0;
		/*0004*/ uint8		unknown0004[4];		// ***Placeholder
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
		uint16 mSpawnID = 0;
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
			memset(mLastName, 0, sizeof(mLastName));
			memset(mTitle, 0, sizeof(mTitle));
			memset(mSuffix, 0, sizeof(mSuffix));
			memset(mUnknowns, 0, sizeof(mUnknowns));
			memset(&mFlags, 0, sizeof(mFlags));
			memset(&mPosition, 0, sizeof(mPosition));

			mFlags.mTargetable = 1;
			mFlags.mTargetableWithHotkey = 1;
			mFlags.mShowName = 1;
		}
		char mName[100]; // Variable
		uint32 mSpawnID = 0;
		uint8 mLevel = 1;
		// Destructible Object writes 4 bytes, float = 10.
		// Else writes 4 bytes, float, SpawnSize - 0.7 comment = eye height?
		float mViewOffset = 20.0f;
		uint8 mActorType = 0;
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
			unsigned mShowHelm : 1;
			unsigned padding26 : 1;
			unsigned mTargetable : 1;
			unsigned mTargetableWithHotkey : 1;
			unsigned mShowName : 1;
			unsigned statue : 1; // No observed effect on NPCs. Added <Unknown Guild> below PC name.
			unsigned mIsTrader : 1; // No observed effect on NPCs. Added "Trader" to PC name.
			unsigned mIsBuyer : 1; // No observed effect on NPCs. Added "Buyer" to PC name + <Unknown Guild> + removed title.
		};
		ActorFlags mFlags;
		uint8 mOtherFlags = 0;
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

		uint32 mAppearance = 0;
		uint32 mUnknown0 = 0;

		uint32 mID0 = 0;
		uint32 mID1 = 0;
		uint32 mID2 = 0;
		uint32 mID3 = 0;

		uint32 mUnknown1 = 0;
		uint32 mUnknown2 = 0;
		uint32 mUnknown3 = 0;
		uint32 mUnknown4 = 0;
		uint32 mUnknown5 = 0;
		uint32 mUnknown6 = 0;
		uint8 mUnknown7
		uint32 mUnknown8 = 0;
		};
		Destructible mDestructible;
		*/

		float mSize = 0.0f;
		uint8 mFaceStyle = 0;
		float mWalkSpeed = 3.0f;
		float mRunSpeed = 6.0f;
		uint32 mRace = 0;
		uint8 mPropertyCount = 1; // HC 1
		uint32 mBodyType = 0;
		uint8 mHPPercent = 100; // Percentage: 0-100
		uint8 mHairColour = 0;
		uint8 mBeardColour = 0;
		uint8 mLeftEyeColour = 0;
		uint8 mRightEyeColour = 0;
		uint8 mHairStyle = 0;
		uint8 mBeardStyle = 0;
		uint32 mDrakkinHeritage = 0;
		uint32 mDrakkinTattoo = 0;
		uint32 mDrakkinDetails = 0;
		uint8 __Unknown3 = 0;
		uint32 mDeity = 0;
		uint32 mGuildID = NO_GUILD;
		uint32 mGuildRank = 0;
		uint8 mClass = 0;
		uint8 mPVP = 0;
		uint8 mStandState = 100;
		uint8 mLight = 0;
		uint8 mFlyMode = 0;
		uint8 __Unknown4 = 0; // equip_chest2
		uint8 __Unknown5 = 0;
		uint8 __Unknown6 = 0;
		uint8 mShowHelm = 0;

		char mLastName[100];

		uint32 mAAtitle = 0; // TODO: Figure out how this works.
		uint8 __Unknown8 = 0;
		uint32 mOwnerSpawnID = 0; // Pet Owner.
		uint8 __Unknown9 = 0;
		uint32 __Unknown10 = 0; // - Stance 64 = normal 4 = aggressive 40 = stun/mezzed
		uint32 __Unknown11 = 0;
		uint32 __Unknown12 = 0;
		uint32 __Unknown13 = 0;
		uint32 __Unknown14 = 0xFFFFFFFF; // Copied HC
		uint16 __Unknown15 = 0xFFFF; // Copied HC

		ActorPosition mPosition;

		uint32 mColours[9];

		struct Equipment {
			uint32 mEquip0 = 0;
			uint32 mEquip1 = 0;
			uint32 mItemID = 0;
		};
		Equipment mEquipment[9];

		char mTitle[100]; // Variable
		char mSuffix[100]; // Variable

		uint32 __Unknown16 = 0;
		uint32 __Unknown17 = 0;
		uint8 mIsMercenary = 0;

		char mUnknowns[28];
	};
}
#pragma pack()