#pragma once

#include "Constants.h"
#include "Utility.h"
#include "Vector3.h"
#include "../common/EQPacket.h"

enum GuildUpdateAction : u32 {
	GUILD_URL = 0,
	GUILD_CHANNEL = 1
};

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

		// C->S
		struct Time : public FixedT<Time, OP_TimeOfDay> {
			u8 mHour = 0;
			u8 mMinute = 0;
			u8 mDay = 0;
			u8 mMonth = 0;
			u32 mYear = 0;
		};

		// Note: ZoneEntry is C->S fixed and S->C variable.
		struct ZoneEntry : public Fixed<ZoneEntry> {
			u32 mUnknown = 0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
		};

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
		struct SetTitle : public Fixed<SetTitle> {
			enum : u32 { SET_TITLE = 0, SET_SUFFIX = 1 };
			u32 mOption = SET_TITLE;
			u32 mTitleID = 0;
		};

		// S->C
		namespace TitleUpdateLimits { static const auto MAX_TEXT = 32; }
		struct TitleUpdate : public FixedT<TitleUpdate, OP_SetTitleReply> {
			static EQApplicationPacket* construct(const u32 pOption, const u32 pSpawnID, const String& pText) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mOption = pOption;
				payload->mSpawnID = pSpawnID;
				strcpy(payload->mText, pText.c_str());

				return packet;
			}
			enum : u32 { UPDATE_TITLE = 0, UPDATE_SUFFIX = 1 };
			u32 mOption = UPDATE_TITLE;
			char mText[TitleUpdateLimits::MAX_TEXT];
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
			u16 mSpawnID = 0;
			u32 mMaterialID = 0;
			u32 mUnused0 = 0;
			u32 mEliteMaterialID = 0; // Not sure what this is.
			u32 mColour = 0;
			u8 mSlot = 0;
		};
		
		// C->S
		struct AutoAttack : public Fixed<AutoAttack> {
			bool mAttacking = false;
			char mUnknown[3];
		};

		// C<->S
		struct MemoriseSpell : public Fixed<MemoriseSpell> {
			enum Action : u32 { SCRIBE = 0, MEMORISE = 1, UNMEMORISE = 2, SPELLBAR_REFRESH = 3 };
			u32 mSlot = 0;
			u32 mSpellID = 0;
			Action mAction = SCRIBE;
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
			u32 mSpellIDs[Limits::Character::MAX_SPELLS_MEMED];	// 0xFFFFFFFF if no action, slot number if to unmem starting at 0
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
		// Based on: CombatAbility_Struct
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
			static EQApplicationPacket* construct(const u32 pApproved, const String& pName, const String& pLastName) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mApproved = pApproved;
				strcpy(payload->mCharacterName, pName.c_str());
				strcpy(payload->mLastName, pLastName.c_str());

				return packet;
			}
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			u32 mApproved = 0;
			char mLastName[Limits::Character::MAX_LAST_NAME_LENGTH];
		};

		// S->C
		// Based on: GMLastName_Struct
		struct SurnameUpdate : public Fixed<SurnameUpdate> {
			char mCharaterName[Limits::Character::MAX_NAME_LENGTH];
			char mGMName[Limits::Character::MAX_NAME_LENGTH];
			char mLastName[Limits::Character::MAX_NAME_LENGTH];
			u16 mUnknown0[4];
		};

		// C<->S
		struct SpawnAppearance : public FixedT<SpawnAppearance, OP_SpawnAppearance> {
			u16 mSpawnID = 0;
			u16 mType = 0;
			u32 mParameter = 0;
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
		// Based on: SkillUpdate_Struct
		struct SkillUpdate : public Fixed<SkillUpdate> {
			u32 mID = 0;
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
		// Based on: moneyOnCorpseStruct
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
		// Based on: ClientTarget_Struct
		struct Target : public Fixed<Target> {
			u32 mSpawnID = 0;
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

		// C->S
		// Based on: Consume_Struct
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
		// Based on: ItemVerifyRequest_Struct
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
			static EQApplicationPacket* construct(const u32 pToSpawnID, const u32 pFromSpawnID) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mToSpawnID = pToSpawnID;
				payload->mFromSpawnID = pFromSpawnID;

				return packet;
			}
			u32 mToSpawnID = 0;
			u32 mFromSpawnID = 0;
		};

		// C->S
		struct TradeCancel : public FixedT<TradeCancel, OP_CancelTrade> {
			static EQApplicationPacket* construct(const u32 pToSpawnID, const u32 pFromSpawnID) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mToSpawnID = pToSpawnID;
				payload->mFromSpawnID = pFromSpawnID;

				return packet;
			}
			u32 mToSpawnID = 0;
			u32 mFromSpawnID = 0;
			String _debug() const {
				StringStream ss;
				ss << "[TradeCancel] ";
				PRINT_MEMBER(mToSpawnID);
				PRINT_MEMBER(mFromSpawnID);
				return ss.str();
			}
		};

		// C->S
		struct TradeAccept : public Fixed<TradeAccept> {
			u32 mFromSpawnID = 0;
			u32 mUnknown0 = 0;
			String _debug() const {
				StringStream ss;
				ss << "[TradeAccept] ";
				PRINT_MEMBER(mFromSpawnID);
				PRINT_MEMBER(mUnknown0);
				return ss.str();
			}
		};

		// C<->S
		struct TradeBusy : public Fixed<TradeBusy> {
			u32 mToSpawnID = 0;
			u32 mFromSpawnID = 0;
			u8 mType = 0;
			u8 mUnknown0 = 0;
			u8 mUnknown1 = 0;
			u8 mUnknown2 = 0;
		};

		// C->S
		struct ServerFilter : public Fixed<ServerFilter> {
			Filters mFilters;
		};

		// C->S
		struct MoveCoin : public Fixed<MoveCoin> {
			u32 mFromSlot = 0;
			u32 mToSlot = 0;
			u32 mFromType = 0;
			u32 mToType = 0;
			i32 mAmount = 0;
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
		struct CrystalCreate : public Fixed<CrystalCreate>  {
			enum Type : u32 { RADIANT = 4, EBON = 5 };
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
			u8 mShowButtons = 0;
			u8 __Unknown0 = 0;
			u32 mDuration = 0;
			u32 mLeftButtonResponse = 1;
			u32 mRightButtonResponse = 2;
			u32 __Unknown1 = 0;
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
			u32 mSlot = 0;
			String _debug() const {
				StringStream ss;
				ss << "{Combine} ";
				PRINT_MEMBER(mSlot);
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
		struct LevelUpdate : public FixedT<LevelUpdate, OP_LevelUpdate> {
			static EQApplicationPacket* construct(const u32 pLevel, const u32 pPreviousLevel, const u32 pExperience) {
				auto packet = create();
				auto payload = convert(packet);
				payload->mLevel = pLevel;
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
			u32 mUnknown = 0;
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
				return pSize >= Limits::Character::MIN_NAME_LENGTH && pSize <= Limits::Character::MAX_NAME_LENGTH;
			}
		};

		struct CreateCharacter : public Fixed<CreateCharacter> {
			u32 mClass = 0;
			u32 mHairColour = 0;
			u32 mBeardColour = 0;
			u32 mBeardStyle = 0;
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

		// S->C
		// Based on: CharacterSelect_Struct
		struct CharacterSelect : public Fixed<CharacterSelect> {
			u32 mRaces[18];
			Colour mEquipmentColours[18][9];
			u8 mBeardColours[18];
			u8 mHairStyles[18];
			u32 mEquipment[18][9];			// 0=helm, 1=chest, 2=arm, 3=bracer, 4=hand, 5=leg, 6=boot, 7=melee1, 8=melee2 (Might not be)
			u32 mSecondaryItems[18];
			u32 mDrakkinHeritages[18];
			u32 mDrakkinTattoos[18];
			u32 mDrakkinDetails[18];
			u32 mDeities[18];
			u8 mGoHomeAvailable[18];
			u8 mTutorialAvailable[18];
			u8 mBeardStyles[18];
			u8	unknown902[18];			// 10x ff
			u32 mPrimaryItems[18]; // ID File
			u8 mHairColours[18];
			u8	unknown0962[2];			// 2x 00
			u32 mZoneIDs[18];
			u8 mClasses[18];
			u8 mFaceStyles[18];
			char mNames[18][64];
			u8 mGenders[18];
			u8	eyecolor1[18];			// Characters Eye Color
			u8	eyecolor2[18];			// Characters Eye 2 Color
			u8 mLevels[18];
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
	}

	namespace LoginServer {
		namespace OpCode {
			enum : u16 {
				WorldInformation = 0x1008,
				WorldStatus = 0x1001,
				ConnectRequest = 0xAB00,
				ConnectResponse = 0xAB01,
				ClientAuthentication = 0x1002,
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
			u8 mResponse = ResponseID::ALLOWED;
			u32 mFromID = 0; // Ignored in LS.
			u32 mToID = 0; // Ignored in LS.
		};

		// LS->W
		namespace ClientAuthenticationLimits {
			static const auto MAX_ACCOUNT = 30;
			static const auto MAX_KEY = 30;
		}
		struct ClientAuthentication : public Fixed<ClientAuthentication> {
			u32 mAccountID = 0;
			char mAccountName[ClientAuthenticationLimits::MAX_ACCOUNT];
			char mKey[ClientAuthenticationLimits::MAX_KEY];
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
			u32 mUnknown = 0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // NOTE: UF does not send this
			char mOtherName[Limits::Character::MAX_NAME_LENGTH]; // Character whose status is being changed.
			u32 mStatus = 0;
		};

		// S->C
		struct Remove : public Fixed<Remove> {
			GuildID mGuildID;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
		};
		
		struct LevelUpdate {
			GuildID mGuildID; // why?!
			char mMemberName[Limits::Character::MAX_NAME_LENGTH];
			u32 mLevel;
		};

		struct MOTD {
			u32 mUnknown0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // Receiver name.. why.
			char mSetByName[Limits::Character::MAX_NAME_LENGTH];
			u32 mUnknown1;
			char mMOTD[Limits::Guild::MAX_MOTD_LENGTH];
		};

		struct GuildUpdate {
			enum Action : u32 { GUILD_URL = 0, GUILD_CHANNEL = 1 };
			Action mAction;
			char mUnknown0[76];
			char mText[512];
			char mUnknown1[3584];
		};

		struct PublicNote {
			u32 mUnknown;
			char mSenderName[Limits::Character::MAX_NAME_LENGTH];
			char mTargetName[Limits::Character::MAX_NAME_LENGTH];
			char mNote[1]; // NOTE: I believe this gets cut off to length 100 by underlying code.
		};

		struct MemberUpdate {
			GuildID mGuildID;
			char mMemberName[Limits::Character::MAX_NAME_LENGTH];
			u32 mZoneID;
			u16 mInstanceID;
			u32 mLastSeen;
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
			memset(mLastName, 0, sizeof(mLastName));
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
		u32 mGuildID = NO_GUILD;
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

		char mLastName[100];

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