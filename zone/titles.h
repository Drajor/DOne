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
#ifndef TITLES_H
#define TITLES_H

#include "../common/types.h"
#include <vector>
#include <string>

class Client;
class EQApplicationPacket;

struct TitleData {
	int mID;
	int mSkillID;
	int mMinSkillValue;
	int mMaxSkillValue;
	int mMinAAPoints;
	int mMaxAAPoints;
	int mClass;
	int mGender;
	int mCharacterID;
	int mStatus;
	int mItemID;
	std::string mPrefix;
	std::string mSuffix;
	int mTitleSetID; // TODO: This needs better documentation.
};

class TitleManager {
public:
	static TitleManager* getSingleton();
	bool initialise();
	std::string getPrefix(int pTitleID);
	std::string getSuffix(int pTitleID);
	bool isEligible(Client* pClient, TitleData& pTitle);
	bool isNewAATitleAvailable(int pAAPoints, int pClass);
	bool isNewTradeSkillTitleAvailable(int pSkillID, int pSkillValue);
	EQApplicationPacket* makeTitlesPacket(Client* pClient);

private:
	TitleManager();
	~TitleManager();
	std::vector<TitleData> mTitles;
};

#endif

