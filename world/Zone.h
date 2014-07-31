#pragma once

#include "Constants.h"
#include "ClientAuthentication.h"
#include <list>
#include <map>
#include "../common/types.h"

class EQStreamFactory;
class EQStreamIdentifier;
class DataStore;
class ZoneClientConnection;

class World;
class ZoneManager;
class Character;
class Group;
class GroupManager;

class Zone {
public:
	Zone(World* pWorld, ZoneManager* pZoneManager, GroupManager* pGroupManager, DataStore* pDataStore, uint32 pPort, uint32 pZoneID, uint32 pInstanceID = 0);
	~Zone();
	bool initialise();
	std::string getLongName() { return mLongName; }
	std::string getShortName() { return mShortName; }
	uint32 getLongNameStringID() { return mLongNameStringID; }

	void addAuthentication(ClientAuthentication& pAuthentication, std::string pCharacterName);
	void removeAuthentication(std::string pCharacterName);
	bool checkAuthentication(std::string pCharacterName);
	void addCharacter(Character* pCharacter);

	void shutdown();
	void update();

	void _updateCharacters();

	void _updateConnections();

	void _updatePreConnections();

	uint32 getID() { return mID; }
	uint32 getInstanceID() { return mInstanceID; }
	uint16 getPort() { return mPort; }
	Character* findCharacter(const std::string pCharacterName);

	void notifyCharacterZoneIn(Character* pCharacter);
	void notifyCharacterZoneOut(Character* pCharacter);
	void notifyCharacterPositionChanged(Character* pCharacter);
	void notifyCharacterLogOut(Character* pCharacter);
	void notifyCharacterLinkDead(Character* pCharacter);
	void notifyCharacterAFK(Character* pCharacter);
	void notifyCharacterShowHelm(Character* pCharacter);
	void notifyCharacterAnonymous(Character* pCharacter);
	void notifyCharacterStanding(Character* pCharacter);
	void notifyCharacterSitting(Character* pCharacter);
	void notifyCharacterCrouching(Character* pCharacter);
	void notifyCharacterChatSay(Character* pCharacter, const std::string pMessage);
	void notifyCharacterChatShout(Character* pCharacter, const std::string pMessage);
	void notifyCharacterChatOOC(Character* pCharacter, const std::string pMessage);
	void notifyCharacterChatGroup(Character* pCharacter, const std::string pMessage);
	void notifyCharacterEmote(Character* pCharacter, const std::string pMessage);
	void notifyCharacterChatAuction(Character* pCharacter, const std::string pMessage);
	void notifyCharacterChatTell(Character* pCharacter, const std::string& pTargetName, const std::string& pMessage);
	void notifyCharacterAnimation(Character* pCharacter, uint8 pAction, uint8 pAnimationID, bool pIncludeSender = false);
	void notifyCharacterLevelIncrease(Character* pCharacter);
	void notifyCharacterLevelDecrease(Character* pCharacter);
	void notifyCharacterGM(Character* pCharacter);

	// Group
	void notifyCharacterGroupInvite(Character* pCharacter, const std::string pToCharacterName);
	void notifyCharacterAcceptGroupInvite(Character* pCharacter, std::string pToCharacterName);
	void notifyCharacterDeclineGroupInvite(Character* pCharacter, std::string pToCharacterName);
	void notifyCharacterGroupDisband(Character* pCharacter, const std::string& pRemoveCharacterName);

	void moveCharacter(Character* pCharacter, float pX, float pY, float pZ);
	uint16 getNextSpawnID() { return mNextSpawnID++; }

	bool trySendTell(const std::string& pSenderName, const std::string& pTargetName, const std::string& pMessage);
	void processCharacterQueuedTells(Character* );
	void whoRequest(Character* pCharacter, WhoFilter& pFilter);
	void getWhoMatches(std::list<Character*>& pMatches, WhoFilter& pFilter);
	void requestSave(Character* pCharacter);
	
private:

	// Performs a global Character search.
	Character* _findCharacter(const std::string& pCharacterName, bool pIncludeZoning = false);

	void _sendChat(Character* pCharacter, ChannelID pChannel, const std::string pMessage);
	void _sendSpawnAppearance(Character* pCharacter, SpawnAppearanceType pType, uint32 pParameter, bool pIncludeSender = false);
	void _sendLevelAppearance(Character* pCharacter);
	void _handleIncomingConnections();
	void _sendCharacterLevel(Character* pCharacter);
	void _handleWhoRequest(Character* pCharacter, WhoFilter& pFilter);
	void _handleCharacterLinkDead(Character* pCharacter);
















	std::map<std::string, ClientAuthentication> mAuthenticatedCharacters;

	uint32 mLongNameStringID;
	std::string mLongName;
	std::string mShortName;
	uint16 mNextSpawnID;
	uint32 mID; // Zone ID
	uint32 mInstanceID;
	uint32 mPort;

	bool mInitialised; // Flag indicating whether the Zone has been initialised.
	EQStreamFactory* mStreamFactory;
	EQStreamIdentifier* mStreamIdentifier;
	World* mWorld;
	ZoneManager* mZoneManager;
	GroupManager* mGroupManager;
	DataStore* mDataStore;
	std::list<Character*> mCharacters; // List of Player Characters in Zone.
	std::list<ZoneClientConnection*> mPreConnections; // Zoning in or logging in
	std::list<ZoneClientConnection*> mConnections;
	std::list<Character*> mLinkDeadCharacters;
};