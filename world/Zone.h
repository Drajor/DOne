#pragma once

#include "Constants.h"
#include "ClientAuthentication.h"
#include "../common/timer.h"
#include "Vector3.h"

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
class LootAllocator;

struct ZonePoint {
	uint16 mID = 0;
	Vector3 mPosition;

	Vector3 mDestinationPosition;
	float mDestinationHeading = 0.0f;

	uint16 mDestinationZoneID = 0;
	uint16 mDestinationInstanceID = 0;
};

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
	inline const Vector3& getSafePoint() const { return mSafePoint; }
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

	void handleSay(Character* pCharacter, const String pMessage);
	void handleShout(Character* pCharacter, const String pMessage);
	void handleOOC(Character* pCharacter, const String pMessage);
	void handleEmote(Character* pCharacter, const String pMessage);
	void handleAuction(Character* pCharacter, const String pMessage);
	void handleTell(Character* pCharacter, const String& pTargetName, const String& pMessage);
	void handleAnimation(Actor* pActor, const uint8 pAnimation, const uint8 pSpeed = 10, const bool pIncludeSender = false);
	void handleLevelIncrease(Character* pCharacter);
	void handleLevelDecrease(Character* pCharacter);
	void notifyCharacterGM(Character* pCharacter);

	void handleTarget(Character* pCharacter, uint16 pSpawnID);
	void handleFaceChange(Character* pCharacter);

	void handleDeath(Actor* pActor, Actor* pKiller, const uint32 pDamage, const uint32 pSkill);

	void handleDamage(Actor* pAttacker, Actor* pDefender, const int32 pAmount, const uint8 pType, const uint16 pSpellID);

	// Guild
	void notifyGuildsChanged();
	void notifyCharacterGuildChange(Character* pCharacter);

	void handleZoneChange(Character* pCharacter, const uint16 pZoneID, const uint16 pInstanceID, const Vector3& pPosition);

	void moveCharacter(Character* pCharacter, float pX, float pY, float pZ);
	inline void moveCharacter(Character* pCharacter, Vector3& pPosition) { moveCharacter(pCharacter, pPosition.x, pPosition.y, pPosition.z); }
	uint16 getNextSpawnID() { return mNextSpawnID++; }

	bool trySendTell(const String& pSenderName, const String& pTargetName, const String& pMessage);
	void processCharacterQueuedTells(Character* );
	void whoRequest(Character* pCharacter, WhoFilter& pFilter);
	void getWhoMatches(std::list<Character*>& pMatches, WhoFilter& pFilter);
	void requestSave(Character* pCharacter);

	
	void handleVisibilityAdd(Character* pCharacter, Actor* pAddActor);
	void handleVisibilityRemove(Character* pCharacter, Actor* pRemoveActor);

	void handleSurnameChange(Actor* pActor);
	void handleTitleChanged(Character* pCharacter, const uint32 pOption);

	void handleCastingBegin(Character* pCharacter, const uint16 pSlot, const uint32 pSpellID);
	void handleCastingFinished(Actor* pActor);

	void sendToVisible(Actor* pActor, EQApplicationPacket* pPacket);
	void sendToVisible(Character* pCharacter, EQApplicationPacket* pPacket, bool pIncludeSender);
	void sendToTargeters(Actor* pActor, EQApplicationPacket* pPacket);

	const bool populate();
	const bool depopulate();

	void handleBeginLootRequest(Character* pCharacter, const uint32 pCorpseSpawnID);
	void handleEndLootRequest(Character* pCharacter);

	void handleConsider(Character* pCharacter, const uint32 pSpawnID);
	void handleConsiderCorpse(Character* pCharacter, const uint32 pSpawnID);

	const std::list<ZonePoint*>& getZonePoints() { return mZonePoints; }

	const bool canBank(Character* pCharacter);

	void handleCriticalHit(Character* pCharacter, int32 pDamage);
	void handleHPChange(Actor* pActor);
private:

	const bool loadZonePoints();
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
	Vector3 mSafePoint;
	uint16 mNextSpawnID = 1;
	const uint16 mID;
	const uint16 mInstanceID;
	const uint32 mPort;

	bool mInitialised = false; // Flag indicating whether the Zone has been initialised.
	bool mPopulated = false;
	EQStreamFactory* mStreamFactory = nullptr;
	EQStreamIdentifier* mStreamIdentifier = nullptr;

	ZonePoint* _getClosestZonePoint(const Vector3& pPosition);
	std::list<ZonePoint*> mZonePoints;

	void _populate(SpawnPoint* pSpawnPoint);
	void _addRespawn(SpawnPoint* pSpawnPoint);
	void _updateSpawnPoints();
	std::list<SpawnPoint*> mRespawns;
	std::list<SpawnPoint*> mSpawnPoints;
	
	LootAllocator* mLootAllocator = nullptr;

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