/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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
#include "../common/debug.h"
#include "cliententry.h"
#include "clientlist.h"
#include "worlddb.h"
#include "zoneserver.h"
#include "WorldConfig.h"
#include "../common/guilds.h"
#include "../common/StringUtil.h"

extern uint32 numplayers;
extern ClientList		client_list;
extern volatile bool RunLoops;

ClientListEntry::ClientListEntry(uint32 in_id, uint32 iLSID, const char* iLoginName, const char* iLoginKey, int16 iWorldAdmin, uint32 ip, uint8 local)
: id(in_id)
{
	ClearVars(true);

	mIP = ip;
	mLoginServerID = iLSID;
	if(iLSID > 0)
		paccountid = database.GetAccountIDFromLSID(iLSID, paccountname, &padmin);
	strn0cpy(mLoginServerAccountName, iLoginName, sizeof(mLoginServerAccountName));
	strn0cpy(mLoginServerKey, iLoginKey, sizeof(mLoginServerKey));
	mWorldAdminStatus = iWorldAdmin;
	plocal=(local==1);

	pinstance = 0;
}

ClientListEntry::~ClientListEntry() {
	if (RunLoops) {
		Camp(); // updates zoneserver's numplayers
		client_list.RemoveCLEReferances(this);
	}
}

void ClientListEntry::SetChar(uint32 iCharID, const char* iCharName) {
	pcharid = iCharID;
	strn0cpy(pname, iCharName, sizeof(pname));
}

void ClientListEntry::SetOnline(ZoneServer* iZS, int8 iOnline) {
	if (iZS == this->Server())
		SetOnline(iOnline);
}

void ClientListEntry::SetOnline(int8 iOnline) {
	if (iOnline >= CLE_Status_Online && mOnline < CLE_Status_Online)
		numplayers++;
	else if (iOnline < CLE_Status_Online && mOnline >= CLE_Status_Online) {
		numplayers--;
	}
	if (iOnline != CLE_Status_Online || mOnline < CLE_Status_Online)
		mOnline = iOnline;
	if (iOnline < CLE_Status_Zoning)
		Camp();
	if (mOnline >= CLE_Status_Online)
		mStale = 0;
}

void ClientListEntry::Update(ZoneServer* iZS, ServerClientList_Struct* scl, int8 iOnline) {
	if (pzoneserver != iZS) {
		if (pzoneserver){
			pzoneserver->RemovePlayer();
		}
		if (iZS){
			iZS->AddPlayer();
		}
	}
	pzoneserver = iZS;
	pzone = scl->zone;
	pinstance = scl->instance_id;
	pcharid = scl->charid;

	strcpy(pname, scl->name);
	if (paccountid == 0) {
		paccountid = scl->AccountID;
		strcpy(paccountname, scl->AccountName);
		strcpy(mLoginServerAccountName, scl->AccountName);
		mIP = scl->IP;
		mLoginServerID = scl->LSAccountID;
		strn0cpy(mLoginServerKey, scl->lskey, sizeof(mLoginServerKey));
	}
	padmin = scl->Admin;
	plevel = scl->level;
	pclass_ = scl->class_;
	prace = scl->race;
	panon = scl->anon;
	ptellsoff = scl->tellsoff;
	pguild_id = scl->guild_id;
	pLFG = scl->LFG;
	gm = scl->gm;
	pClientVersion = scl->ClientVersion;

	// Fields from the LFG Window
	if((scl->LFGFromLevel != 0) && (scl->LFGToLevel != 0)) {
		pLFGFromLevel = scl->LFGFromLevel;
		pLFGToLevel = scl->LFGToLevel;
		pLFGMatchFilter = scl->LFGMatchFilter;
		memcpy(pLFGComments, scl->LFGComments, sizeof(pLFGComments));
	}

	SetOnline(iOnline);
}

void ClientListEntry::LeavingZone(ZoneServer* iZS, int8 iOnline) {
	if (iZS != 0 && iZS != pzoneserver)
		return;
	SetOnline(iOnline);

	if (pzoneserver){
		pzoneserver->RemovePlayer();
	}
	pzoneserver = 0;
	pzone = 0;
}

void ClientListEntry::ClearVars(bool iAll) {
	if (iAll) {
		mOnline = CLE_Status_Never;
		mStale = 0;

		mLoginServerID = 0;
		memset(mLoginServerAccountName, 0, sizeof(mLoginServerAccountName));
		memset(mLoginServerKey, 0, sizeof(mLoginServerKey));
		mWorldAdminStatus = 0;

		paccountid = 0;
		memset(paccountname, 0, sizeof(paccountname));
		padmin = 0;
	}
	pzoneserver = 0;
	pzone = 0;
	pcharid = 0;
	memset(pname, 0, sizeof(pname));
	plevel = 0;
	pclass_ = 0;
	prace = 0;
	panon = 0;
	ptellsoff = 0;
	pguild_id = GUILD_NONE;
	pLFG = 0;
	gm = 0;
	pClientVersion = 0;
}

void ClientListEntry::Camp(ZoneServer* iZS) {
	if (iZS != 0 && iZS != pzoneserver)
		return;
	if (pzoneserver){
		pzoneserver->RemovePlayer();
	}

	ClearVars();

	mStale = 0;
}

bool ClientListEntry::CheckStale() {
	mStale++;
	if (mStale > 20) {
		if (mOnline > CLE_Status_Offline)
			SetOnline(CLE_Status_Offline);
		else
			return true;
	}
	return false;
}

bool ClientListEntry::CheckAuth(uint32 iLSID, const char* iKey) {
	if (strncmp(mLoginServerKey, iKey,10) == 0) {
		if (paccountid == 0 && getLoginServerAccountID()>0) {
			int16 tmpStatus = WorldConfig::get()->DefaultStatus;
			paccountid = database.CreateAccount(mLoginServerAccountName, tmpStatus, getLoginServerAccountID());
			if (!paccountid) {
				_log(WORLD__CLIENTLIST_ERR,"Error adding local account for LS login: '%s', duplicate name?" ,mLoginServerAccountName);
				return false;
			}
			strn0cpy(paccountname, mLoginServerAccountName, sizeof(paccountname));
			padmin = tmpStatus;
		}
		char lsworldadmin[15] = "0";
		database.GetVariable("honorlsworldadmin", lsworldadmin, sizeof(lsworldadmin));
		if (atoi(lsworldadmin) == 1 && mWorldAdminStatus != 0 && (padmin < mWorldAdminStatus || padmin == 0))
			padmin = mWorldAdminStatus;
		return true;
	}
	return false;
}