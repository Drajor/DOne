#include "WorldClientConnection.h"

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

#include "WorldConfig.h"

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
#include "Constants.h"
#include "LogSystem.h"

#ifdef _WINDOWS
	#include <windows.h>
	#include <winsock.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
#endif

WorldClientConnection::WorldClientConnection(EQStreamInterface* pStreamInterface, World* pWorld) :
	mStreamInterface(pStreamInterface),
	mWorld(pWorld),
	mIdentified(false),
	mReservedCharacterName("")
{
	mIP = mStreamInterface->GetRemoteIP();
	mPort = ntohs(mStreamInterface->GetRemotePort());

	zoneID = 0;
	char_name[0] = 0;
	mCharacterID = 0;
	ClientVersionBit = 0;

	ClientVersionBit = 1 << (mStreamInterface->ClientVersion() - 1);
}

WorldClientConnection::~WorldClientConnection() {
	//let the stream factory know were done with this stream
	mStreamInterface->Close();
	mStreamInterface->ReleaseFromUse();
}

void WorldClientConnection::_sendLogServer() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_LogServer, sizeof(LogServer_Struct));
	LogServer_Struct* payload = reinterpret_cast<LogServer_Struct*>(outPacket->pBuffer);
	const char *wsn=WorldConfig::get()->ShortName.c_str();
	memcpy(payload->worldshortname,wsn,strlen(wsn));

	if(RuleB(Mail, EnableMailSystem))
		payload->enablemail = 1;

	if(RuleB(Chat, EnableVoiceMacros))
		payload->enablevoicemacros = 1;

	payload->enable_pvp = (RuleI(World, PVPSettings));

	if(RuleB(World, IsGMPetitionWindowEnabled))
		payload->enable_petition_wnd = 1;

	payload->enable_FV = 0;

	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_sendEnterWorld(std::string pCharacterName) {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_EnterWorld, pCharacterName.length() + 1);
	memcpy(outPacket->pBuffer, pCharacterName.c_str(), pCharacterName.length() + 1);
	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_sendExpansionInfo() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ExpansionInfo, sizeof(ExpansionInfo_Struct));
	ExpansionInfo_Struct* payload = reinterpret_cast<ExpansionInfo_Struct*>(outPacket->pBuffer);
	payload->Expansions = (RuleI(World, ExpansionSettings));
	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_sendCharacterSelectInfo() {
	if (ClientVersionBit & BIT_RoFAndLater)
	{
		// Can make max char per account into a rule - New to VoA
		_sendMaxCharCreate(10);
		_sendMembership();
		_sendMembershipSettings();
	}

	// Send OP_SendCharInfo
	EQApplicationPacket *outPacket = new EQApplicationPacket(OP_SendCharInfo, sizeof(CharacterSelect_Struct));
	CharacterSelect_Struct* payload = reinterpret_cast<CharacterSelect_Struct*>(outPacket->pBuffer);
	mWorld->getCharacterSelectInfo(mWorldAccountID, payload);
	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_sendMaxCharCreate(int max_chars) {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SendMaxCharacters, sizeof(MaxCharacters_Struct));
	MaxCharacters_Struct* payload = reinterpret_cast<MaxCharacters_Struct*>(outPacket->pBuffer);

	payload->max_chars = max_chars;

	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_sendMembership() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SendMembership, sizeof(Membership_Struct));
	Membership_Struct* payload = reinterpret_cast<Membership_Struct*>(outPacket->pBuffer);

	/*
		The remaining entry fields probably hold more membership restriction data that needs to be identified.
		Here is a possible list based on the EQ Website membership comparison list:
		1. Spell Ranks Allowed
		2. Prestige Items Usable
		3. In-Game Mail Service (send/recieve)
		4. Parcel Delivery (send/recieve)
		5. Loyalty Rewards Per-Week (30, 60, Max)
		6. Mercenary Tiers (Apprentice 1-2, Apprentice All Tiers, All Tiers)
		7. Neighborhood House
		8. Active Journal Quests (Tasks?) (10, 15, Max)
		9. Guild Function (join, join and create)
		10. Broker System (restricted, unlimited)
		11. Voice Chat
		12. Chat Ability
		13. Progression Server
		14. Customer Support
		15. In-Game Popup Advertising
		That is 15 possible fields, and there are 15 unknowns in the struct...Coincidence?
	*/

	payload->membership = 2;				//Hardcode to gold for now. We don't use anything else.
	payload->races = 0x1ffff;			// Available Races (4110 for silver)
	payload->classes = 0x1ffff;			// Available Classes (4614 for silver) - Was 0x101ffff
	payload->entrysize = 21;				// Number of membership setting entries below
	payload->entries[0] = 0xffffffff;	// Max AA Restriction
	payload->entries[1] = 0xffffffff;	// Max Level Restriction
	payload->entries[2] = 0xffffffff;	// Max Char Slots per Account (not used by client?)
	payload->entries[3] = 0xffffffff;	// 1 for Silver
	payload->entries[4] = 8;				// Main Inventory Size (0xffffffff on Live for Gold, but limitting to 8 until 10 is supported)
	payload->entries[5] = 0xffffffff;	// Max Platinum per level
	payload->entries[6] = 1;				// 0 for Silver
	payload->entries[7] = 1;				// 0 for Silver
	payload->entries[8] = 1;				// 1 for Silver
	payload->entries[9] = 0xffffffff;	// Unknown - Maybe Loyalty Points every 12 hours? 60 per week for Silver
	payload->entries[10] = 1;			// 1 for Silver
	payload->entries[11] = 0xffffffff;	// Shared Bank Slots
	payload->entries[12] = 0xffffffff;	// Unknown - Maybe Max Active Tasks?
	payload->entries[13] = 1;			// 1 for Silver
	payload->entries[14] = 1;			// 0 for Silver
	payload->entries[15] = 1;			// 0 for Silver
	payload->entries[16] = 1;			// 1 for Silver
	payload->entries[17] = 1;			// 0 for Silver
	payload->entries[18] = 1;			// 0 for Silver
	payload->entries[19] = 0xffffffff;	// 0 for Silver
	payload->entries[20] = 0xffffffff;	// 0 for Silver
	payload->exit_url_length = 0;
	//mc->exit_url = 0; // Used on Live: "http://www.everquest.com/free-to-play/exit-silver"

	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_sendMembershipSettings() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SendMembershipDetails, sizeof(Membership_Details_Struct));
	Membership_Details_Struct* payload = reinterpret_cast<Membership_Details_Struct*>(outPacket->pBuffer);

	payload->membership_setting_count = 66;
	int32 gold_settings[22] = {-1,-1,-1,-1,-1,-1,1,1,1,-1,1,-1,-1,1,1,1,1,1,1,-1,-1,0};
	uint32 entry_count = 0;
	for (int setting_id=0; setting_id < 22; setting_id++)
	{
		for (int setting_index=0; setting_index < 3; setting_index++)
		{

			payload->settings[entry_count].setting_index = setting_index;
			payload->settings[entry_count].setting_id = setting_id;
			payload->settings[entry_count].setting_value = gold_settings[setting_id];
			entry_count++;
		}
	}

	payload->race_entry_count = 15;
	payload->class_entry_count = 15;

	uint32 cur_purchase_id = 90287;
	uint32 cur_purchase_id2 = 90301;
	uint32 cur_bitwise_value = 1;
	for (int entry_id=0; entry_id < 15; entry_id++)
	{
		if (entry_id == 0)
		{
			payload->membership_races[entry_id].purchase_id = 1;
			payload->membership_races[entry_id].bitwise_entry = 0x1ffff;
			payload->membership_classes[entry_id].purchase_id = 1;
			payload->membership_classes[entry_id].bitwise_entry = 0x1ffff;
		}
		else
		{
			payload->membership_races[entry_id].purchase_id = cur_purchase_id;

			if (entry_id < 3)
			{
				payload->membership_classes[entry_id].purchase_id = cur_purchase_id;
			}
			else
			{
				payload->membership_classes[entry_id].purchase_id = cur_purchase_id2;
				cur_purchase_id2++;
			}

			if (entry_id == 1)
			{
				payload->membership_races[entry_id].bitwise_entry = 4110;
				payload->membership_classes[entry_id].bitwise_entry = 4614;
			}
			else if (entry_id == 2)
			{
				payload->membership_races[entry_id].bitwise_entry = 4110;
				payload->membership_classes[entry_id].bitwise_entry = 4614;
			}
			else
			{
				if (entry_id == 12)
				{
					// Live Skips 4096
					cur_bitwise_value *= 2;
				}
				payload->membership_races[entry_id].bitwise_entry = cur_bitwise_value;
				payload->membership_classes[entry_id].bitwise_entry = cur_bitwise_value;
			}
			cur_purchase_id++;
		}
		cur_bitwise_value *= 2;
	}
	payload->exit_url_length = 0;	// Live uses 42
	//strcpy(eq->exit_url, "http://www.everquest.com/free-to-play/exit");
	payload->exit_url_length2 = 0;	// Live uses 49
	//strcpy(eq->exit_url2, "http://www.everquest.com/free-to-play/exit-silver");

	/*
	Account Access Level Settings

	ID	-	Free	Silver	Gold	-	Possible Setting
	00	-	250		1000	-1		-	Max AA Restriction
	01	-	-1		-1		-1		-	Max Level Restriction
	02	-	2		4		-1		-	Max Char Slots per Account
	03	-	1		1		-1		-	Max Spell Rank
	04	-	4		6		-1		-	Main Inventory Size
	05	-	100		500		-1		-	Max Platinum per level
	06	-	0		0		1		-	Send Mail?
	07	-	0		0		1		-	Send Parcels?
	08	-	1		1		1		-	Voice Chat Unlimited?
	09	-	2		5		-1		-	Mercenary Tiers
	10	-	0		1		1		-	Create Guilds?
	11	-	0		0		-1		-	Shared Bank Slots
	12	-	9		14		-1		-	Max Journal Quests - 1
	13	-	0		1		1		-	Neighborhood-House Allowed?
	14	-	0		0		1		-	Prestige Enabled?
	15	-	0		0		1		-	Broker System Unlimited?
	16	-	0		1		1		-	Chat UnRestricted?
	17	-	0		0		1		-	Progression Server Access?
	18	-	0		0		1		-	Full Customer Support?
	19	-	0		0		-1		-	0 for Silver
	20	-	0		0		-1		-	0 for Silver
	21	-	0		0		0		-	Unknown 0
	*/

	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_sendPostEnterWorld() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_PostEnterWorld, 1);
	outPacket->size = 0;
	_queuePacket(outPacket);
	safe_delete(outPacket);
}

bool WorldClientConnection::_handleSendLoginInfoPacket(const EQApplicationPacket* pPacket) {
	if (pPacket->size != sizeof(LoginInfo_Struct)) return false;

	/*
	OP_SendLoginInfo is sent;
		- Transitioning from Server Select to Character Select
		- Zoning
		- Camping
	*/

	LoginInfo_Struct* payload = reinterpret_cast<LoginInfo_Struct*>(pPacket->pBuffer);

	// Quagmire - max len for name is 18, pass 15
	char name[19] = {0};
	char key[16] = {0};
	strn0cpy(name, (char*)payload->login_info,18);
	strn0cpy(key, (char*)&(payload->login_info[strlen(name)+1]), 15);

	if (strlen(key) <= 1) {
		// TODO: Find out how to tell the client wrong username/password
		clog(WORLD__CLIENT_ERR,"Login without a password");
		return false;
	}

	mZoning = (payload->zoning == 1);

	uint32 id = atoi(name);

	if (!mWorld->isLoginServerConnected()) {
		clog(WORLD__CLIENT_ERR, "Error: Login server login while not connected to login server.");
		return false;
	}

	// This is first communication from client after Server Select
	if (!mIdentified) {
		if (mWorld->tryIdentify(this, id, key)) {
			/*
				OP_GuildsList, OP_LogServer, OP_ApproveWorld
				All sent in EQEmu but not actually required to get to Character Select. More research on the effects of not sending are required.
			*/
			_sendGuildList();
			_sendLogServer();
			_sendApproveWorld();
			_sendEnterWorld(""); // Empty character name when coming from Server Select. 
			_sendPostEnterWorld(); // Required.
			_sendExpansionInfo(); // Required.
			_sendCharacterSelectInfo(); // Required.
		}
		// TODO: Else drop this connection .. whatever is happening its bad.
	}
	
	//loginInfo->login_info->

	////cle = client_list.CheckAuth(id, password);
	//if (cle) {
	//	if (cle->AccountID() == 0) {
	//		return false;
	//	}

	//	cle->SetOnline();

	//	clog(WORLD__CLIENT,"Logged in. Mode=%s",pZoning ? "(Zoning)" : "(CharSel)");
	//	clog(WORLD__CLIENT, "LS Account #%d", cle->getLoginServerAccountID());

	//	if (!pZoning)
	//		SendGuildList();
	//	SendLogServer();
	//	SendApproveWorld();
	//	SendEnterWorld(cle->name());
	//	SendPostEnterWorld();
	//	if (!pZoning) {
	//		SendExpansionInfo();
	//		SendCharInfo();
	//		database.LoginIP(cle->AccountID(), long2ip(GetIP()).c_str());
	//	}

	//}
	//else {
	//	// TODO: Find out how to tell the client wrong username/password
	//	clog(WORLD__CLIENT_ERR,"Bad/Expired session key '%s'",name);
	//	return false;
	//}

	//cle->SetIP(GetIP());
	//cle->SetOnline();

	//clog(WORLD__CLIENT, "Logged in. Mode=%s", mZoning ? "(Zoning)" : "(CharSel)");
	//clog(WORLD__CLIENT, "LS Account #%d", cle->getLoginServerAccountID());

	//if (!mZoning)
	//	SendGuildList();
	//SendLogServer();
	//SendApproveWorld();
	//SendEnterWorld(cle->name());
	//SendPostEnterWorld();
	//if (!mZoning) {
	//	SendExpansionInfo();
	//	SendCharInfo();
	//	//database.LoginIP(cle->AccountID(), long2ip(GetIP()).c_str());
	//}

	return true;
}

bool WorldClientConnection::_handleNameApprovalPacket(const EQApplicationPacket* pPacket) {
	if (getWorldAccountID() == 0) {
		clog(WORLD__CLIENT_ERR,"Name approval request with no logged in account");
		return false;
	}

	snprintf(char_name, 64, "%s", (char*)pPacket->pBuffer);
	// TODO: Consider why race and class are sent here?
	uchar race = pPacket->pBuffer[64];
	uchar clas = pPacket->pBuffer[68];

	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ApproveName, 1);
	bool valid = true;
	std::string characterName = char_name;
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
	if (valid && !mWorld->isCharacterNameUnique(characterName)) {
		valid = false;
	}
	// Check if name is reserved.
	if (valid && mWorld->isCharacterNameReserved(characterName)) {
		valid = false;
	}
	// Reserve character name.
	if (valid) {
		// For the rare chance that more than one user tries to create a character with same name.
		mWorld->reserveCharacterName(mWorldAccountID, characterName);
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
		PlayableClassIDs::Warrior,
		PlayableClassIDs::Cleric,
		PlayableClassIDs::Wizard,
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

	EQApplicationPacket *outPacket = new EQApplicationPacket(OP_CharacterCreateRequest, len);
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
	if (pPacket->size != sizeof(CharCreate_Struct))
	{
		clog(WORLD__CLIENT_ERR,"Wrong size on OP_CharacterCreate. Got: %d, Expected: %d",pPacket->size,sizeof(CharCreate_Struct));
		DumpPacket(pPacket);
		// the previous behavior was essentially returning true here
		// but that seems a bit odd to me.
		return true;
	}

	// If character creation fails we just dump the client. As far as I can tell there is no nice way of handling failure here.
	CharCreate_Struct* payload = reinterpret_cast<CharCreate_Struct*>(pPacket->pBuffer);
	if (!mWorld->createCharacter(mWorldAccountID, mReservedCharacterName, payload)) {
		// TODO: Dump client.
	}

	mReservedCharacterName = "";
	return true;
}

bool WorldClientConnection::_handleEnterWorldPacket(const EQApplicationPacket* pPacket) {
	// Note: Enter Tutorial and Return Home are ignored at the moment.

	// Sent after Character Creation.
	// Sent after pressing 'Enter World' at Character Select.
	// Is it sent while zoning?

	EnterWorld_Struct* payload = reinterpret_cast<EnterWorld_Struct*>(pPacket->pBuffer);
	std::string characterName = payload->name;

	// Check: Character belongs to this account. This also checks whether the character actually exists.
	if (mWorld->isWorldEntryAllowed(mWorldAccountID, characterName)){
		// Send MOTD?
		// Send ChatServer?
		// Send ChatServer2?
		// OP_ZoneServerInfo
		//_sendZoneServerInfo(); // /wave
		_sendZoneUnavailable();
		return true;
	}

	Log::error("World refused entry! Account: %i, Character: %s"); // TODO:
	mStreamInterface->Close();
	return false;


	//if (zoneID == 0 || !database.GetZoneName(zoneID)) {
	//	// This is to save people in an invalid zone, once it's removed from the DB
	//	database.MoveCharacterToZone(mCharacterID, "arena");
	//	clog(WORLD__CLIENT_ERR, "Zone not found in database zone_id=%i, moveing char to arena character:%s", zoneID, char_name);
	//}

	//if(instanceID > 0)
	//{
	//	if(!database.VerifyInstanceAlive(instanceID, getCharacterID()))
	//	{
	//		zoneID = database.MoveCharacterToBind(mCharacterID);
	//		instanceID = 0;
	//	}
	//	else
	//	{
	//		if(!database.VerifyZoneInstance(zoneID, instanceID))
	//		{
	//			zoneID = database.MoveCharacterToBind(mCharacterID);
	//			instanceID = 0;
	//		}
	//	}
	//}

	//if(!mZoning) {
	//	database.SetGroupID(char_name, 0, mCharacterID);
	//	database.SetLoginFlags(mCharacterID, false, false, 1);
	//}
	//else{
	//	uint32 groupid=database.GetGroupID(char_name);
	//	if(groupid>0){
	//		char* leader=0;
	//		char leaderbuf[64]={0};
	//		if((leader=database.GetGroupLeaderForLogin(char_name,leaderbuf)) && strlen(leader)>1){
	//			EQApplicationPacket* outapp3 = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
	//			GroupJoin_Struct* gj=(GroupJoin_Struct*)outapp3->pBuffer;
	//			gj->action=8;
	//			strcpy(gj->yourname,char_name);
	//			strcpy(gj->membername,leader);
	//			QueuePacket(outapp3);
	//			safe_delete(outapp3);
	//		}
	//	}
	//}

	//outapp = new EQApplicationPacket(OP_MOTD);
	//char tmp[500] = {0};
	//if (database.GetVariable("MOTD", tmp, 500)) {
	//	outapp->size = strlen(tmp)+1;
	//	outapp->pBuffer = new uchar[outapp->size];
	//	memset(outapp->pBuffer,0,outapp->size);
	//	strcpy((char*)outapp->pBuffer, tmp);

	//} else {
	//	// Null Message of the Day. :)
	//	outapp->size = 1;
	//	outapp->pBuffer = new uchar[outapp->size];
	//	outapp->pBuffer[0] = 0;
	//}
	//QueuePacket(outapp);
	//safe_delete(outapp);

	//int MailKey = MakeRandomInt(1, INT_MAX);

	//database.SetMailKey(mCharacterID, getIP(), MailKey);

	//char ConnectionType;

	//if(ClientVersionBit & BIT_UnderfootAndLater)
	//	ConnectionType = 'U';
	//else if(ClientVersionBit & BIT_SoFAndLater)
	//	ConnectionType = 'S';
	//else
	//	ConnectionType = 'C';

	//EQApplicationPacket *outapp2 = new EQApplicationPacket(OP_SetChatServer);
	//char buffer[112];

	//const WorldConfig *Config = WorldConfig::get();

	//sprintf(buffer,"%s,%i,%s.%s,%c%08X",
	//	Config->ChatHost.c_str(),
	//	Config->ChatPort,
	//	Config->ShortName.c_str(),
	//	this->GetCharName(), ConnectionType, MailKey
	//);
	//outapp2->size=strlen(buffer)+1;
	//outapp2->pBuffer = new uchar[outapp2->size];
	//memcpy(outapp2->pBuffer,buffer,outapp2->size);
	//QueuePacket(outapp2);
	//safe_delete(outapp2);

	//outapp2 = new EQApplicationPacket(OP_SetChatServer2);

	//if(ClientVersionBit & BIT_TitaniumAndEarlier)
	//	ConnectionType = 'M';

	//sprintf(buffer,"%s,%i,%s.%s,%c%08X",
	//	Config->MailHost.c_str(),
	//	Config->MailPort,
	//	Config->ShortName.c_str(),
	//	this->GetCharName(), ConnectionType, MailKey
	//);
	//outapp2->size=strlen(buffer)+1;
	//outapp2->pBuffer = new uchar[outapp2->size];
	//memcpy(outapp2->pBuffer,buffer,outapp2->size);
	//QueuePacket(outapp2);
	//safe_delete(outapp2);

	//_sendEnterWorldSomethingElseThatNeedsRenamingBadlyFoo();

	return true;
}

bool WorldClientConnection::_handleDeleteCharacterPacket(const EQApplicationPacket* pPacket) {
	// TODO: Check character is not in zone before deleting..
	std::string characterName = (char*)pPacket->pBuffer;
	if (mWorld->deleteCharacter(mWorldAccountID, characterName)) {
		Log::info("Delete Character: %s"); // TODO:
		_sendCharacterSelectInfo();
	}
	// TODO: Log Error / Drop connection.
	return true;
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
	// Check our Stream Interface.
	if (!mStreamInterface->CheckState(ESTABLISHED)) {
		// TODO: Can this even occur?
		Log::error("Client disconnected (net inactive on send)");
		return false;
	}
	// Check if unidentified and sending something other than OP_SendLoginInfo
	// NOTE: Many functions called below assume getIdentified is checked here so do not remove it.
	if (!getIdentified() && pPacket->GetOpcode() != OP_SendLoginInfo) {
		Log::error("Unidentified Client sent %s, expected OP_SendLoginInfo"); //OpcodeNames[opcode]
		return false;
	}

	EmuOpcode opcode = pPacket->GetOpcode();
	switch(opcode)
	{
		case OP_AckPacket:
		case OP_World_Client_CRC1:
		case OP_World_Client_CRC2:
		{
			return true;
		}
		case OP_SendLoginInfo:
		{
			return _handleSendLoginInfoPacket(pPacket);
		}
		case OP_ApproveName: //Name approval
		{
			return _handleNameApprovalPacket(pPacket);
		}
		case OP_RandomNameGenerator:
		{
			return _handleGenerateRandomNamePacket(pPacket);
		}
		case OP_CharacterCreateRequest:
		{
			// SoF+ Sends this when the user clicks 'Create a New Character' at the Character Select Screen.
			return _handleCharacterCreateRequestPacket(pPacket);
		}
		case OP_CharacterCreate: //Char create
		{
			return _handleCharacterCreatePacket(pPacket);
		}
		case OP_EnterWorld: // Enter world
		{
								Utility::print("OP_EnterWorld");
			return _handleEnterWorldPacket(pPacket);
		}
		case OP_DeleteCharacter:
		{
			return _handleDeleteCharacterPacket(pPacket);
		}
		case OP_WorldComplete:
		{
			mStreamInterface->Close();
			return true;
		}
		case OP_ZoneChange:
		{
			// HoT sends this to world while zoning and wants it echoed back.
			return _handleZoneChangePacket(pPacket);
		}
		case OP_LoginUnknown1:
		case OP_LoginUnknown2:
		case OP_CrashDump:
		case OP_WearChange:
		case OP_LoginComplete:
		case OP_ApproveWorld:
		case OP_WorldClientReady:
		{
			// Essentially we are just 'eating' these packets, indicating
			// they are handled.
			return true;
		}
		default:
		{
			clog(WORLD__CLIENT_ERR,"Received unknown EQApplicationPacket");
			_pkt(WORLD__CLIENT_ERR,pPacket);
			return true;
		}
	}
	return true;
}

bool WorldClientConnection::update() {
	// Check our connection to the User.
	if (!mStreamInterface->CheckState(ESTABLISHED)) {
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

void WorldClientConnection::_sendZoneServerInfo() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ZoneServerInfo, sizeof(ZoneServerInfo_Struct));
	ZoneServerInfo_Struct* payload = reinterpret_cast<ZoneServerInfo_Struct*>(outPacket->pBuffer);
	payload->port = 7000;
	strcpy(payload->ip, "127.0.0.1");
	_queuePacket(outPacket);
	safe_delete(outPacket);
}


void WorldClientConnection::_sendZoneUnavailable() {
	// NOTE: In HoT it doesnt matter what 'zone name' is sent back.
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ZoneUnavail, sizeof(ZoneUnavail_Struct));
	ZoneUnavail_Struct* payload = reinterpret_cast<ZoneUnavail_Struct*>(outPacket->pBuffer);
	strcpy(payload->zonename, "NONE");
	_queuePacket(outPacket);
	safe_delete(outPacket);
}

void WorldClientConnection::_queuePacket(const EQApplicationPacket* pPacket, bool ack_req) {
	clog(WORLD__CLIENT_TRACE, "Sending EQApplicationPacket OpCode 0x%04x",pPacket->GetOpcode());
	_pkt(WORLD__CLIENT_TRACE, pPacket);

	ack_req = true;	// It's broke right now, dont delete this line till fix it. =P
	mStreamInterface->QueuePacket(pPacket, ack_req);
}

void WorldClientConnection::_sendGuildList() {
//	EQApplicationPacket *outapp;
//	outapp = new EQApplicationPacket(OP_GuildsList);
//
//	//ask the guild manager to build us a nice guild list packet
//	outapp->pBuffer = guild_mgr.makeGuildList("", outapp->size);
//	if(outapp->pBuffer == nullptr) {
//		clog(GUILDS__ERROR, "Unable to make guild list!");
//		return;
//	}
//
//	clog(GUILDS__OUT_PACKETS, "Sending OP_GuildsList of length %d", outapp->size);
////	_pkt(GUILDS__OUT_PACKET_TRACE, outapp);
//
//	eqs->FastQueuePacket((EQApplicationPacket **)&outapp);
}

// @merth: I have no idea what this struct is for, so it's hardcoded for now
void WorldClientConnection::_sendApproveWorld() {
	// Send OPCode: OP_ApproveWorld, size: 544
	 EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ApproveWorld, sizeof(ApproveWorld_Struct));
	 ApproveWorld_Struct* payload = reinterpret_cast<ApproveWorld_Struct*>(outPacket->pBuffer);
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