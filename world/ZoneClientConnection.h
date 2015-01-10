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
class DataStore;
class Guild;
class Item;
struct GuildMember;

class GroupManager;
class RaidManager;
class GuildManager;

class ZoneClientConnection {
public:
	enum ZoneConnectionStatus {
		NONE,
		ZoneEntryReceived,		// On OP_ZoneEntry
		PlayerProfileSent,
		ClientRequestZoneData,	// On OP_ReqNewZone
		ZoneInformationSent,
		ClientRequestSpawn,		// On OP_ReqClientSpawn
		Complete				// On OP_ClientReady
	};
public:
	ZoneClientConnection(EQStreamInterface* pStreamInterface, Zone* pZone, GroupManager* pGroupManager, RaidManager* pRaidManager, GuildManager* pGuildManager);
	~ZoneClientConnection();

	// Static initialise.
	static void _initalise();

	// Static deinitialise.
	static void _deinitialise();

	bool isConnected();
	inline bool isReadyForZoneIn() const { return mZoneConnectionStatus == Complete; }
	inline Character* getCharacter() { return mCharacter; }
	void update();
	bool _handlePacket(const EQApplicationPacket* pPacket);
	void _handleZoneEntry(const EQApplicationPacket* pPacket);
	void _handleRequestClientSpawn(const EQApplicationPacket* pPacket);

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

	void sendGuildRank();
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
private:
	void _handleUnknown(const EQApplicationPacket* pPacket);
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
	void _handleClientUpdate(const EQApplicationPacket* pPacket);
	void _handleSpawnAppearance(const EQApplicationPacket* pPacket);
	void _handleCamp(const EQApplicationPacket* pPacket);
	void _handleChannelMessage(const EQApplicationPacket* pPacket);
	void _handleLogOut(const EQApplicationPacket* pPacket);
	void _handleDeleteSpawn(const EQApplicationPacket* pPacket);
	void _handleRequestNewZoneData(const EQApplicationPacket* pPacket);

	void _sendZoneData();
	void _handleClientReady(const EQApplicationPacket* pPacket);
	void _handleSendAATable(const EQApplicationPacket* pPacket);
	void _handleUpdateAA(const EQApplicationPacket* pPacket);
	void _handleTarget(const EQApplicationPacket* pPacket);
	void _handleTGB(const EQApplicationPacket* pPacket);
	void _handleEmote(const EQApplicationPacket* pPacket);
	void _handleAnimation(const EQApplicationPacket* pPacket);
	void _handleWhoRequest(const EQApplicationPacket* pPacket);
	void _handleGroupInvite(const EQApplicationPacket* pPacket);
	void _handleGroupFollow(const EQApplicationPacket* pPacket);
	void _handleGroupCanelInvite(const EQApplicationPacket* pPacket);
	void _handleGroupDisband(const EQApplicationPacket* pPacket);
	void _handleGroupMakeLeader(const EQApplicationPacket* pPacket);
	void _handleZoneChange(const EQApplicationPacket* pPacket);
	void _handleFaceChange(const EQApplicationPacket* pPacket);

	void _handleGuildCreate(const EQApplicationPacket* pPacket);
	void _handleGuildDelete(const EQApplicationPacket* pPacket);
	void _handleGuildInvite(const EQApplicationPacket* pPacket);
	void _handleGuildInviteAccept(const EQApplicationPacket* pPacket);
	void _handleGuildRemove(const EQApplicationPacket* pPacket);
	void _handleSetGuildMOTD(const EQApplicationPacket* pPacket);
	void _handleGetGuildMOTD(const EQApplicationPacket* pPacket);
	void _handleSetGuildURLOrChannel(const EQApplicationPacket* pPacket);
	void _handleSetGuildPublicNote(const EQApplicationPacket* pPacket);
	void _handleGetGuildStatus(const EQApplicationPacket* pPacket);
	void _handleGuildDemote(const EQApplicationPacket* pPacket);
	void _handleGuildBanker(const EQApplicationPacket* pPacket);
	void _handleGuildMakeLeader(const EQApplicationPacket* pPacket);

	void _handleAutoAttack(const EQApplicationPacket* pPacket);

	void _unimplementedFeature(String pOpCodeName);

	void _handleMemoriseSpell(const EQApplicationPacket* pPacket);
	void _handleDeleteSpell(const EQApplicationPacket* pPacket);
	void _handleLoadSpellSet(const EQApplicationPacket* pPacket);
	void _handleSwapSpell(const EQApplicationPacket* pPacket);
	void _handleCastSpell(const EQApplicationPacket* pPacket);

	void _handleCombatAbility(const EQApplicationPacket* pPacket);
	void _handleTaunt(const EQApplicationPacket* pPacket);

	void _handleConsider(const EQApplicationPacket* pPacket);

	void _handleConsiderCorpse(const EQApplicationPacket* pPacket);

	void _handleSurname(const EQApplicationPacket* pPacket);
	void _handleClearSurname(const EQApplicationPacket* pPacket);
	void _handleGMLastName(const EQApplicationPacket* pPacket);

	void _handleSetTitle(const EQApplicationPacket* pPacket);
	void _handleRequestTitles(const EQApplicationPacket* pPacket);

	void _handleBeginLootRequest(const EQApplicationPacket* pPacket);
	void _handleEndLootRequest(const EQApplicationPacket* pPacket);
	void _handleLootItem(const EQApplicationPacket* pPacket);
	void _handleMoveItem(const EQApplicationPacket* pPacket);
	const bool _handleMoveItemImpl(const EQApplicationPacket* pPacket);

	void inventoryError();

	void _handleConsume(const EQApplicationPacket* pPacket);
	void _handlePotionBelt(const EQApplicationPacket* pPacket);
	void _handleItemRightClick(const EQApplicationPacket* pPacket);
	void _handleOpenContainer(const EQApplicationPacket* pPacket);
	
	void _handleTradeRequest(const EQApplicationPacket* pPacket);
	void _handleTradeRequestAck(const EQApplicationPacket* pPacket);
	void _handleCancelTrade(const EQApplicationPacket* pPacket);
	void _handleAcceptTrade(const EQApplicationPacket* pPacket);
	void _handleTradeBusy(const EQApplicationPacket* pPacket);

	void _handleSetServerFiler(const EQApplicationPacket* pPacket);

	void _handleItemLinkClick(const EQApplicationPacket* pPacket);
	void _handleItemView(const EQApplicationPacket* pPacket);
	void _handleMoveCoin(const EQApplicationPacket* pPacket);
	const bool _handleMoveCoinImpl(const EQApplicationPacket* pPacket);
	void _handleCrystalCreate(const EQApplicationPacket* pPacket);
	void _handleCrystalReclaim(const EQApplicationPacket* pPacket);
	void _handleEnvironmentalDamage(const EQApplicationPacket* pPacket);
	void _handlePopupResponse(const EQApplicationPacket* pPacket);
	void _handleClaimRequest(const EQApplicationPacket* pPacket);
	void _handleAugmentItem(const EQApplicationPacket* pPacket);
	void _handleAugmentInfo(const EQApplicationPacket* pPacket);
	void _handleReadBook(const EQApplicationPacket* pPacket);
	void _handleCombine(const EQApplicationPacket* pPacket);
	
	void _handleShopRequest(const EQApplicationPacket* pPacket);
	void _handleShopEnd(const EQApplicationPacket* pPacket);
	void _handleShopSell(const EQApplicationPacket* pPacket);
	void _handleShopBuy(const EQApplicationPacket* pPacket);

	void _handleAlternateCurrencyReclaim(const EQApplicationPacket* pPacket);

	void _handleRandomRequest(const EQApplicationPacket* pPacket);
	void _handleDropItem(const EQApplicationPacket* pPacket);

	void _handleXTargetAutoAddHaters(const EQApplicationPacket* pPacket);
	void _handleRespawnWindowSelect(const EQApplicationPacket* pPacket);

	void _handleAAAction(const EQApplicationPacket* pPacket);
	void _handleLeadershipExperienceToggle(const EQApplicationPacket* pPacket);

	bool mConnected = false;
	Timer mForceSendPositionTimer;
	EQStreamInterface* mStreamInterface = nullptr;
	Zone* mZone = nullptr;
	Character* mCharacter = nullptr;
	ZoneConnectionStatus mZoneConnectionStatus = ZoneConnectionStatus::NONE;
	
	GuildManager* mGuildManager = nullptr;
	GroupManager* mGroupManager = nullptr;
	RaidManager* mRaidManager = nullptr;

	static EQApplicationPacket* mPlayerProfilePacket;

	static EQApplicationPacket* mGroupJoinPacket;
	static EQApplicationPacket* mGroupLeavePacket;
	static EQApplicationPacket* mGroupDisbandPacket;
	static EQApplicationPacket* mGroupLeaderChangePacket;
	static EQApplicationPacket* mGroupUpdateMembersPacket;
};