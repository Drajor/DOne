#pragma once

#include "Constants.h"
#include "ClientAuthentication.h"
#include "../common/timer.h"

class EQStreamFactory;
class EQStreamIdentifier;
class DataStore;
class ZoneClientConnection;

class World;
class ZoneManager;
class Character;
class Group;
class GroupManager;
class Guild;
class Raid;
class RaidManager;

class Zone {
public:
	Zone(uint32 pPort, ZoneID pZoneID, InstanceID pInstanceID = 0);
	~Zone();

	bool initialise();
	String getLongName() { return mLongName; }
	String getShortName() { return mShortName; }
	uint32 getLongNameStringID() { return mLongNameStringID; }
	float getGravity() { return 0.4f; }
	float getMinimumZ() { return -5000.0f; } // NOTE: The lowest point in the zone a Character should be able to reach.
	std::uint8_t getZoneType() { return 255; } // Unknown.

	std::uint32_t getNumCharacters() { return mCharacters.size(); }

	void addAuthentication(ClientAuthentication& pAuthentication, String pCharacterName);
	void removeAuthentication(String pCharacterName);
	bool checkAuthentication(String pCharacterName);
	bool getAuthentication(String pCharacterName, ClientAuthentication& pAuthentication);

	void shutdown();
	void update();

	void _updateCharacters();

	void _updateConnections();

	void _updatePreConnections();

	ZoneID getID() { return mID; }
	InstanceID getInstanceID() { return mInstanceID; }
	uint16 getPort() { return mPort; }
	Character* findCharacter(const String pCharacterName);

	void notifyCharacterZoneIn(Character* pCharacter);
	void notifyCharacterZoneOut(Character* pCharacter);
	void notifyCharacterPositionChanged(Character* pCharacter);
	void notifyCharacterLinkDead(Character* pCharacter);
	void notifyCharacterAFK(Character* pCharacter);
	void notifyCharacterShowHelm(Character* pCharacter);
	void notifyCharacterAnonymous(Character* pCharacter);
	void notifyCharacterStanding(Character* pCharacter);
	void notifyCharacterSitting(Character* pCharacter);
	void notifyCharacterCrouching(Character* pCharacter);
	void notifyCharacterChatSay(Character* pCharacter, const String pMessage);
	void notifyCharacterChatShout(Character* pCharacter, const String pMessage);
	void notifyCharacterChatOOC(Character* pCharacter, const String pMessage);
	void notifyCharacterChatGroup(Character* pCharacter, const String pMessage);
	void notifyCharacterEmote(Character* pCharacter, const String pMessage);
	void notifyCharacterChatAuction(Character* pCharacter, const String pMessage);
	void notifyCharacterChatTell(Character* pCharacter, const String& pTargetName, const String& pMessage);
	void notifyCharacterAnimation(Character* pCharacter, uint8 pAction, uint8 pAnimationID, bool pIncludeSender = false);
	void notifyCharacterLevelIncrease(Character* pCharacter);
	void notifyCharacterLevelDecrease(Character* pCharacter);
	void notifyCharacterGM(Character* pCharacter);

	// Group
	void notifyCharacterGroupInvite(Character* pCharacter, const String pToCharacterName);
	void notifyCharacterAcceptGroupInvite(Character* pCharacter, String pToCharacterName);
	void notifyCharacterDeclineGroupInvite(Character* pCharacter, String pToCharacterName);
	void notifyCharacterGroupDisband(Character* pCharacter, const String& pRemoveCharacterName);
	void notifyCharacterMakeLeaderRequest(Character* pCharacter, String pNewLeaderName);

	// Guild
	void notifyGuildsChanged();
	void notifyCharacterGuildChange(Character* pCharacter);

	void notifyCharacterZoneChange(Character* pCharacter, ZoneID pZoneID, uint16 pInstanceID);
	Character* getZoningCharacter(String pCharacterName);

	void moveCharacter(Character* pCharacter, float pX, float pY, float pZ);
	uint16 getNextSpawnID() { return mNextSpawnID++; }

	bool trySendTell(const String& pSenderName, const String& pTargetName, const String& pMessage);
	void processCharacterQueuedTells(Character* );
	void whoRequest(Character* pCharacter, WhoFilter& pFilter);
	void getWhoMatches(std::list<Character*>& pMatches, WhoFilter& pFilter);
	void requestSave(Character* pCharacter);

	
	
private:

	// Performs a global Character search.
	Character* _findCharacter(const String& pCharacterName, bool pIncludeZoning = false);

	void _sendDespawn(uint16 pSpawnID, bool pDecay = false);
	void _sendChat(Character* pCharacter, ChannelID pChannel, const String pMessage);
	void _sendSpawnAppearance(Character* pCharacter, SpawnAppearanceType pType, uint32 pParameter, bool pIncludeSender = false);
	void _sendLevelAppearance(Character* pCharacter);
	void _handleIncomingConnections();
	void _sendCharacterLevel(Character* pCharacter);
	void _handleWhoRequest(Character* pCharacter, WhoFilter& pFilter);
	void _handleCharacterLinkDead(Character* pCharacter);
	







	std::map<String, ClientAuthentication> mAuthenticatedCharacters;

	uint32 mLongNameStringID;
	String mLongName;
	String mShortName;
	uint16 mNextSpawnID;
	ZoneID mID;
	InstanceID mInstanceID;
	uint32 mPort;

	bool mInitialised; // Flag indicating whether the Zone has been initialised.
	EQStreamFactory* mStreamFactory;
	EQStreamIdentifier* mStreamIdentifier;
	std::list<Character*> mCharacters; // List of Player Characters in Zone.
	std::list<ZoneClientConnection*> mPreConnections; // Zoning in or logging in
	std::list<ZoneClientConnection*> mConnections;
	//std::list<Character*> mLinkDeadCharacters;
	struct LinkDeadCharacter {
		Timer* mTimer;
		Character* mCharacter;
	};
	std::list<LinkDeadCharacter> mLinkDeadCharacters;
	//std::unordered_map<Timer, Character*> mLinkDeadCharacters;
};