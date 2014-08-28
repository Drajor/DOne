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

	namespace Zone {

		struct ZoneEntry : public FixedSizedPayload<ZoneEntry> {
			uint32 mUnknown = 0;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
		};

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
	}

	namespace World {

		struct LoginInformation : public FixedSizedPayload<LoginInformation> {
			char mInformation[64]; // Account ID and Account Key
			uint8 mUnknown0[124];
			uint8 mZoning;
			uint8 mUnknown1[275];
		};

		struct EnterWorld : public FixedSizedPayload<EnterWorld> {
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			uint32 mTutorial = 0;
			uint32 mReturnHome = 0;
		};

		struct DeleteCharacter : VariableLengthPayload<DeleteCharacter> {
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
			uint32 mFace = 0;
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

		struct MakeLeader{
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH];
			char mLeaderName[Limits::Character::MAX_NAME_LENGTH];

			inline static MakeLeader* convert(unsigned char* pData) { return reinterpret_cast<MakeLeader*>(pData); }
			inline static const bool sizeCheck(const std::size_t pSize) { return pSize == sizeof(MakeLeader); }
		};

		struct Demote {
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // Character doing the demoting
			char mDemoteName[Limits::Character::MAX_NAME_LENGTH]; // Character being demoted.
			
			inline static Demote* convert(unsigned char* pData) { return reinterpret_cast<Demote*>(pData); }
			inline static const bool sizeCheck(const std::size_t pSize) { return pSize == sizeof(Demote); }
		};

		// Used for changing both banker and alt status of a guild member.
		struct BankerAltStatus {
			uint32 mUnknown;
			char mCharacterName[Limits::Character::MAX_NAME_LENGTH]; // NOTE: UF does not send this
			char mOtherName[Limits::Character::MAX_NAME_LENGTH]; // Character whose status is being changed.
			uint32 mStatus;

			inline static BankerAltStatus* convert(unsigned char* pData) { return reinterpret_cast<BankerAltStatus*>(pData); }
			inline static const bool sizeCheck(const std::size_t pSize) { return pSize == sizeof(BankerAltStatus); }
		};

		struct Remove {
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
		uint8 mHairColor = 0;
		uint8 mCurrentHP = 100;
		uint8 max_hp = 100;				// (name prolly wrong)takes on the value 100 for players, 100 or 110 for NPCs and 120 for PC corpses...
		uint8 mIsFindable = 0;
		uint8 mUnknown5[5]; // TODO: I suspect this can be set as spawn ID and the memory used for position updates.
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