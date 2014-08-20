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

#include "ClientAuthentication.h"
#include <string>
#include "../common/linked_list.h"
#include "../common/timer.h"
#include "../common/logsys.h"
#include "../common/eq_packet_structs.h"

class EQApplicationPacket;
class EQStreamInterface;
class World;

class WorldClientConnection {
public:
	WorldClientConnection(EQStreamInterface* pStreamInterface, World* pWorld);
	~WorldClientConnection();

	bool update();

	bool getAuthenticated() { return mAuthenticated; }
	uint32 getIP() { return mIP; }
	uint16 getPort() { return mPort; }

	void setWorldAccountID(uint32 pWorldAccountID) { mWorldAccountID = pWorldAccountID; mAuthentication.mWorldAccountID = pWorldAccountID; }
	void setLoginServerAccountID(uint32 pLoginServerAccountID) { mLoginServerAccountID = pLoginServerAccountID; mAuthentication.mLoginServerAccountID = pLoginServerAccountID; }
	void setLoginServerKey(String pLoginServerKey) { mLoginServerKey = pLoginServerKey; mAuthentication.mKey = pLoginServerKey; }
	void setLoginServerAccountName(String pLoginServerAccountName) { mLoginServerAccountName = pLoginServerAccountName; mAuthentication.mLoginServerAccountName = pLoginServerAccountName; }
	void _setAuthenticated(bool pIdentified) { mAuthenticated = pIdentified; } // This method should only ever be called by World::checkAuthentication
private:
	
	void _queuePacket(const EQApplicationPacket* app, bool ack_req = true);

	void _sendCharacterSelectInfo();
	void _sendGuildList();
	void _sendEnterWorld(String pCharacterName);
	void _sendExpansionInfo();
	void _sendLogServer();
	void _sendApproveWorld();
	void _sendPostEnterWorld();
	void _sendZoneUnavailable();
	void _sendZoneServerInfo(uint16 pPort);

	bool mAuthenticated;
	ClientAuthentication mAuthentication;
	uint16 mPort;
	uint32 mIP;
	uint32 mWorldAccountID;
	uint32 mLoginServerAccountID;
	String mLoginServerAccountName;
	String mLoginServerKey;
	uint32 mCharacterID;
	String mReservedCharacterName;
	bool mConnectionDropped;
	
	
	
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
	void dropConnection() { mConnectionDropped = true; }
	EQStreamInterface* const mStreamInterface;
	World* mWorld;
};

#endif
