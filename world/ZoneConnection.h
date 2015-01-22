#pragma once

#include "Constants.h"
#include "Vector3.h"

#include "../common/timer.h"

class EQStreamInterface;
class EQApplicationPacket;
class Zone;
class Character;
class Object;
class Door;
class IDataStore;
class ILog;
class ILogFactory;
class Guild;
class Item;
class GuildMember;

class GroupManager;
class RaidManager;
class GuildManager;

namespace ZCStatus{
	enum : u32 {
		None,
		ZoneEntryReceived,		// On OP_ZoneEntry
		PlayerProfileSent,
		ClientRequestZoneData,	// On OP_ReqNewZone
		ZoneInformationSent,
		ClientRequestSpawn,		// On OP_ReqClientSpawn
		Complete				// On OP_ClientReady
	};
}

class ZoneConnection {
public:
	// Static initialise.
	static void _initalise();

	// Static deinitialise.
	static void _deinitialise();

public:
	ZoneConnection();
	~ZoneConnection();

	const bool initialise(EQStreamInterface* pStreamInterface, ILogFactory* pLogFactory, Zone* pZone, GuildManager* pGuildManager);
	inline const i64& getConnectTime() const { return mConnectTime; }

	bool isConnected();
	inline bool isConnectComplete() const { return mConnectingStatus == ZCStatus::Complete; }
	inline Character* getCharacter() { return mCharacter; }
	void update();
	bool handlePacket(const EQApplicationPacket* pPacket);

	void dropConnection();

	void sendPosition();
	void sendMessage(const u32 pType, String pMessage);
	void sendSimpleMessage(const u32 pType, const u32 pStringID);
	static EQApplicationPacket* makeSimpleMessage(const u32 pType, const u32 pStringID, String pParameter0, String pParameter1 = EmptyString, String pParameter2 = EmptyString, String pParameter3 = EmptyString, String pParameter4 = EmptyString, String pParameter5 = EmptyString, String pParameter6 = EmptyString, String pParameter7 = EmptyString, String pParameter8 = EmptyString, String pParameter9 = EmptyString);
	void sendSimpleMessage(const u32 pType, const u32 pStringID, String pParameter0, String pParameter1 = EmptyString, String pParameter2 = EmptyString, String pParameter3 = EmptyString, String pParameter4 = EmptyString, String pParameter5 = EmptyString, String pParameter6 = EmptyString, String pParameter7 = EmptyString, String pParameter8 = EmptyString, String pParameter9 = EmptyString);
	void sendAppearance(uint16 pType, uint32 pParameter);
	void sendHealthUpdate();

	void sendWearChange(const uint16 pSpawnID, const uint32 pMaterialID, const uint32 pEliteMaterialID, const uint32 pColour, const uint8 pSlotID);

	void sendExperienceUpdate(const u32 pExperience, const u32 pAAExperience);
	void sendAAExperienceUpdate(const u32 pAAExperience, const u32 pUnspentAA, const u32 pExperienceToAA);
	void sendLeadershipExperienceUpdate(const double pGroupExperience, const u32 pGroupPoints, const double pRaidExperience, const u32 pRaidPoints);
	
	void sendExperienceMessage();
	void sendGroupExperienceMessage();
	void sendRaidExperienceMessage();

	void sendExperienceLossMessage();
	void sendLevelGainMessage();
	void sendLevelsGainMessage(const u8 pLevels);
	void sendLevelLostMessage();

	void sendAAExperienceOnMessage();
	void sendAAExperienceOffMessage();
	void sendAAPointGainMessage(const u32 pUnspentAAPoints);

	void sendGainGroupLeadershipExperienceMessage();
	void sendGainGroupLeadershipPointMessage();
	void sendGainRaidLeadershipExperienceMessage();
	void sendGainRaidLeadershipPointMessage();

	void sendLevelAppearance(const u32 pParameter1);
	void sendLevelUpdate(const u32 pPreviousLevel, const u32 pCurrentLevel, const u32 pExperienceRatio);
	void sendStats();
	void sendWhoResponse(const u32 pWhoType, std::list<Character*>& pMatches);

	static EQApplicationPacket* makeChannelMessage(const u32 pChannel, const String& pSenderName, const String& pMessage);
	void sendChannelMessage(const u32 pChannel, const String& pSenderName, const String& pMessage);
	void sendTell(const String& pSenderName, const String& pMessage);
	void sendGroupMessage(const String& pSenderName, const String& pMessage);
	void sendGuildMessage(const String& pSenderName, const String& pMessage);

	void sendGroupInvite(const String pFromCharacterName);
	void sendGroupCreate();
	void sendGroupLeaderChange(const String pCharacterName);
	void sendGroupAcknowledge();
	void sendGroupFollow(const String& pLeaderCharacterName, const String& pMemberCharacterName);
	void sendGroupJoin(const String& pCharacterName);
	void sendGroupUpdate(std::list<String>& pGroupMemberNames);
	void sendGroupDisband();
	void sendGroupLeave(const String& pLeavingCharacterName);

	void sendRequestZoneChange(const uint16 pZoneID, const uint16 pInstanceID, const Vector3& pPosition);
	void sendZoneChange(const uint16 pZoneID, const uint16 pInstanceID, const Vector3& pPosition, const int32 pSuccess);

	void sendGuildRank(const u32 pRank);
	void sendGuildInvite(String pInviterName, GuildID pGuildID);
	void sendGuildMOTD(const String& pMOTD, const String& pMOTDSetByName);
	void sendGuildMOTDReply(const String& pMOTD, const String& pMOTDSetByName);
	void sendGuildMembers(const std::list<GuildMember*>& pGuildMembers);
	void sendGuildURL(const String& pURL);
	void sendGuildChannel(const String& pChannel);

	void sendSurnameApproval(const bool pSuccess);

	void sendDeleteSpellDelete(const uint16 pSlot, const bool pSuccess);
	void sendScribeSpell(const u16 pSlot, const u32 pSpellID);
	void sendMemoriseSpell(const uint16 pSlot, const uint32 pSpellID);
	void _sendMemoriseSpell(const uint16 pSlot, const uint32 pSpellID, const uint32 pAction);
	void sendUnmemoriseSpell(const uint16 pSlot);
	void sendInterruptCast();
	void sendRefreshSpellBar(const uint16 pSlot, const uint32 pSpellID);
	void sendEnableSpellBar(const uint32 pSpellID);

	void sendManaUpdate();
	void sendEnduranceUpdate();

	void sendSkillValue(const uint32 pSkillID, const uint32 pValue);
	void sendLanguageValue(const uint32 pLanguageID, const uint32 pValue) { sendSkillValue(pLanguageID + 100, pValue); }

	void sendLootComplete();
	void sendLootResponse(uint8 pResponse, uint32 pPlatinum = 0, uint32 pGold = 0, uint32 pSilver = 0, uint32 pCopper = 0);

	void sendConsiderResponse(const uint32 pSpawnID, const uint32 pMessage);
	void sendStamina(const uint32 pFood, const uint32 pDrink);

	void sendItemRightClickResponse(const int32 pSlot, const uint32 pTargetSpawnID);
	void sendTradeRequest(const uint32 pFromSpawnID);

	void sendCurrencyUpdate();
	void sendCrystals();

	void sendPopup(const String& pTitle, const String& pText);
	void sendItemSummon(Item* pItem);
	void sendItemView(Item* pItem);
	void sendItemTrade(Item* pItem);
	void sendItemShop(Item* pItem);
	//void sendSpellCastOn();

	void sendTradeRequestAcknowledge(const uint32 pToSpawnID);
	void sendTradeFinished();
	void sendFinishWindow();
	void sendFinishWindow2();
	void sendTradeCancel(const uint32 pToSpawnID);

	void sendDeleteItem(const uint32 pSlot, const uint32 pStacks = 0xFFFFFFFF, const uint32 pToSlot = 0xFFFFFFFF);
	void sendMoveItem(const uint32 pFromSlot, const uint32 pToSlot = 0xFFFFFFFF, const uint32 pStacks = 0xFFFFFFFF);
	void sendReadBook(const uint32 pWindow, const uint32 pSlot, const uint32 pType, const String& pText);
	void sendCombineReply();

	void sendShopRequestReply(const uint32 pNPCSpawnID, const uint32 pAction, const float pRate = 1.0f);
	void sendShopEndReply();
	void sendShopSellReply(const uint32 pSpawnID, const uint32 pSlotID, const uint32 pStacks, const uint32 pPrice);
	void sendShopBuyReply(const uint32 pSpawnID, const uint32 pItemInstanceID, const uint32 pStacks, const uint64 pPrice, const uint32 pResponse = 0);
	void sendShopDeleteItem(const uint32 pSpawnID, const uint32 pItemInstanceID, const int32 pUnknown);

	void sendAddNimbus(const uint32 pSpawnID, const uint32 pEffectID);
	void sendRemoveNimbus(const uint32 pNimbusID);

	// Populates the 'Alt Currency' tab (Inventory Window).
	void sendAlternateCurrencies();

	// Updates all alternate currencies.
	void sendAlternateCurrencyQuantities(const bool pSendZero);

	// Updates a specific alternate currency.
	void sendAlternateCurrencyQuantity(const uint32 pCurrencyID, const uint32 pQuantity);

	// Updates a specific alternate currency.
	void sendAlternateCurrencyQuantity(const uint32 pCurrencyID);

	void sendMOTD(const String& pMOTD);

	void sendObject(Object* pObject);

	void sendRespawnWindow();

	void sendPacket(const EQApplicationPacket* pPacket);

	void sendZoneEntry();

	const bool handleZoneEntry(const EQApplicationPacket* pPacket);
	const bool handleRequestClientSpawn(const EQApplicationPacket* pPacket);
	const bool handleClientReady(const EQApplicationPacket* pPacket);
	const bool handleRequestNewZoneData(const EQApplicationPacket* pPacket);
	const bool handleClientUpdate(const EQApplicationPacket* pPacket);
	const bool handleSpawnAppearance(const EQApplicationPacket* pPacket);
	const bool handleTarget(const EQApplicationPacket* pPacket);

	const bool handleChannelMessage(const EQApplicationPacket* pPacket); // TODO: Write some unit tests for this method.

	// Group Packets.
	const bool handleGroupInvite(const EQApplicationPacket* pPacket);
	const bool handleGroupAcceptInvite(const EQApplicationPacket* pPacket);
	const bool handleGroupDeclineInvite(const EQApplicationPacket* pPacket);
	const bool handleGroupDisband(const EQApplicationPacket* pPacket);
	const bool handleGroupMakeLeader(const EQApplicationPacket* pPacket);

	// Raid Packets.

	// Guild Packets.
	const bool handleGuildCreate(const EQApplicationPacket* pPacket);
	const bool handleGuildDelete(const EQApplicationPacket* pPacket);
	const bool handleGuildInvite(const EQApplicationPacket* pPacket);
	const bool handleGuildInviteResponse(const EQApplicationPacket* pPacket);
	const bool handleGuildRemove(const EQApplicationPacket* pPacket);
	const bool handleGuildSetMOTD(const EQApplicationPacket* pPacket);
	const bool handleGuildGetMOTD(const EQApplicationPacket* pPacket);
	const bool handleSetGuildURLOrChannel(const EQApplicationPacket* pPacket);
	const bool handleSetGuildPublicNote(const EQApplicationPacket* pPacket); // TODO: I need to check whether this has a fixed length.
	const bool handleGetGuildStatus(const EQApplicationPacket* pPacket);
	const bool handleGuildDemote(const EQApplicationPacket* pPacket);
	const bool handleGuildSetFlags(const EQApplicationPacket* pPacket);
	const bool handleGuildMakeLeader(const EQApplicationPacket* pPacket);

	// Spells
	const bool handleMemoriseSpell(const EQApplicationPacket* pPacket);
	const bool handleDeleteSpell(const EQApplicationPacket* pPacket);
	const bool handleLoadSpellSet(const EQApplicationPacket* pPacket);
	const bool handleSwapSpell(const EQApplicationPacket* pPacket);
	const bool handleCastSpell(const EQApplicationPacket* pPacket);

	// Misc
	const bool handleZoneChange(const EQApplicationPacket* pPacket);
	const bool handleFaceChange(const EQApplicationPacket* pPacket);
	const bool handleAutoAttack(const EQApplicationPacket* pPacket);
	const bool handleTGB(const EQApplicationPacket* pPacket);
	const bool handleEmote(const EQApplicationPacket* pPacket);
	const bool handleAnimation(const EQApplicationPacket* pPacket);
	const bool handleWhoRequest(const EQApplicationPacket* pPacket);
	const bool handleCombatAbility(const EQApplicationPacket* pPacket);
	const bool handleTaunt(const EQApplicationPacket* pPacket);
	const bool handleConsider(const EQApplicationPacket* pPacket);
	const bool handleConsiderCorpse(const EQApplicationPacket* pPacket);
	const bool handleBeginLootRequest(const EQApplicationPacket* pPacket);
	const bool handleEndLootRequest(const EQApplicationPacket* pPacket);
	const bool handleLootItem(const EQApplicationPacket* pPacket);
	const bool handleConsume(const EQApplicationPacket* pPacket);
	const bool handleItemRightClick(const EQApplicationPacket* pPacket);
	const bool handleOpenContainer(const EQApplicationPacket* pPacket);
	const bool handleSetServerFiler(const EQApplicationPacket* pPacket);
	const bool handleItemLinkClick(const EQApplicationPacket* pPacket);

	const bool handleMoveItem(const EQApplicationPacket* pPacket);
	const bool handleMoveItemImpl(const EQApplicationPacket* pPacket);

	const bool handleMoveCoin(const EQApplicationPacket* pPacket);
	const bool handleMoveCoinImpl(const EQApplicationPacket* pPacket);

	// Shop / Merchant
	const bool handleShopRequest(const EQApplicationPacket* pPacket);
	const bool handleShopEnd(const EQApplicationPacket* pPacket);
	const bool handleShopSell(const EQApplicationPacket* pPacket);
	const bool handleShopBuy(const EQApplicationPacket* pPacket);

	// Trade.
	const bool handleTradeRequest(const EQApplicationPacket* pPacket);
	const bool handleTradeRequestAck(const EQApplicationPacket* pPacket);
	const bool handleCancelTrade(const EQApplicationPacket* pPacket);
	const bool handleAcceptTrade(const EQApplicationPacket* pPacket);
	const bool handleTradeBusy(const EQApplicationPacket* pPacket);

	// Alternate Currency
	const bool handleCrystalCreate(const EQApplicationPacket* pPacket);
	const bool handleCrystalReclaim(const EQApplicationPacket* pPacket);
	const bool handleAlternateCurrencyReclaim(const EQApplicationPacket* pPacket);

	const bool handleAugmentItem(const EQApplicationPacket* pPacket);
	const bool handleAugmentInfo(const EQApplicationPacket* pPacket);
	const bool handleReadBook(const EQApplicationPacket* pPacket);
	const bool handleCombine(const EQApplicationPacket* pPacket);
	const bool handleEnvironmentDamage(const EQApplicationPacket* pPacket);
	const bool handlePopupResponse(const EQApplicationPacket* pPacket);
	const bool handleClaimRequest(const EQApplicationPacket* pPacket);
	const bool handleCamp(const EQApplicationPacket* pPacket);
	const bool handlePotionBelt(const EQApplicationPacket* pPacket);
	const bool handleRandomRequest(const EQApplicationPacket* pPacket);
	const bool handleDropItem(const EQApplicationPacket* pPacket);
	const bool handleXTargetAutoAddHaters(const EQApplicationPacket* pPacket);
	const bool handleRespawnWindowSelect(const EQApplicationPacket* pPacket);
	const bool handleAAAction(const EQApplicationPacket* pPacket);
	const bool handleLeadershipExperienceToggle(const EQApplicationPacket* pPacket);
	const bool handleSurname(const EQApplicationPacket* pPacket);
	const bool handleClearSurname(const EQApplicationPacket* pPacket);
	const bool handleLogOut(const EQApplicationPacket* pPacket);
	const bool handleDeleteSpawn(const EQApplicationPacket* pPacket);
	const bool handleSetTitle(const EQApplicationPacket* pPacket);
	const bool handleRequestTitles(const EQApplicationPacket* pPacket);
	const bool handlePetCommand(const EQApplicationPacket* pPacket);
	const bool handleSaveRequest(const EQApplicationPacket* pPacket);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TEST LINE

	// Unsure below
	const bool handleSendAATable(const EQApplicationPacket* pPacket);
	const bool handleUpdateAA(const EQApplicationPacket* pPacket);
	const bool handleItemView(const EQApplicationPacket* pPacket);
	const bool handleGMLastName(const EQApplicationPacket* pPacket);

	inline const bool hasSizeError() const { return mSizeError; } // For unit testing.
	inline const bool hasStringError() const { return mStringError; } // For unit testing.
private:
	const bool handleUnknown(const EQApplicationPacket* pPacket);
	void _sendTimeOfDay();

	void _sendPlayerProfile();
	
	void _sendZoneSpawns();
	void _sendTributeUpdate();
	void _sendInventory();
	void _sendWeather();
	void _sendGuildNames();

	void _sendPreLogOutReply();
	void _sendLogOutReply();
	

	void _sendDoors();
	void _sendObjects();
	void _sendZonePoints();
	void _sendAAStats();
	void _sendZoneServerReady();
	void _sendExpZoneIn();
	void _sendWorldObjectsSent();


	void _sendZoneData();

	void _unimplementedFeature(String pOpCodeName);
	void inventoryError();
	void updateLogContext();
	bool mInitialised = false;
	bool mConnected = false;
	i64 mConnectTime = 0; // Time of initialise.
	Timer mForceSendPositionTimer;
	EQStreamInterface* mStreamInterface = nullptr;
	Zone* mZone = nullptr;
	Character* mCharacter = nullptr;
	u32 mConnectingStatus = ZCStatus::None;
	
	ILog* mLog = nullptr;
	GuildManager* mGuildManager = nullptr;

	static EQApplicationPacket* mPlayerProfilePacket;

	static EQApplicationPacket* mGroupJoinPacket;
	static EQApplicationPacket* mGroupLeavePacket;
	static EQApplicationPacket* mGroupDisbandPacket;
	static EQApplicationPacket* mGroupLeaderChangePacket;
	static EQApplicationPacket* mGroupUpdateMembersPacket;

	bool mSizeError = false; // For unit testing.
	bool mStringError = false; // For unit testing.
};