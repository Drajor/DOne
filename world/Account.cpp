#include "Account.h"
#include "AccountManager.h"
#include "Data.h"

Account::Account(AccountManager* pOwner) : mOwner(pOwner) {

}

Account::~Account() {
	if (mOwner && mSaveNeeded) {
		mOwner->_save(this);
	}
}


const u32 Account::numCharacters() const {
	return mData->mCharacterData.size();
}

const bool Account::ownsCharacter(const String& pCharacterName) const {
	for (auto i : mData->mCharacterData) {
		if (i->mName == pCharacterName)
			return true;
	}

	return false;
}

Data::AccountCharacter* Account::getData(const String& pCharacterName) {
	for (auto i : mData->mCharacterData) {
		if (i->mName == pCharacterName)
			return i;
	}
	return nullptr;
}