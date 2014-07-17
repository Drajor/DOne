/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "../common/linked_list.h"
#include "../common/timer.h"
#include "../common/logsys.h"
#include "../common/eq_packet_structs.h"

#define CLIENT_TIMEOUT 30000

class EQApplicationPacket;
class EQStreamInterface;
class World;

/*
TODO: Work out when to drop a WorldClientConnection. It looks like Zone previously had a role in this.
*/

class WorldClientConnection {
public:
	WorldClientConnection(EQStreamInterface* ieqs, World* pWorld);
	~WorldClientConnection();

	bool update();

	bool getIdentified() { return mIdentified; }
	uint32 getIP() { return mIP; }
	uint16 getPort() { return mPort; }
	uint32 getWorldAccountID() { return mWorldAccountID; }
	uint32 getLoginServerAccountID() { return mLoginServerAccountID; }
	std::string getLoginServerKey() { return mLoginServerKey; }
	uint32 getCharacterID() { return mCharacterID; }

	void setWorldAccountID(uint32 pWorldAccountID) { mWorldAccountID = pWorldAccountID; }
	void setLoginServerAccountID(uint32 pLoginServerAccountID) { mLoginServerAccountID = pLoginServerAccountID; }
	void setLoginServerKey(std::string pLoginServerKey) { mLoginServerKey = pLoginServerKey; }
	void setLoginServerAccountName(std::string pLoginServerAccountName) { mLoginServerAccountName = pLoginServerAccountName; }
	void _setIdentified(bool pIdentified) { mIdentified = pIdentified; } // This method should only ever be called by World::tryIdentify
private:
	
	void _queuePacket(const EQApplicationPacket* app, bool ack_req = true);

	void _sendCharacterSelectInfo();
	void _sendMaxCharCreate(int max_chars);
	void _sendMembership();
	void _sendMembershipSettings();
	void _sendGuildList();
	void _sendEnterWorld(std::string pCharacterName);
	void _sendExpansionInfo();
	void _sendLogServer();
	void _sendApproveWorld();
	void _sendPostEnterWorld();
	void _sendZoneUnavailable();
	void _sendZoneServerInfo();

	bool mIdentified;
	uint16 mPort;
	uint32 mIP;
	uint32 mWorldAccountID;
	uint32 mLoginServerAccountID;
	std::string mLoginServerAccountName;
	std::string mLoginServerKey;
	uint32 mCharacterID;
	std::string mReservedCharacterName;
	
	
	
	char	char_name[64];
	uint32	zoneID;
	uint32	instanceID;
	bool	mZoning;
	uint32 ClientVersionBit;

	bool _handlePacket(const EQApplicationPacket* pPacket);
	bool _handleGenerateRandomNamePacket(const EQApplicationPacket* pPacket);
	bool _handleCharacterCreatePacket(const EQApplicationPacket* pPacket);
	bool _handleEnterWorldPacket(const EQApplicationPacket* pPacket);
	bool _handleDeleteCharacterPacket(const EQApplicationPacket* pPacket);
	bool _handleZoneChangePacket(const EQApplicationPacket* pPacket);
	bool _handleSendLoginInfoPacket(const EQApplicationPacket* packet);
	bool _handleCharacterCreateRequestPacket(const EQApplicationPacket* packet);
	bool _handleNameApprovalPacket(const EQApplicationPacket* packet);

	EQStreamInterface* const mStreamInterface;
	World* mWorld;
};

#endif
