#pragma once

#include "../common/types.h"
#include "../common/timer.h"
#include <string>

class EQStreamInterface;
class EQApplicationPacket;
class Zone;
class Character;
class DataStore;
class CommandHandler;
struct NewSpawn_Struct;

static const std::string EmptyString = std::string();

class ZoneClientConnection {
public:
	enum ZoneConnectionStatus {
		NONE,
		ZoneEntryReceived,		// On OP_ZoneEntry
		PlayerProfileSent,
		ClientRequestZoneData,	// On OP_ReqNewZone
		ZoneInformationSent,
		ClientRequestSpawn,		// On OP_ReqClientSpawn
		Complete				// On OP_ClientReady
	};
public:
	ZoneClientConnection(EQStreamInterface* pStreamInterface, DataStore* pDataStore, Zone* pZone);
	~ZoneClientConnection();
	void update();
	bool _handlePacket(const EQApplicationPacket* pPacket);
	void _handleZoneEntry(const EQApplicationPacket* pPacket);
	void _handleRequestClientSpawn(const EQApplicationPacket* pPacket);

	void dropConnection();

	void sendPosition();
	void sendMessage(uint32 pType, std::string pMessage);
	void sendSimpleMessage(uint32 pType, uint32 pStringID);
	void sendSimpleMessage(uint32 pType, uint32 pStringID, std::string pParameter0, std::string pParameter1 = EmptyString, std::string pParameter2 = EmptyString, std::string pParameter3 = EmptyString, std::string pParameter4 = EmptyString, std::string pParameter5 = EmptyString, std::string pParameter6 = EmptyString, std::string pParameter7 = EmptyString, std::string pParameter8 = EmptyString, std::string pParameter9 = EmptyString);
	void sendAppearance(uint16 pType, uint32 pParameter);
	void sendHPUpdate();

	void sendExperienceUpdate();
	void sendExperienceGain();
	void sendExperienceLoss();
	void sendLevelGain();
	void sendLevelLost();
	void sendLevelAppearance();
	void sendLevelUpdate();
	void sendStats();

	void populateSpawnStruct(NewSpawn_Struct* pSpawn);
	EQApplicationPacket* makeCharacterSpawnPacket(); // Caller is responsible for memory deallocation.
	EQApplicationPacket* makeCharacterPositionUpdate(); // Caller is responsible for memory deallocation.

	void sendPacket(EQApplicationPacket* pPacket);

private:
	void _sendTimeOfDay();

	void _sendPlayerProfile();
	void _sendZoneEntry();
	void _sendZoneSpawns();
	void _sendTributeUpdate();
	void _sendInventory();
	void _sendWeather();

	void _sendPreLogOutReply();
	void _sendLogOutReply();
	

	void _sendDoors();
	void _sendObjects();
	void _sendZonePoints();
	void _sendAAStats();
	void _sendZoneServerReady();
	void _sendExpZoneIn();
	void _sendWorldObjectsSent();
	void _handleClientUpdate(const EQApplicationPacket* pPacket);
	void _handleSpawnAppearance(const EQApplicationPacket* pPacket);
	void _handleCamp(const EQApplicationPacket* pPacket);
	void _handleChannelMessage(const EQApplicationPacket* pPacket);
	void _handleLogOut(const EQApplicationPacket* pPacket);
	void _handleDeleteSpawn(const EQApplicationPacket* pPacket);
	void _handleRequestNewZoneData(const EQApplicationPacket* pPacket);

	void _sendNewZoneData();
	void _handleClientReady(const EQApplicationPacket* pPacket);
	void _handleSendAATable(const EQApplicationPacket* pPacket);
	void _handleUpdateAA(const EQApplicationPacket* pPacket);
	void _handleTarget(const EQApplicationPacket* pPacket);
	void _handleTGB(const EQApplicationPacket* pPacket);
	void _handleEmote(const EQApplicationPacket* pPacket);
	void _handleAnimation(const EQApplicationPacket* pPacket);
	


	bool mConnected;
	Timer mForceSendPositionTimer;
	EQStreamInterface* mStreamInterface;
	DataStore* mDataStore;
	Zone* mZone;
	Character* mCharacter;
	ZoneConnectionStatus mZoneConnectionStatus;
	CommandHandler* mCommandHandler; // For now every connection has their own.
};