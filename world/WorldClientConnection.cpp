#include "WorldClientConnection.h"
#include "GuildManager.h"

#include "../common/debug.h"
#include "../common/EQPacket.h"
#include "../common/EQStreamIntf.h"
#include "../common/misc.h"
#include "../common/rulesys.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "../common/packet_dump.h"
#include "../common/EQStreamIntf.h"
#include "../common/Item.h"
#include "../common/races.h"
#include "../common/classes.h"
#include "../common/languages.h"
#include "../common/skills.h"
#include "../common/extprofile.h"
#include "../common/StringUtil.h"
#include "../common/clientversions.h"
#include "../common/MiscFunctions.h"

#include <iostream>
#include <iomanip>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <limits.h>

#include "../common/servertalk.h"
#include "World.h"
#include "Utility.h"
#include "Limits.h"
#include "Constants.h"
#include "LogSystem.h"
#include "AccountManager.h"
#include "Data.h"
#include "Payload.h"
#include "Settings.h"

#ifdef _WINDOWS
	#include <windows.h>
	#include <winsock.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
#endif

WorldClientConnection::WorldClientConnection(EQStreamInterface* pStreamInterface) :
	mStreamInterface(pStreamInterface)
{
	mIP = mStreamInterface->GetRemoteIP();
	mPort = ntohs(mStreamInterface->GetRemotePort());

	char_name[0] = 0;

	ClientVersionBit = 1 << (mStreamInterface->ClientVersion() - 1);
}

WorldClientConnection::~WorldClientConnection() {
	//let the stream factory know were done with this stream
	mStreamInterface->Close();
	mStreamInterface->ReleaseFromUse();
}

void WorldClientConnection::_sendLogServer() {
	auto outPacket = new EQApplicationPacket(OP_LogServer, sizeof(LogServer_Struct));
	auto payload = reinterpret_cast<LogServer_Struct*>(outPacket->pBuffer);

	strcpy(payload->worldshortname, Settings::getServerShortName().c_str());	
	payload->enablemail = 1;
	payload->enablevoicemacros = 1;
	payload->enable_pvp = 0;
	payload->enable_petition_wnd = 1;
	payload->enable_FV = 0;

	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_sendEnterWorld(String pCharacterName) {
	auto outPacket = new EQApplicationPacket(OP_EnterWorld, pCharacterName.length() + 1);
	strcpy(reinterpret_cast<char*>(outPacket->pBuffer), pCharacterName.c_str());
	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_sendExpansionInfo() {
	auto outPacket = new EQApplicationPacket(OP_ExpansionInfo, sizeof(ExpansionInfo_Struct));
	auto payload = reinterpret_cast<ExpansionInfo_Struct*>(outPacket->pBuffer);
	payload->Expansions = (RuleI(World, ExpansionSettings));
	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_sendCharacterSelectInfo() {
	auto outPacket = new EQApplicationPacket(OP_SendCharInfo, sizeof(CharacterSelect_Struct));
	auto payload = reinterpret_cast<CharacterSelect_Struct*>(outPacket->pBuffer);
	AccountData* accountData = AccountManager::getInstance().getAccount(mAccountID);
	EXPECTED(accountData);

	//for (auto i = 0; i < MAX_CHARACTERS_CHARSELECT; i++) {
	int charSlot = 0;
	for (auto i : accountData->mCharacterData) {
		strcpy(payload->name[charSlot], i->mName.c_str());
		payload->race[charSlot] = i->mRace;
		payload->class_[charSlot] = i->mClass;
		payload->zone[charSlot] = i->mZoneID;
		payload->level[charSlot] = i->mLevel;

		payload->face[charSlot] = i->mFaceStyle;
		payload->gender[charSlot] = i->mGender;
		payload->beard[charSlot] = i->mBeardStyle;
		payload->beardcolor[charSlot] = i->mBeardColour;
		payload->hairstyle[charSlot] = i->mHairStyle;
		payload->haircolor[charSlot] = i->mHairColour;
		payload->drakkin_heritage[charSlot] = i->mDrakkinHeritage;
		payload->drakkin_tattoo[charSlot] = i->mDrakkinTattoo;
		payload->drakkin_details[charSlot] = i->mDrakkinDetails;
		payload->deity[charSlot] = i->mDeity;
		payload->drakkin_tattoo[charSlot] = i->mDrakkinTattoo;
		payload->eyecolor1[charSlot] = i->mEyeColourLeft;
		payload->eyecolor2[charSlot] = i->mEyeColourRight;


		payload->tutorial[charSlot] = 0;
		payload->gohome[charSlot] = 0;

		// Equipment
		payload->primary[charSlot] = i->mPrimary;
		payload->secondary[charSlot] = i->mSecondary;

		for (auto j = 0; j < Limits::Account::MAX_EQUIPMENT_SLOTS; j++) {
			payload->cs_colors[charSlot][j].color = i->mEquipment[j].mColour;
			payload->equip[charSlot][j] = i->mEquipment[j].mMaterial;
		}

		charSlot++;
	}

	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_sendPostEnterWorld() {
	auto outPacket = new EQApplicationPacket(OP_PostEnterWorld, 1);
	outPacket->size = 0;
	_queuePacket(outPacket);
	safe_delete(outPacket);
}

bool WorldClientConnection::_handleSendLoginInfoPacket(const EQApplicationPacket* pPacket) {
	using namespace Payload::World;
	EXPECTED_BOOL(pPacket);
	EXPECTED_BOOL(LoginInformation::sizeCheck(pPacket->size));

	auto payload = LoginInformation::convert(pPacket->pBuffer);

	String accountIDStr = Utility::safeString(payload->mInformation, 19);
	String accountKey = Utility::safeString(payload->mInformation + accountIDStr.length() + 1, 16);

	uint32 accountID = 0;
	EXPECTED_BOOL(Utility::stoSafe(accountID, accountIDStr));

	// Check authentication.
	// NOTE: This ensures that World is expecting a connection from this account.
	EXPECTED_BOOL(World::getInstance().checkAuthentication(this, accountID, accountKey));

	mZoning = (payload->mZoning == 1);
	// TODO: Ensure we are expecting a zoning client.

	// Going to: Another Zone
	if (mZoning) {
		_sendLogServer();
		_sendApproveWorld();
		_sendEnterWorld("Playerzero");
		_sendPostEnterWorld();
	}
	// Going to: Character Selection Screen.
	else {
		AccountManager::getInstance().ensureAccountLoaded(accountID);

		_sendGuildList(); // NOTE: Required. Character guild names do not work (on entering world) without it.
		_sendLogServer();
		_sendApproveWorld();
		_sendEnterWorld(""); // Empty character name when coming from Server Select. 
		_sendPostEnterWorld(); // Required.
		_sendExpansionInfo(); // Required.
		_sendCharacterSelectInfo(); // Required.
	}

	return true;

	/*
	OP_GuildsList, OP_LogServer, OP_ApproveWorld
	All sent in EQEmu but not actually required to get to Character Select. More research on the effects of not sending are required.
	*/
}

bool WorldClientConnection::_handleNameApprovalPacket(const EQApplicationPacket* pPacket) {
	// TODO: Check this packet size.
	
	snprintf(char_name, 64, "%s", (char*)pPacket->pBuffer);
	// TODO: Consider why race and class are sent here?
	uchar race = pPacket->pBuffer[64];
	uchar clas = pPacket->pBuffer[68];

	auto outPacket = new EQApplicationPacket(OP_ApproveName, 1);
	bool valid = true;
	String characterName = char_name;
	const int nameLength = characterName.length();
	// Check length (4 >= x <= 15) 
	if (nameLength < 4 || nameLength > 15) {
		valid = false;
	}
	// Check case of first character (must be uppercase)
	else if (characterName[0] < 'A' || characterName[0] > 'Z') {
		valid = false;
	}
	// Check each character is alpha.
	for (int i = 0; i < nameLength; i++) {
		if (!isalpha(characterName[i])) {
			valid = false;
			break;
		}
	}
	// Check if name already in use.
	if (valid && !World::getInstance().isCharacterNameUnique(characterName)) {
		valid = false;
	}
	// Check if name is reserved.
	if (valid && World::getInstance().isCharacterNameReserved(characterName)) {
		valid = false;
	}
	// Reserve character name.
	if (valid) {
		// For the rare chance that more than one user tries to create a character with same name.
		World::getInstance().reserveCharacterName(mAccountID, characterName);
		mReservedCharacterName = characterName;
	}

	outPacket->pBuffer[0] = valid? 1 : 0;
	_queuePacket(outPacket);
	safe_delete(outPacket);

	if(!valid) {
		memset(char_name, 0, sizeof(char_name));
	}

	return true;
}

bool WorldClientConnection::_handleGenerateRandomNamePacket(const EQApplicationPacket* pPacket) {
	// creates up to a 10 char name
	char vowels[18]="aeiouyaeiouaeioe";
	char cons[48]="bcdfghjklmnpqrstvwxzybcdgklmnprstvwbcdgkpstrkd";
	char rndname[17]="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	char paircons[33]="ngrkndstshthphsktrdrbrgrfrclcr";
	int rndnum = MakeRandomInt(0, 75),n=1;
	bool dlc=false;
	bool vwl=false;
	bool dbl=false;
	if (rndnum>63)
	{	// rndnum is 0 - 75 where 64-75 is cons pair, 17-63 is cons, 0-16 is vowel
		rndnum=(rndnum-61)*2;	// name can't start with "ng" "nd" or "rk"
		rndname[0]=paircons[rndnum];
		rndname[1]=paircons[rndnum+1];
		n=2;
	}
	else if (rndnum>16)
	{
		rndnum-=17;
		rndname[0]=cons[rndnum];
	}
	else
	{
		rndname[0]=vowels[rndnum];
		vwl=true;
	}
	int namlen=MakeRandomInt(5, 10);
	for (int i=n;i<namlen;i++)
	{
		dlc=false;
		if (vwl)	//last char was a vowel
		{			// so pick a cons or cons pair
			rndnum=MakeRandomInt(0, 62);
			if (rndnum>46)
			{	// pick a cons pair
				if (i>namlen-3)	// last 2 chars in name?
				{	// name can only end in cons pair "rk" "st" "sh" "th" "ph" "sk" "nd" or "ng"
					rndnum=MakeRandomInt(0, 7)*2;
				}
				else
				{	// pick any from the set
					rndnum=(rndnum-47)*2;
				}
				rndname[i]=paircons[rndnum];
				rndname[i+1]=paircons[rndnum+1];
				dlc=true;	// flag keeps second letter from being doubled below
				i+=1;
			}
			else
			{	// select a single cons
				rndname[i]=cons[rndnum];
			}
		}
		else
		{		// select a vowel
			rndname[i]=vowels[MakeRandomInt(0, 16)];
		}
		vwl=!vwl;
		if (!dbl && !dlc)
		{	// one chance at double letters in name
			if (!MakeRandomInt(0, i+9))	// chances decrease towards end of name
			{
				rndname[i+1]=rndname[i];
				dbl=true;
				i+=1;
			}
		}
	}

	rndname[0]=toupper(rndname[0]);
	NameGeneration_Struct* payload = reinterpret_cast<NameGeneration_Struct*>(pPacket->pBuffer);
	memset(payload->name,0,64);
	strcpy(payload->name,rndname);

	_queuePacket(pPacket);
	return true;
}

bool WorldClientConnection::_handleCharacterCreateRequestPacket(const EQApplicationPacket* packet) {
	struct RaceClassAllocation {
		unsigned int Index;
		unsigned int BaseStats[7];
		unsigned int DefaultPointAllocation[7];
	};

	struct RaceClassCombos {
		unsigned int ExpansionRequired;
		unsigned int Race;
		unsigned int Class;
		unsigned int Deity;
		unsigned int AllocationIndex;
		unsigned int Zone;
	};
	/*
	[Client Limitation][HoT] If the client is not an race/class combinations for any one class the class icon will not be greyed out.
	*/
	static const std::list<int> PlayableRaces = {
		PlayableRaceIDs::Human
	};

	static const std::list<int> PlayableClasses = {
		ClassIDs::Warrior,
		ClassIDs::Cleric,
		ClassIDs::Wizard,
	};

	std::list<RaceClassAllocation> raceClassAllocations;
	RaceClassAllocation r;
	r.Index = 0;
	memset(r.BaseStats, 0, sizeof(r.BaseStats));
	memset(r.DefaultPointAllocation, 0, sizeof(r.DefaultPointAllocation));
	raceClassAllocations.push_back(r);
	std::list<RaceClassCombos> raceClassCombos;

	//int allocationIndex = 0;
	for (auto i : PlayableRaces) {
		for (auto j : PlayableClasses) {
			raceClassCombos.push_back({ 0, i, j, PlayerDeityIDs::Agnostic, 0, ZoneIDs::NorthQeynos });
		}
	}

	uint32 allocs = raceClassAllocations.size();
	uint32 combos = raceClassCombos.size();
	uint32 len = sizeof(RaceClassAllocation) * allocs;
	len += sizeof(RaceClassCombos) * combos;
	len += sizeof(uint8);
	len += sizeof(uint32);
	len += sizeof(uint32);

	auto outPacket = new EQApplicationPacket(OP_CharacterCreateRequest, len);
	unsigned char *ptr = outPacket->pBuffer;
	*((uint8*)ptr) = 0;
	ptr += sizeof(uint8);

	*((uint32*)ptr) = allocs; // number of allocs.
	ptr += sizeof(uint32);

	for (auto i : raceClassAllocations) {
		memcpy(ptr, &i, sizeof(RaceClassAllocation));
		ptr += sizeof(RaceClassAllocation);
	}

	*((uint32*)ptr) = combos; // number of combos.
	ptr += sizeof(uint32);
	for (auto i : raceClassCombos) {
		memcpy(ptr, &i, sizeof(RaceClassCombos));
		ptr += sizeof(RaceClassCombos);
	}

	_queuePacket(outPacket);
	safe_delete(outPacket);
	return true;
}

bool WorldClientConnection::_handleCharacterCreatePacket(const EQApplicationPacket* pPacket) {
	using namespace Payload::World;
	EXPECTED_BOOL(pPacket);
	PACKET_SIZE_CHECK_BOOL(CreateCharacter::sizeCheck(pPacket->size));

	auto payload = CreateCharacter::convert(pPacket->pBuffer);

	if (!AccountManager::getInstance().handleCharacterCreate(mAccountID, mReservedCharacterName, payload)){
		dropConnection();
		return false;
	}

	mReservedCharacterName = "";
	return true;
}

bool WorldClientConnection::_handleEnterWorldPacket(const EQApplicationPacket* pPacket) {
	using namespace Payload::World;
	EXPECTED_BOOL(pPacket);
	EXPECTED_BOOL(EnterWorld::sizeCheck(pPacket->size));

	auto payload = EnterWorld::convert(pPacket->pBuffer);
	String characterName = Utility::safeString(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED_BOOL(Limits::Character::nameLength(characterName));

	// Check: Account owns the Character.
	EXPECTED_BOOL(AccountManager::getInstance().checkOwnership(mAccountID, characterName));

	return World::getInstance().handleEnterWorld(this, characterName, mZoning);

	//// Client is between zones.
	//if (mZoning) {
	//	// Retrieve zone change data.
	//	ZoneTransfer zoneTransfer;
	//	if (World::getInstance().getCharacterZoneTransfer(characterName, zoneTransfer)) {
	//		World::getInstance().removeZoneTransfer(characterName); // Remove zone transfer authority.
	//		// Add authentication to the zone the character is going to.
	//		World::getInstance().addZoneAuthentication(mAuthentication, characterName, zoneTransfer.mToZoneID, zoneTransfer.mToInstanceID);
	//		// Tell client which Zone to connect to.
	//		_sendZoneServerInfo(World::getInstance().getZonePort(zoneTransfer.mToZoneID, zoneTransfer.mToInstanceID));

	//		return true;
	//	}
	//	// Failed to find ZoneTransfer data for character.
	//	else {
	//		StringStream ss; ss << "[World Client Connection] Unable to retrieve ZoneTransfer for " << characterName << ", dropping connection.";
	//		Log::error(ss.str());
	//		dropConnection();
	//		return false;
	//	}
	//}
	//// Client is moving from Character Select / Character Create.
	//else {
	//	
	//	//CharacterData* characterData = World::getInstance().loadCharacter(characterName);
	//	//EXPECTED_BOOL(characterData);

	//	// TODO: At the moment Characters always go to NQ.
	//	World::getInstance().addZoneAuthentication(mAuthentication, characterName, ZoneIDs::NorthQeynos, 0);
	//	// Send MOTD?
	//	// Send ChatServer?
	//	// Send ChatServer2?
	//	_sendChatServer();
	//	_sendZoneServerInfo(World::getInstance().getZonePort(ZoneIDs::NorthQeynos, 0));
	//	return true;
	//}
}

void WorldClientConnection::_sendChatServer(const String& pCharacterName) {
	std::stringstream ss;
	ss << "127.0.0.1" << "," << Settings::getUCSPort() << "," << Settings::getServerShortName() << "." << pCharacterName << ",";
	ss << "U" << std::hex << std::setfill('0') << std::setw(8) << 34; // TODO: Set up mail key
	String data = ss.str();
	auto outPacket = new EQApplicationPacket(OP_SetChatServer, reinterpret_cast<const unsigned char*>(data.c_str()), data.length()+1);
	auto outPacket2 = new EQApplicationPacket(OP_SetChatServer2, reinterpret_cast<const unsigned char*>(data.c_str()), data.length() + 1);
	_queuePacket(outPacket);
	_queuePacket(outPacket2);
	safe_delete(outPacket);
	safe_delete(outPacket2);
}

bool WorldClientConnection::_handleDeleteCharacterPacket(const EQApplicationPacket* pPacket) {
	using namespace Payload::World;
	EXPECTED_BOOL(pPacket);
	EXPECTED_BOOL(DeleteCharacter::sizeCheck(pPacket->size));

	//auto payload = DeleteCharacter::convert(pPacket->pBuffer);
	String characterName = Utility::safeString(reinterpret_cast<char*>(pPacket->pBuffer), Limits::Character::MAX_NAME_LENGTH);

	if (World::getInstance().deleteCharacter(mAccountID, characterName)) {
		StringStream ss; ss << "[World Client Connection] Character: " << characterName << " deleted.";
		Log::info(ss.str());
		_sendCharacterSelectInfo();
		return true;
	}

	Log::error("[World Client Connection] Failed to delete character.");
	return false;
}

bool WorldClientConnection::_handleZoneChangePacket(const EQApplicationPacket* pPacket) {
	// HoT sends this to world while zoning and wants it echoed back.
	if(ClientVersionBit & BIT_RoFAndLater)
	{
		_queuePacket(pPacket);
	}
	return true;
}

bool WorldClientConnection::_handlePacket(const EQApplicationPacket* pPacket) {
	EXPECTED_BOOL(pPacket);

	// Check if unidentified and sending something other than OP_SendLoginInfo
	// NOTE: Many functions called below assume getIdentified is checked here so do not remove it.
	if (!getAuthenticated() && pPacket->GetOpcode() != OP_SendLoginInfo) {
		Log::error("Unidentified Client sent %s, expected OP_SendLoginInfo"); //OpcodeNames[opcode]
		return false;
	}

	EmuOpcode opcode = pPacket->GetOpcode();
	switch(opcode) {
		case OP_AckPacket:
		case OP_World_Client_CRC1:
		case OP_World_Client_CRC2:
			return true;
		case OP_SendLoginInfo:
			// NOTE: Sent when Client initially connects (Moving from Server Selection to Character Selection).
			// NOTE: Sent when Client is moving from one zone to another.
			return _handleSendLoginInfoPacket(pPacket);
		case OP_ApproveName:
			return _handleNameApprovalPacket(pPacket);
		case OP_RandomNameGenerator:
			return _handleGenerateRandomNamePacket(pPacket);
		case OP_CharacterCreateRequest:
			// SoF+ Sends this when the user clicks 'Create a New Character' at the Character Select Screen.
			return _handleCharacterCreateRequestPacket(pPacket);
		case OP_CharacterCreate:
			return _handleCharacterCreatePacket(pPacket);
		case OP_EnterWorld:
			return _handleEnterWorldPacket(pPacket);
		case OP_DeleteCharacter:
			return _handleDeleteCharacterPacket(pPacket);
		case OP_WorldComplete:
			mStreamInterface->Close();
			return true;
		case OP_ZoneChange:
			// HoT sends this to world while zoning and wants it echoed back.
			return _handleZoneChangePacket(pPacket);
		case OP_LoginUnknown1:
		case OP_LoginUnknown2:
		case OP_CrashDump:
		case OP_WearChange:
		case OP_LoginComplete:
		case OP_ApproveWorld:
		case OP_WorldClientReady:
			return true;
		default:
			// Ignore.
			//Log::error("[World Client Connection] Got unexpected packet, ignoring.");
			return true;
	}
	return true;
}

bool WorldClientConnection::update() {
	// Check our connection.
	if (mConnectionDropped || !mStreamInterface->CheckState(ESTABLISHED)) {
		Utility::print("WorldClientConnection Lost.");
		return false;
	}

	bool ret = true;

	// Handle any incoming packets.
	EQApplicationPacket* packet = 0;
	while(ret && (packet = (EQApplicationPacket *)mStreamInterface->PopPacket())) {
		ret = _handlePacket(packet);
		delete packet;
	}

	return ret;
}

void WorldClientConnection::_sendZoneServerInfo(const uint16 pPort) {
	auto outPacket = new EQApplicationPacket(OP_ZoneServerInfo, sizeof(ZoneServerInfo_Struct));
	auto payload = reinterpret_cast<ZoneServerInfo_Struct*>(outPacket->pBuffer);
	payload->port = pPort;
	strcpy(payload->ip, "127.0.0.1");
	_queuePacket(outPacket);
	safe_delete(outPacket);
}


void WorldClientConnection::_sendZoneUnavailable() {
	// NOTE: In UF it doesnt matter what 'zone name' is sent back.
	auto outPacket = new EQApplicationPacket(OP_ZoneUnavail, sizeof(ZoneUnavail_Struct));
	auto payload = reinterpret_cast<ZoneUnavail_Struct*>(outPacket->pBuffer);
	strcpy(payload->zonename, "NONE");
	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_queuePacket(const EQApplicationPacket* pPacket, bool ack_req) {
	ack_req = true;	// It's broke right now, dont delete this line till fix it. =P
	mStreamInterface->QueuePacket(pPacket, ack_req);
}

void WorldClientConnection::_sendGuildList() {
	auto outPacket = new EQApplicationPacket(OP_GuildsList);
	outPacket->size = Limits::Guild::MAX_NAME_LENGTH + (Limits::Guild::MAX_NAME_LENGTH * Limits::Guild::MAX_GUILDS); // TODO: Work out the minimum sized packet UF will accept.
	outPacket->pBuffer = reinterpret_cast<unsigned char*>(GuildManager::getInstance()._getGuildNames());

	mStreamInterface->QueuePacket(outPacket);
	outPacket->pBuffer = nullptr;
	safe_delete(outPacket);
}

// @merth: I have no idea what this struct is for, so it's hardcoded for now
void WorldClientConnection::_sendApproveWorld() {
	// Send OPCode: OP_ApproveWorld, size: 544
	 auto outPacket = new EQApplicationPacket(OP_ApproveWorld, sizeof(ApproveWorld_Struct));
	 auto payload = reinterpret_cast<ApproveWorld_Struct*>(outPacket->pBuffer);
	uchar foo[] = {
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x95,0x5E,0x30,0xA5,0xCA,0xD4,0xEA,0xF5,
//0xCB,0x14,0xFC,0xF7,0x78,0xE2,0x73,0x15,0x90,0x17,0xCE,0x7A,0xEB,0xEC,0x3C,0x34,
//0x5C,0x6D,0x10,0x05,0xFC,0xEA,0xED,0x19,0xC5,0x0D,0x7A,0x82,0x17,0xCC,0xCC,0x71,
//0x56,0x38,0xDF,0x78,0x8D,0xE6,0x44,0xD3,0x6F,0xDB,0xE3,0xCF,0x21,0x30,0x75,0x2F,
//0xCD,0xDC,0xE9,0xB4,0xA4,0x4E,0x58,0xDE,0xEE,0x54,0xDD,0x87,0xDA,0xE9,0xC6,0xC8,
//0x02,0xDD,0xC4,0xFD,0x94,0x36,0x32,0xAD,0x1B,0x39,0x0F,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x87,0x13,0xbe,0xc8,0xa7,0x77,0xcb,
0x27,0xed,0xe1,0xe6,0x5d,0x1c,0xaa,0xd3,0x3c,0x26,0x3b,0x6d,0x8c,0xdb,0x36,0x8d,
0x91,0x72,0xf5,0xbb,0xe0,0x5c,0x50,0x6f,0x09,0x6d,0xc9,0x1e,0xe7,0x2e,0xf4,0x38,
0x1b,0x5e,0xa8,0xc2,0xfe,0xb4,0x18,0x4a,0xf7,0x72,0x85,0x13,0xf5,0x63,0x6c,0x16,
0x69,0xf4,0xe0,0x17,0xff,0x87,0x11,0xf3,0x2b,0xb7,0x73,0x04,0x37,0xca,0xd5,0x77,
0xf8,0x03,0x20,0x0a,0x56,0x8b,0xfb,0x35,0xff,0x59,0x00,0x00,0x00,0x00,0x00,0x00,

//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x42,0x69,0x2a,0x87,0xdd,0x04,0x3d,
//0x7f,0xb1,0xb3,0xbb,0xde,0xd5,0x5f,0xfc,0x1f,0xb3,0x25,0x94,0x16,0xd5,0xf3,0x97,
//0x43,0xdf,0xb9,0x69,0x68,0xdf,0x2b,0x64,0x98,0xf5,0x44,0xbe,0x38,0x65,0xef,0xff,
//0x36,0x89,0x90,0xcf,0x26,0xbb,0x9f,0x76,0xd5,0xaf,0x6d,0xf2,0x08,0xbe,0xce,0xd8,
//0x3e,0x4b,0x53,0x8a,0xf3,0x44,0x7c,0x19,0x49,0x5d,0x97,0x99,0xd8,0x8b,0xee,0x10,
//0x1a,0x7d,0xb7,0x8b,0x49,0x9b,0x40,0x8c,0xea,0x49,0x09,0x00,0x00,0x00,0x00,0x00,
//
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x15,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53,0xC3,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00
};
	memcpy(payload->unknown544, foo, sizeof(foo));
	_queuePacket(outPacket);
	safe_delete(outPacket);
}
