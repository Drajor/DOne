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
#include "LoginServerList.h"
#include "LoginServer.h"
#include "../common/servertalk.h"

extern LoginServerList loginserverlist;

#ifdef _WINDOWS
void AutoInitLoginServer(void *tmp) {
#else
void *AutoInitLoginServer(void *tmp) {
#endif
	loginserverlist.InitLoginServer();
#ifndef WIN32
	return 0;
#endif
}

LoginServerList::LoginServerList() { }
LoginServerList::~LoginServerList() { }

void LoginServerList::Add(const char* iAddress, uint16 iPort, const char* Account, const char* Password) {
	mLoginServers.push_back(new LoginServer(iAddress, iPort, Account, Password));
}

bool LoginServerList::Process() {
	for (auto i : mLoginServers) {
		i->Process();
	}
	return true;
}

void LoginServerList::InitLoginServer() {
	for (auto i : mLoginServers) {
		i->InitLoginServer();
	}
}

bool LoginServerList::SendStatus() {
	for (auto i : mLoginServers) {
		i->SendStatus();
	}
	return true;
}

bool LoginServerList::SendPacket(ServerPacket* pack) {
	for (auto i : mLoginServers) {
		i->SendPacket(pack);
	}
	return true;
}

bool LoginServerList::Connected() {
	for (auto i : mLoginServers) {
		if (i->Connected()) return true;
	}
	return false;
}

bool LoginServerList::AllConnected() {
	for (auto i : mLoginServers) {
		if (!i->Connected()) return false;
	}
	return true;
}