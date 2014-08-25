#pragma once

#include "Constants.h"

enum GuildUpdateAction : uint32 {
	GUILD_URL = 0,
	GUILD_CHANNEL = 1
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
}
#pragma pack()