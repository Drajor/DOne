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
class Actor;
class NPC;
class Scene;

class Zone {
public:
	Zone(const uint32 pPort, const ZoneID pZoneID, const InstanceID pInstanceID = 0);
	~Zone();

	bool initialise();

	inline const String& getLongName() const { return mLongName; }
	inline const String& getShortName() const { return mShortName; }
	inline const uint32 getLongNameStringID() const { return mLongNameStringID; }
	inline const float getGravity() const { return 0.4f; }
	inline const float getMinimumZ() const { return -5000.0f; } // NOTE: The lowest point in the zone a Character should be able to reach.
	inline const uint8 getZoneType() const { return 255; } // Unknown.
	inline const uint32 getNumCharacters() const { return mCharacters.size(); }
	inline const ZoneID getID() const { return mID; }
	inline const InstanceID getInstanceID() const { return mInstanceID; }
	inline const uint16 getPort() const { return mPort; }

	void addAuthentication(ClientAuthentication& pAuthentication, String pCharacterName);
	void removeAuthentication(String pCharacterName);
	bool checkAuthentication(String pCharacterName);
	bool getAuthentication(String pCharacterName, ClientAuthentication& pAuthentication);

	void shutdown();
	void update();

	void _updateCharacters();

	void _updateConnections();


	

	void _updatePreConnections();



	Character* findCharacter(const String pCharacterName);
	Actor* findActor(const SpawnID pSpawnID);

	void addActor(Actor* pActor);
	void removeActor(Actor* pActor);

	void notifyCharacterZoneIn(Character* pCharacter);
	void handleActorPositionChange(Actor* pActor);
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
	void notifyCharacterEmote(Character* pCharacter, const String pMessage);
	void notifyCharacterChatAuction(Character* pCharacter, const String pMessage);
	void notifyCharacterChatTell(Character* pCharacter, const String& pTargetName, const String& pMessage);
	void notifyCharacterAnimation(Character* pCharacter, uint8 pAction, uint8 pAnimationID, bool pIncludeSender = false);
	void notifyCharacterLevelIncrease(Character* pCharacter);
	void notifyCharacterLevelDecrease(Character* pCharacter);
	void notifyCharacterGM(Character* pCharacter);

	void handleTarget(Character* pCharacter, SpawnID pSpawnID);
	void handleFaceChange(Character* pCharacter);

	// Guild
	void notifyGuildsChanged();
	void notifyCharacterGuildChange(Character* pCharacter);

	void notifyCharacterZoneChange(Character* pCharacter, ZoneID pZoneID, uint16 pInstanceID);

	void moveCharacter(Character* pCharacter, float pX, float pY, float pZ);
	uint16 getNextSpawnID() { return mNextSpawnID++; }

	bool trySendTell(const String& pSenderName, const String& pTargetName, const String& pMessage);
	void processCharacterQueuedTells(Character* );
	void whoRequest(Character* pCharacter, WhoFilter& pFilter);
	void getWhoMatches(std::list<Character*>& pMatches, WhoFilter& pFilter);
	void requestSave(Character* pCharacter);

	
	void handleVisibilityAdd(Character* pCharacter, Actor* pAddActor);
	void handleVisibilityRemove(Character* pCharacter, Actor* pRemoveActor);
	
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

	void _onLeaveZone(Character* pCharacter);
	void _onCamp(Character* pCharacter);
	void _onLinkdead(Character* pCharacter);
	std::map<String, ClientAuthentication> mAuthenticatedCharacters;

	uint32 mLongNameStringID = 0;
	String mLongName = "";
	String mShortName = "";
	uint16 mNextSpawnID = 1;
	const ZoneID mID;
	const InstanceID mInstanceID;
	const uint32 mPort;

	bool mInitialised = false; // Flag indicating whether the Zone has been initialised.
	EQStreamFactory* mStreamFactory = nullptr;
	EQStreamIdentifier* mStreamIdentifier = nullptr;

	Scene* mScene = nullptr;
	std::list<Character*> mCharacters;
	std::list<NPC*> mNPCs;
	std::list<Actor*> mActors;

	std::list<ZoneClientConnection*> mPreConnections; // Zoning in or logging in
	std::list<ZoneClientConnection*> mConnections;

	struct LinkDeadCharacter {
		Timer* mTimer;
		Character* mCharacter;
	};
	std::list<LinkDeadCharacter> mLinkDeadCharacters;
};