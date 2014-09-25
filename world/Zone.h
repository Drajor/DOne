#pragma once

#include "Constants.h"
#include "ClientAuthentication.h"
#include "../common/timer.h"

class EQStreamFactory;
class EQStreamIdentifier;
class EQApplicationPacket;
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
class SpawnPoint;
struct ZonePoint;

class Zone {
public:
	Zone(const uint32 pPort, const uint16 pZoneID, const uint16 pInstanceID);
	~Zone();

	const bool initialise();
	void shutdown();
	void update();
	const bool checkAuthentication(Character* pCharacter);

	inline const String& getLongName() const { return mLongName; }
	inline const String& getShortName() const { return mShortName; }
	inline const uint32 getLongNameStringID() const { return mLongNameStringID; }
	inline const float getGravity() const { return 0.4f; }
	inline const float getMinimumZ() const { return -5000.0f; } // NOTE: The lowest point in the zone a Character should be able to reach.
	inline const uint8 getZoneType() const { return 255; } // Unknown.
	inline const uint32 getNumCharacters() const { return mCharacters.size(); }
	inline const uint16 getID() const { return mID; }
	inline const uint16 getInstanceID() const { return mInstanceID; }
	inline const uint16 getPort() const { return mPort; }

	Character* findCharacter(const String pCharacterName);
	Actor* findActor(const SpawnID pSpawnID);

	void addActor(Actor* pActor);
	void removeActor(Actor* pActor);

	void notifyCharacterZoneIn(Character* pCharacter);
	void handleActorPositionChange(Actor* pActor);
	void handleLinkDead(Character* pCharacter);
	void handleAFK(Character* pCharacter);
	void handleShowHelm(Character* pCharacter);
	void handleAnonymous(Character* pCharacter);
	void handleStanding(Character* pCharacter);
	void handleSitting(Character* pCharacter);
	void handleCrouching(Character* pCharacter);

	void notifyCharacterChatSay(Character* pCharacter, const String pMessage);
	void notifyCharacterChatShout(Character* pCharacter, const String pMessage);
	void notifyCharacterChatOOC(Character* pCharacter, const String pMessage);
	void notifyCharacterEmote(Character* pCharacter, const String pMessage);
	void notifyCharacterChatAuction(Character* pCharacter, const String pMessage);
	void handleTell(Character* pCharacter, const String& pTargetName, const String& pMessage);
	void notifyCharacterAnimation(Character* pCharacter, uint8 pAction, uint8 pAnimationID, bool pIncludeSender = false);
	void handleLevelIncrease(Character* pCharacter);
	void handleLevelDecrease(Character* pCharacter);
	void notifyCharacterGM(Character* pCharacter);

	void handleTarget(Character* pCharacter, SpawnID pSpawnID);
	void handleFaceChange(Character* pCharacter);

	void handleDeath(Actor* pActor);

	// Guild
	void notifyGuildsChanged();
	void notifyCharacterGuildChange(Character* pCharacter);

	void handleZoneChange(Character* pCharacter, const uint16 pZoneID, const uint16 pInstanceID);

	void moveCharacter(Character* pCharacter, float pX, float pY, float pZ);
	uint16 getNextSpawnID() { return mNextSpawnID++; }

	bool trySendTell(const String& pSenderName, const String& pTargetName, const String& pMessage);
	void processCharacterQueuedTells(Character* );
	void whoRequest(Character* pCharacter, WhoFilter& pFilter);
	void getWhoMatches(std::list<Character*>& pMatches, WhoFilter& pFilter);
	void requestSave(Character* pCharacter);

	
	void handleVisibilityAdd(Character* pCharacter, Actor* pAddActor);
	void handleVisibilityRemove(Character* pCharacter, Actor* pRemoveActor);

	void handleSurnameChange(Actor* pActor);
	void handleTitleChanged(Character* pCharacter, TitleOption pOption);

	void handleCastingBegin(Character* pCharacter, const uint16 pSlot, const uint32 pSpellID);
	void handleCastingFinished(Actor* pActor);

	void sendToVisible(Actor* pActor, EQApplicationPacket* pPacket);
	void sendToVisible(Character* pCharacter, EQApplicationPacket* pPacket, bool pIncludeSender);

	const bool populate();
	const bool depopulate();

	void handleBeginLootRequest(Character* pCharacter, const uint32 pCorpseSpawnID);
	void handleEndLootRequest(Character* pCharacter);

	void handleConsider(Character* pCharacter, const uint32 pSpawnID);
	void handleConsiderCorpse(Character* pCharacter, const uint32 pSpawnID);

	const std::list<ZonePoint*>& getZonePoints() { return mZonePoints; }
private:

	const bool loadSpawnPoints();

	// Performs a global Character search.
	Character* _findCharacter(const String& pCharacterName, bool pIncludeZoning = false);

	void _updateCharacters();
	void _updateNPCs();
	void _updateConnections();
	void _updatePreConnections();

	void _sendDespawn(const uint16 pSpawnID, const bool pDecay = false);
	void _sendChat(Character* pCharacter, ChannelID pChannel, const String pMessage);
	void _sendSpawnAppearance(Character* pCharacter, SpawnAppearanceType pType, uint32 pParameter, bool pIncludeSender = false);
	void _sendLevelAppearance(Character* pCharacter);
	void _handleIncomingConnections();
	void _sendCharacterLevel(Character* pCharacter);
	void _handleWhoRequest(Character* pCharacter, WhoFilter& pFilter);

	void _onLeaveZone(Character* pCharacter);
	void _onCamp(Character* pCharacter);
	void _onLinkdead(Character* pCharacter);

	uint32 mLongNameStringID = 0;
	String mLongName = "";
	String mShortName = "";
	uint16 mNextSpawnID = 1;
	const uint16 mID;
	const uint16 mInstanceID;
	const uint32 mPort;

	bool mInitialised = false; // Flag indicating whether the Zone has been initialised.
	bool mPopulated = false;
	EQStreamFactory* mStreamFactory = nullptr;
	EQStreamIdentifier* mStreamIdentifier = nullptr;

	void _populate(SpawnPoint* pSpawnPoint);
	void _addRespawn(SpawnPoint* pSpawnPoint);
	void _updateSpawnPoints();
	std::list<SpawnPoint*> mRespawns;
	std::list<SpawnPoint*> mSpawnPoints;

	Scene* mScene = nullptr;
	std::list<Character*> mCharacters;
	std::list<NPC*> mNPCs;
	std::list<Actor*> mActors;

	std::list<ZoneClientConnection*> mPreConnections; // Zoning in or logging in
	std::list<ZoneClientConnection*> mConnections;

	std::list<ZonePoint*> mZonePoints;

	struct LinkDeadCharacter {
		Timer* mTimer;
		Character* mCharacter;
	};
	std::list<LinkDeadCharacter> mLinkDeadCharacters;
};