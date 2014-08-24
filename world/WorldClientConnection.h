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
	inline uint32 getIP() { return mIP; }
	inline uint16 getPort() { return mPort; }

	inline void setAccountID(const uint32 pAccountID) { mAccountID = pAccountID; mAuthentication.mLoginServerAccountID = pAccountID; }
	inline void setKey(const String& pKey) { mKey = pKey; mAuthentication.mKey = pKey; }
	inline void setAccountName(const String& pAccountName) { mAccountName = pAccountName; mAuthentication.mLoginServerAccountName = pAccountName; }

	void _setAuthenticated(bool pIdentified) { mAuthenticated = pIdentified; }

	void _sendZoneServerInfo(const uint16 pPort);
	uint32 getAccountID(){ return mAccountID; }
private:

	inline void dropConnection() { mConnectionDropped = true; }
	
	void _queuePacket(const EQApplicationPacket* app, bool ack_req = true);

	void _sendCharacterSelectInfo();
	void _sendGuildList();
	void _sendEnterWorld(String pCharacterName);
	void _sendExpansionInfo();
	void _sendLogServer();
	void _sendApproveWorld();
	void _sendPostEnterWorld();
	void _sendZoneUnavailable();

	bool mAuthenticated = false;
	ClientAuthentication mAuthentication;
	uint16 mPort = 0;
	uint32 mIP = 0;
	uint32 mAccountID = 0;
	String mAccountName = "";
	String mKey = "";
	String mReservedCharacterName = "";
	bool mConnectionDropped = false;
	
	char	char_name[64];
	bool mZoning;
	uint32 ClientVersionBit = 0;

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
