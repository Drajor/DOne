#include "Account.h"
#include "LogSystem.h"
#include "Data.h"

Account::Account(Data::Account* pData, ILog* pLog) : mData(pData), mLog(pLog) { }

const u32 Account::getLoginAccountID() const { return mData->mLoginAccountID; }
const u32 Account::getLoginServerID() const { return mData->mLoginServerID; }
const String& Account::getLoginAccountName() const { return mData->mLoginAccountName; }
const i8 Account::getStatus() const { return mData->mStatus; }

void Account::setStatus(const i8 pStatus) { mData->mStatus = pStatus; }

const i64 Account::getSuspensionTime() const { return mData->mSuspendedUntil; }

void Account::setSuspensionTime(const i64 pTime) { mData->mSuspendedUntil = pTime; }

const u32 Account::numCharacters() const {
	return mData->mCharacterData.size();
}

const bool Account::ownsCharacter(const String& pCharacterName) const {
	for (auto i : mData->mCharacterData) {
		if (i->mName == pCharacterName)
			return true;
	}

	mLog->error("Ownership failed, Character: " + pCharacterName);
	return false;
}

Data::AccountCharacter* Account::getData(const String& pCharacterName) {
	for (auto i : mData->mCharacterData) {
		if (i->mName == pCharacterName)
			return i;
	}

	mLog->error("Could not find data for Character: " + pCharacterName);
	return nullptr;
}

const i32 Account::getSharedPlatinum() const { return mData->mPlatinumSharedBank; }
void Account::setSharedPlatinum(const i32 pSharedPlatinum) { mData->mPlatinumSharedBank = pSharedPlatinum; }

const bool Account::isBanned() const { return getStatus() == AccountStatus::Banned; }
const bool Account::isSuspended() const { return getStatus() == AccountStatus::Suspended; }
