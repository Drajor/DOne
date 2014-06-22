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
#include "TitleManager.h"
#include "../common/debug.h"
#include "../common/eq_packet_structs.h"
#include "masterentity.h"
#include "../common/StringUtil.h"
#include "worldserver.h"

TitleManager::TitleManager() {}
TitleManager::~TitleManager() {}

TitleManager* TitleManager::getSingleton() {
	static TitleManager* titleManager = nullptr;
	if (titleManager == nullptr)
		titleManager = new TitleManager;

	return titleManager;
}


bool TitleManager::initialise() {
	mTitles.clear();

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = nullptr;
	MYSQL_RES* result;
	MYSQL_ROW row;

	// SQL:
	if (!database.RunQuery(query, MakeAnyLenString(&query, "SELECT `id`, `skill_id`, `min_skill_value`, `max_skill_value`, `min_aa_points`, `max_aa_points`, `class`, `gender`, `char_id`, `status`, `item_id`, `prefix`, `suffix`, `title_set` from titles"), errbuf, &result)) {
		LogFile->write(EQEMuLog::Error, "Unable to load titles: %s : %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);

	TitleData title;
	while ((row = mysql_fetch_row(result))) {
		title.mID = atoi(row[0]);
		title.mSkillID = (SkillUseTypes)atoi(row[1]);
		title.mMinSkillValue = atoi(row[2]);
		title.mMaxSkillValue = atoi(row[3]);
		title.mMinAAPoints = atoi(row[4]);
		title.mMaxAAPoints = atoi(row[5]);
		title.mClass = atoi(row[6]);
		title.mGender = atoi(row[7]);
		title.mCharacterID = atoi(row[8]);
		title.mStatus = atoi(row[9]);
		title.mItemID = atoi(row[10]);
		title.mPrefix = row[11];
		title.mSuffix = row[12];
		title.mTitleSetID = atoi(row[13]);
		mTitles.push_back(title);
	}
	mysql_free_result(result);

	return(true);
}

EQApplicationPacket* TitleManager::makeTitlesPacket(Client* pClient) {
	std::vector<TitleData> eligibleTitles;

	// Determine which titles pClient is eligible for and calculate string length.
	uint32 length = 4;
	for (auto& i : mTitles) {
		if (!isEligible(pClient, i))
			continue;

		eligibleTitles.push_back(i);
		length += i.mPrefix.length() + i.mSuffix.length() + 6;
	}

	// Prepare packet.
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_SendTitleList, length);
	char* buffer = (char*)outapp->pBuffer;
	VARSTRUCT_ENCODE_TYPE(uint32, buffer, eligibleTitles.size());

	for (auto& i : eligibleTitles) {
		VARSTRUCT_ENCODE_TYPE(uint32, buffer, i.mID);
		VARSTRUCT_ENCODE_STRING(buffer, i.mPrefix.c_str());
		VARSTRUCT_ENCODE_STRING(buffer, i.mSuffix.c_str());
	}
	return outapp;
}

std::string TitleManager::getPrefix(int pTitleID) {
	for (auto& i : mTitles) {
		if (i.mID == pTitleID) return i.mPrefix;
	}

	// TODO: Logging here, this falls outside regular operation.
	return "";
}

std::string TitleManager::getSuffix(int pTitleID) {
	for (auto& i : mTitles) {
		if (i.mID == pTitleID) return i.mSuffix;
	}

	// TODO: Logging here, this falls outside regular operation.
	return "";
}

bool TitleManager::isEligible(Client* pClient, TitleData& pTitle) {
	// Character ID constraint.
	if (pTitle.mCharacterID >= 0 && pClient->CharacterID() != static_cast<uint32>(pTitle.mCharacterID)) return false;
	
	// Status constraint.
	if (pTitle.mStatus >= 0 && pClient->Admin() < pTitle.mStatus) return false;
	
	// Gender constraint.
	if (pTitle.mGender >= 0 && pClient->GetBaseGender() != pTitle.mGender) return false;
	
	// Class constraint.
	if (pTitle.mClass >= 0 && pClient->GetBaseClass() != pTitle.mClass) return false;
	
	// Minimum AA constraint.
	if (pTitle.mMinAAPoints >= 0 && pClient->GetAAPointsSpent() < static_cast<uint32>(pTitle.mMinAAPoints)) return false;
	
	// Maximum AA constraint.
	if (pTitle.mMaxAAPoints >= 0 && pClient->GetAAPointsSpent() > static_cast<uint32>(pTitle.mMaxAAPoints)) return false;
	
	// Skill constraint.
	if (pTitle.mSkillID >= 0) {
		// Minimum skill constraint.
		if (pTitle.mMinSkillValue >= 0 && pClient->GetRawSkill(static_cast<SkillUseTypes>(pTitle.mSkillID)) < static_cast<uint32>(pTitle.mMinSkillValue))
			return false;

		// Maximum skill constraint.
		if (pTitle.mMaxSkillValue >= 0 && pClient->GetRawSkill(static_cast<SkillUseTypes>(pTitle.mSkillID)) > static_cast<uint32>(pTitle.mMaxSkillValue))
			return false;
	}

	// Item ID constraint.
	if ((pTitle.mItemID >= 1) && (pClient->GetInv().HasItem(pTitle.mItemID, 0, 0xFF) == SLOT_INVALID))
		return false;

	// Title Set constraint.
	if ((pTitle.mTitleSetID > 0) && (!pClient->CheckTitle(pTitle.mTitleSetID)))
		return false;

	return true;
}

bool TitleManager::isNewAATitleAvailable(int pAAPoints, int pClass) {
	for (auto& i : mTitles) {
		if (((i.mClass == -1) || (i.mClass == pClass)) && (i.mMinAAPoints == pAAPoints))
			return true;
	}
	return false;
}

bool TitleManager::isNewTradeSkillTitleAvailable(int pSkillID, int pSkillValue) {
	for (auto& i : mTitles) {
		if (i.mSkillID == pSkillID && i.mMinSkillValue == pSkillValue)
			return true;
	}
	return false;
}