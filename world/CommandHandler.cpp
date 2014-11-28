#include "CommandHandler.h"
#include "World.h"
#include "Character.h"
#include "NPC.h"
#include "Zone.h"
#include "ZoneData.h"
#include "ZoneManager.h"
#include "GuildManager.h"
#include "Utility.h"
#include "LogSystem.h"
#include "ZoneClientConnection.h"
#include "Limits.h"
#include "ItemGenerator.h"
#include "ItemDataStore.h"
#include "Item.h"
#include "Inventory.h"

#define PACKET_PLAY
#ifdef PACKET_PLAY
#include "../common/eq_packet_structs.h"
#include "../common/EQPacket.h"
#include "Payload.h"
#endif

/*****************************************************************************************************************************/
class ZoneCommand : public Command {
public:
	ZoneCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #zone <Zone ID> <Zone Instance ID>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameters
		if (pParameters.size() != 2) {
			invalidParameters(pParameters);
			return false;
		}

		uint32 zoneID = 0;
		uint32 instanceID = 0;
		const bool ok = Utility::stou32Safe(zoneID, pParameters[0]) && Utility::stou32Safe(instanceID, pParameters[1]);
		if (!ok) {
			// TODO: Parameter error.
			return false;
		}

		Vector3 zoneSafePoint;
		if (!ZoneDataManager::getInstance().getSafePoint(zoneID, zoneSafePoint)){
			// TODO:
			return false;
		}

		mInvoker->setZoneChange(zoneID, instanceID);
		mInvoker->getConnection()->sendRequestZoneChange(zoneID, instanceID, zoneSafePoint);
		return true;
	}
};

/*****************************************************************************************************************************/
class WarpCommand : public Command {
public:
	WarpCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #warp (warps yourself to your target).");
		mHelpMessages.push_back("Usage: #warp <X> <Y> <Z> (warps your target or yourself to the specified position).");
		mHelpMessages.push_back("Usage: #warp safe (warps your target or yourself to the zone safe point).");
		mHelpMessages.push_back("Usage: #warp <name> (warps you to a specific Character).");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Warp to target.
		if (pParameters.empty()) {
			Actor* target = mInvoker->getTarget();
			if (!target) { return false; }
			Vector3 position = target->getPosition();
			mInvoker->notify("Warping to target " + position.toString());
			mInvoker->getZone()->moveCharacter(mInvoker, position);
			return true;
		}
		
		// Warp to position
		if (pParameters.size() == 3) {
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			// Check: Parameter conversion.
			const bool ok = Utility::stofSafe(x, pParameters[0]) && Utility::stofSafe(y, pParameters[1]) && Utility::stofSafe(z, pParameters[2]);
			if (!ok) { return false; }
			Vector3 position(x, y, z);
			mInvoker->notify("Warping to position " + position.toString());
			mInvoker->getZone()->moveCharacter(mInvoker, x, y, z);
			return true;
		}

		// Warp to safe
		if (pParameters.size() == 1 && pParameters[0] == "safe") {
			Vector3 safePoint = mInvoker->getZone()->getSafePoint();
			mInvoker->notify("Warping to zone safe point " + safePoint.toString());
			mInvoker->getZone()->moveCharacter(mInvoker, safePoint);
			return true;
		}

		// Warp to Character
		if (pParameters.size() == 1) {
			const String characterName = pParameters[0];
			auto character = ZoneManager::getInstance().findCharacter(characterName);
			if (!character) {
				mInvoker->notify("Could not find " + characterName);
				return true;
			}

			mInvoker->notify("Warping to " + character->getName() + " at " + character->getPosition().toString());
			mInvoker->getZone()->moveCharacter(mInvoker, Vector3(character->getPosition()));
			return true;
		}

		return false;
	}
};

/*****************************************************************************************************************************/
class GMCommand : public Command {
public:
	GMCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #gm on / #gm off");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 1) {
			invalidParameters(pParameters);
			return false;
		}

		bool gm = pParameters[0] == "on";
		mInvoker->setIsGM(gm);
		mInvoker->getZone()->notifyCharacterGM(mInvoker);
		return true;
	}
};

/*****************************************************************************************************************************/
class ZoneListCommand : public Command {
public:
	ZoneListCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #zonelist");
	};

	const bool handleCommand(CommandParameters pParameters) {
		ZoneSearchResult result = ZoneManager::getInstance().getAllZones();
		for (auto i : result) {
			StringStream ss; ss << "[Zone] " << i.mName << "(" << i.mID << "," << i.mInstanceID << ") Players: " << i.mNumCharacters;
			mInvoker->getConnection()->sendMessage(MessageType::Aqua, ss.str());
		}

		return true;
	}
};

/*****************************************************************************************************************************/
class AddExperienceCommand : public Command {
public:
	AddExperienceCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #addexp <number>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 1) {
			invalidParameters(pParameters);
			return false;
		}

		uint32 expAdd = 0;
		// Check: Parameter conversion.
		const bool ok = Utility::stou32Safe(expAdd, pParameters[0]);
		if (!ok) {
			return false;
		}

		mInvoker->addExperience(expAdd);
		return true;
	}
};

/*****************************************************************************************************************************/
class RemoveExperienceCommand : public Command {
public:
	RemoveExperienceCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #remexp <number>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 1) {
			invalidParameters(pParameters);
			return false;
		}

		uint32 expRemove = 0;
		// Check: Parameter conversion.
		const bool ok = Utility::stou32Safe(expRemove, pParameters[0]);
		if (!ok) {
			return false;
		}

		mInvoker->removeExperience(expRemove);
		return true;
	}
};

/*****************************************************************************************************************************/
class LocationCommand : public Command {
public:
	LocationCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #loc");
	};

	const bool handleCommand(CommandParameters pParameters) {
		Vector3 position = mInvoker->getPosition();
		StringStream ss;
		ss << "Your location is " << position.toString() << " Heading: " << mInvoker->getHeading();
		mInvoker->message(MessageType::White, ss.str());

		return true;
	}
};

/*****************************************************************************************************************************/
class LevelCommand : public Command {
public:
	LevelCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #level <number>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 1) {
			invalidParameters(pParameters);
			return false;
		}

		uint32 level = 0;
		// Check: Parameter conversion.
		const bool ok = Utility::stou32Safe(level, pParameters[0]);
		if (!ok) {
			return false;
		}

		mInvoker->setCharacterLevel(static_cast<uint8>(level));
		return true;
	}
};

/*****************************************************************************************************************************/
class StatsCommand : public Command {
public:
	StatsCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage Example: #setstat str 10");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 2) {
			invalidParameters(pParameters);
			return false;
		}

		String statStr = pParameters[0];
		String statName = "unknown";
		uint32 value = 0;
		Statistic statistic;
		// Check: Parameter conversion.
		const bool ok = Utility::stou32Safe(value, pParameters[1]);
		if (!ok) {
			return false;
		}

		// Determine which statistic is changing.
		if (statStr == "str") { statistic = Statistic::Strength; statName = "strength"; }
		else if (statStr == "sta") { statistic = Statistic::Stamina; statName = "stamina"; }
		else if (statStr == "cha") { statistic = Statistic::Charisma; statName = "charisma"; }
		else if (statStr == "dex") { statistic = Statistic::Dexterity; statName = "dexterity"; }
		else if (statStr == "int") { statistic = Statistic::Intelligence; statName = "intelligence"; }
		else if (statStr == "agi") { statistic = Statistic::Agility; statName = "agility"; }
		else if (statStr == "wis") { statistic = Statistic::Wisdom; statName = "wisdom"; }
		else {
			return false;
		}

		StringStream ss;
		ss << "Changing " << mInvoker->getName() << "'s " << statName << " from " << mInvoker->getBaseStatistic(statistic) << " to " << value;
		mInvoker->message(MessageType::LightGreen, ss.str());
		mInvoker->setBaseStatistic(statistic, value);

		return true;
	}
};

/*****************************************************************************************************************************/
class ZoneSearchCommand : public Command {
public:
	ZoneSearchCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #zs <text>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 1) {
			invalidParameters(pParameters);
			return false;
		}

		ZoneDataSearchResults results = ZoneDataManager::getInstance().searchByName(pParameters[0]);
		mInvoker->message(MessageType::Yellow, "Search found " + std::to_string(results.size()) + " results.");
		for (auto i : results){
			mInvoker->message(MessageType::Yellow, "[Zone " + std::to_string(i.mID) + "] " + i.mShortName + " | " + i.mLongName );
		}

		return true;
	}
};

/*****************************************************************************************************************************/
class GuildSearchCommand : public Command {
public:
	GuildSearchCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #guildsearch <optional text>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Get all guilds.
		if (pParameters.size() == 0) {
			GuildSearchResults results = GuildManager::getInstance().getAllGuilds();
			for (auto i : results){
				mInvoker->message(MessageType::Yellow, "[Guild " + std::to_string(i.mID) + "] " + i.mName);
			}
		}
		// Search guilds.
		else if (pParameters.size() == 1) {
		}
		else {
			invalidParameters(pParameters);
			return false;
		}

		return false;
	}
};

/*****************************************************************************************************************************/
class GuildInformationCommand : public Command {
public:
	GuildInformationCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #ginfo");
	};

	const bool handleCommand(CommandParameters pParameters) {
		if (mInvoker->hasGuild()) {
			static const String RankNames[] = { "Member", "Officer", "Leader" };
			mInvoker->message(MessageType::Yellow, RankNames[mInvoker->getGuildRank()] + " of " + std::to_string(mInvoker->getGuildID()));
		}
		else {
			mInvoker->message(MessageType::Yellow, "No Guild");
		}

		return true;
	}
};

/*****************************************************************************************************************************/
// Allows for cross zone and offline guild promotion.
class GuildPromoteCommand : public Command {
public:
	GuildPromoteCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #guildpromote <name>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		if (pParameters.size() == 1 && Limits::Character::nameLength(pParameters[0]) ) {
			if (mInvoker->hasGuild() && mInvoker->getGuildRank() == GuildRanks::Leader) {
				GuildManager::getInstance().handlePromote(mInvoker, pParameters[0]);
			}
			else {
				mInvoker->message(MessageType::Yellow, "No guild or not guild leader.");
			}

			return true;
		}

		return false;
	}
};

/*****************************************************************************************************************************/
// Allows for cross zone and offline guild demotion.
class GuildDemoteCommand : public Command {
public:
	GuildDemoteCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #guilddemote <name>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		if (pParameters.size() == 1 && Limits::Character::nameLength(pParameters[0])) {
			if (mInvoker->hasGuild() && mInvoker->getGuildRank() == GuildRanks::Leader) {
				GuildManager::getInstance().handleDemote(mInvoker, pParameters[0]);
			}
			else {
				mInvoker->message(MessageType::Yellow, "No guild or not guild leader.");
			}

			return true;
		}

		return false;
	}
};

class WearChangeCommand : public Command {
public:
	WearChangeCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #wc <slot> <material> <colour>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		if (pParameters.size() != 3) {
			invalidParameters(pParameters);
			return false;
		}

		uint32 slotID = 0;
		uint32 materialID = 0;
		uint32 colour = 0;
		// Check: Parameter conversion.
		bool ok = Utility::stou32Safe(slotID, pParameters[0]) && Utility::stou32Safe(materialID, pParameters[1]) && Utility::stou32Safe(colour, pParameters[2]);
		if (!ok) {
			return false;
		}

		mInvoker->getConnection()->sendWearChange(mInvoker->getSpawnID(), slotID, materialID, colour);
		return true;
	}
};

class SurnameCommand : public Command {
public:
	SurnameCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #surname <name>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 1) {
			invalidParameters(pParameters);
			return false;
		}
		const bool hasTarget = mInvoker->hasTarget();

		// Check: Has a target.
		if (!hasTarget) {
			mInvoker->notify("You must target an actor.");
			return false;
		}
		Actor* changeActor = mInvoker->getTarget();

		// Check: Length.
		if (!Limits::Character::surnameLengthClient(pParameters[0])) {
			mInvoker->notify("Length invalid");
			return false;
		}

		changeActor->setLastName(pParameters[0]);
		changeActor->getZone()->handleSurnameChange(changeActor);
		return true;
	}
};

class FindSpellCommand : public Command {
public:
	FindSpellCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #findspell <name>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 1) {
			invalidParameters(pParameters);
			return false;
		}

		// TODO:
		return true;
	}
};

class SetSkillCommand : public Command {
public:
	SetSkillCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #setskill <id> <value>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 2) {
			invalidParameters(pParameters);
			return false;
		}

		uint32 skillID = 0;
		uint32 skillValue = 0;
		// Check: Parameter conversion.
		bool ok = Utility::stou32Safe(skillID, pParameters[0]) && Utility::stou32Safe(skillValue, pParameters[1]);
		if (!ok) {
			return false;
		}

		// Check: Skill ID is valid.
		if (!Limits::Skills::validID(skillID)) {
			mInvoker->notify("Skill ID out of range: " + std::to_string(skillID));
			return false;
		}
		if (!mInvoker->setSkill(skillID, skillValue)) {
			return false;
		}

		mInvoker->getConnection()->sendSkillValue(skillID, skillValue);
		return true;
	}
};

class GetSkillCommand : public Command {
public:
	GetSkillCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #getskill <id>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 1) {
			invalidParameters(pParameters);
			return false;
		}

		uint32 skillID = 0;
		// Check: Parameter conversion.
		bool ok = Utility::stou32Safe(skillID, pParameters[0]);
		if (!ok) {
			conversionError(pParameters[0]);
			return false;
		}

		// Check: Skill ID is valid.
		if (!Limits::Skills::validID(skillID)) {
			mInvoker->notify("Skill ID out of range: " + std::to_string(skillID));
			return false;
		}

		// TODO: Target Based.
		// TODO: Base and adjusted skill levels.

		String skillName = Utility::Skills::fromID(skillID);
		uint32 skillValue = mInvoker->getSkill(skillID);
		mInvoker->notify("Your " + skillName + " is " + std::to_string(skillValue));
		return true;
	}
};

class SkillListCommand : public Command {
public:
	SkillListCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #skills");
	};

	const bool handleCommand(CommandParameters pParameters) {
		for (int i = 0; i < Limits::Skills::MAX_ID; i++)
			mInvoker->notify(std::to_string(i) + " - " + Utility::Skills::fromID(i));
		return true;
	}
};

class SetLanguageCommand : public Command {
public:
	SetLanguageCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #setlanguage <id> <value>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 2) {
			invalidParameters(pParameters);
			return false;
		}

		uint32 languageID = 0;
		uint32 languageValue = 0;
		// Check: Parameter conversion.
		bool ok = Utility::stou32Safe(languageID, pParameters[0]) && Utility::stou32Safe(languageValue, pParameters[1]);
		if (!ok) {
			return false;
		}

		// Check: Language ID is valid.
		if (!Limits::Languages::validID(languageID)) {
			mInvoker->notify("Language ID out of range: " + std::to_string(languageID));
			return false;
		}
		if (!mInvoker->setLanguage(languageID, languageValue)) {
			return false;
		}

		mInvoker->getConnection()->sendLanguageValue(languageID , languageValue);
		return true;
	}
};

class GetLanguageCommand : public Command {
public:
	GetLanguageCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #getlanguage <id>");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 1) {
			invalidParameters(pParameters);
			return false;
		}

		uint32 languageID = 0;
		// Check: Parameter conversion.
		bool ok = Utility::stou32Safe(languageID, pParameters[0]);
		if (!ok) {
			conversionError(pParameters[0]);
			return false;
		}

		// Check: Language ID is valid.
		if (!Limits::Languages::validID(languageID)) {
			mInvoker->notify("Language ID out of range: " + std::to_string(languageID));
			return false;
		}

		// TODO: Target Based.

		String languageName = Utility::Languages::fromID(languageID);
		uint32 languageValue = mInvoker->getLanguage(languageID);
		mInvoker->notify("Your " + languageName + " is " + std::to_string(languageValue));
		return true;
	}
};

class LanguageListCommand : public Command {
public:
	LanguageListCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #languages");
	};

	const bool handleCommand(CommandParameters pParameters) {
		for (int i = 0; i < Limits::Languages::MAX_ID; i++)
			mInvoker->notify(std::to_string(i) + " - " + Utility::Languages::fromID(i));
		return true;
	}
};

class DepopulateCommand : public Command {
public:
	DepopulateCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #depop");
	};

	const bool handleCommand(CommandParameters pParameters) {
		mInvoker->notify("Depopulating Zone...");
		mInvoker->getZone()->depopulate();
		return false;
	}
};

class PopulateCommand : public Command {
public:
	PopulateCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #pop");
	};

	const bool handleCommand(CommandParameters pParameters) {
		mInvoker->notify("Populating Zone...");
		mInvoker->getZone()->populate();
		return true;
	}
};

class RepopulateCommand : public Command {
public:
	RepopulateCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #repop");
	};

	const bool handleCommand(CommandParameters pParameters) {
		mInvoker->notify("Repopulating Zone...");
		mInvoker->notify("Depopulating Zone...");
		mInvoker->getZone()->depopulate();
		mInvoker->notify("Populating Zone...");
		mInvoker->getZone()->populate();
		return true;
	}
};

class WorldLockCommand : public Command {
public:
	WorldLockCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #lock 1/0");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() != 1) {
			invalidParameters(pParameters);
			return false;
		}

		uint32 lock = 0;
		// Check: Parameter conversion.
		bool ok = Utility::stou32Safe(lock, pParameters[0]);
		if (!ok) {
			conversionError(pParameters[0]);
			return false;
		}
		bool locked = lock == 1 ? true : false;
		World::getInstance().setLocked(locked);
		String s = locked ? "Locked" : "Unlocked";
		mInvoker->notify("World " + s);
		return true;
	}
};

class KillCommand : public Command {
public:
	KillCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #kill");
		mRequiresTarget = true;
	};

	const bool handleCommand(CommandParameters pParameters) {
		Actor* target = mInvoker->getTarget();

		if (target->isNPC()) {
			mInvoker->getZone()->handleDeath(target, mInvoker, 1, 0);
			return true;
		}

		return false;
	}
};

/*****************************************************************************************************************************/
class SummonItemCommand : public Command {
public:
	SummonItemCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #si <item id> <qty=1> <stacks=1>");
		setMinimumParameters(1);
		setMaximumParameters(3);
	};

	const bool handleCommand(CommandParameters pParameters) {
		uint32 itemID = 0;
		if (!Utility::stoSafe(itemID, pParameters[0])) {
			conversionError(pParameters[0]);
			return false;
		}

		// Convert (optional) quantity.
		uint8 quantity = 1;
		if (pParameters.size() >= 2) {
			if (!Utility::stoSafe(quantity, pParameters[1])) {
				conversionError(pParameters[1]);
				return false;
			}
		}

		// Convert (optional) stacks.
		uint32 stacks = 1;
		if (pParameters.size() == 3) {
			if (!Utility::stoSafe(stacks, pParameters[2])) {
				conversionError(pParameters[1]);
				return false;
			}
		}

		auto data = ItemDataStore::getInstance().get(itemID);
		EXPECTED_BOOL(data);
		for (auto i = 0; i < quantity; i++) {
			auto item = new Item(data);

			if (item->isStackable()) {
				item->setStacks(stacks > item->getMaxStacks() ? item->getMaxStacks() : stacks);
			}

			mInvoker->getInventory()->pushCursor(item);
			mInvoker->getConnection()->sendItemSummon(item);
		}

		return true;
	}
};

/*****************************************************************************************************************************/
class SummonRandomItemCommand : public Command {
public:
	SummonRandomItemCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Description: Summons a quantity of random items.");
		mHelpMessages.push_back("Usage: #sri <type> <level> <rarity> <qty=1>");
		mHelpMessages.push_back("Types: cont, head, chest, arms, wrists, legs, hands, feet ");
	};

	const bool handleCommand(CommandParameters pParameters) {
		// Check: Parameter #
		if (pParameters.size() < 3) {
			invalidParameters(pParameters);
			return false;
		}

		// Convert 'Level'
		uint8 level = 0;
		if (!Utility::stoSafe(level, pParameters[1])) {
			conversionError(pParameters[1]);
			return false;
		}

		// Convert 'Rarity'
		uint8 rarityIndex = 0;
		if (!Utility::stoSafe(rarityIndex, pParameters[2])) {
			conversionError(pParameters[2]);
			return false;
		}
		if (!RarityRangeCheck(rarityIndex)) {
			// TODO:
			return false;
		}
		Rarity rarity = RarityArray[rarityIndex];

		// Convert 'Quantity'
		uint8 quantity = 1;
		if (pParameters.size() == 4) {
			if (!Utility::stoSafe(quantity, pParameters[3])) {
				conversionError(pParameters[3]);
				return false;
			}
		}
		
		// Random Head.
		if (pParameters[0] == "head") {
			for (auto i = 0; i < quantity; i++) {
				Item* item = ItemGenerator::makeHead(level, rarity);
				mInvoker->getInventory()->pushCursor(item);
				mInvoker->getConnection()->sendItemSummon(item);
			}

			return true;
		}

		// Random Chest.
		if (pParameters[0] == "chest") {
			for (auto i = 0; i < quantity; i++) {
				Item* item = ItemGenerator::makeChest(level, rarity);
				mInvoker->getInventory()->pushCursor(item);
				mInvoker->getConnection()->sendItemSummon(item);
			}

			return true;
		}

		// Random Arms.
		if (pParameters[0] == "arms") {
			for (auto i = 0; i < quantity; i++) {
				Item* item = ItemGenerator::makeArms(level, rarity);
				mInvoker->getInventory()->pushCursor(item);
				mInvoker->getConnection()->sendItemSummon(item);
			}

			return true;
		}

		// Random Arms.
		if (pParameters[0] == "wrists") {
			for (auto i = 0; i < quantity; i++) {
				Item* item = ItemGenerator::makeWrists(level, rarity);
				mInvoker->getInventory()->pushCursor(item);
				mInvoker->getConnection()->sendItemSummon(item);
			}

			return true;
		}

		// Random Legs.
		if (pParameters[0] == "legs") {
			for (auto i = 0; i < quantity; i++) {
				Item* item = ItemGenerator::makeLegs(level, rarity);
				mInvoker->getInventory()->pushCursor(item);
				mInvoker->getConnection()->sendItemSummon(item);
			}

			return true;
		}

		// Random Arms.
		if (pParameters[0] == "hands") {
			for (auto i = 0; i < quantity; i++) {
				Item* item = ItemGenerator::makeHands(level, rarity);
				mInvoker->getInventory()->pushCursor(item);
				mInvoker->getConnection()->sendItemSummon(item);
			}

			return true;
		}

		// Random Feet.
		if (pParameters[0] == "feet") {
			for (auto i = 0; i < quantity; i++) {
				Item* item = ItemGenerator::makeFeet(level, rarity);
				mInvoker->getInventory()->pushCursor(item);
				mInvoker->getConnection()->sendItemSummon(item);
			}

			return true;
		}

		// Random Container.
		if (pParameters[0] == "cont") {
			for (auto i = 0; i < quantity; i++) {
				Item* item = ItemGenerator::makeRandomContainer(rarity);
				mInvoker->getInventory()->pushCursor(item);
				mInvoker->getConnection()->sendItemSummon(item);
			}

			return true;
		}

		// Random Shield
		if (pParameters[0] == "shield") {
			for (auto i = 0; i < quantity; i++) {
				Item* item = ItemGenerator::makeShield(level, rarity);
				mInvoker->getInventory()->pushCursor(item);
				mInvoker->getConnection()->sendItemSummon(item);
			}
			return true;
		}

		return false;
	}
};

/*****************************************************************************************************************************/
class SummonCommand : public Command {
public:
	SummonCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #summon <name> or current target.");
	};

	const bool handleCommand(CommandParameters pParameters) {

		// Summon target.
		if (pParameters.empty()) {
			if (!mInvoker->hasTarget()) return false;
			if (!mInvoker->targetIsCharacter()) return false; // Only summon Characters for now.

			Character* target = Actor::cast<Character*>(mInvoker->getTarget());

			mInvoker->notify("Summoning " + target->getName());
			target->notify("You have been summoned!");
			mInvoker->getZone()->moveCharacter(target, Vector3(mInvoker->getPosition()));

			return true;
		}

		// Summon by name.
		if (pParameters.size() == 1) {
			Zone* zone = mInvoker->getZone();
			const String characterName = pParameters[0];
			// Find Character to summon.
			Character* character = ZoneManager::getInstance().findCharacter(characterName);
			if (!character) {
				mInvoker->notify("Could not find " + characterName);
				return true;
			}

			// Check: Same Zone.
			if (character->getZone() == mInvoker->getZone()) {
				mInvoker->notify("Summoning " + character->getName());
				character->notify("You have been summoned!");
				zone->moveCharacter(character, Vector3(mInvoker->getPosition()));
				return true;
			}
			// Different zone.
			else {
				mInvoker->notify("Summoning " + character->getName());

				character->notify("You have been summoned!");
				character->setZoneChange(zone->getID(), zone->getInstanceID());
				character->getConnection()->sendRequestZoneChange(zone->getID(), zone->getInstanceID(), mInvoker->getPosition());
				return true;
			}

			return true;
		}

		return false;
	}
};

/*****************************************************************************************************************************/
class KickCommand : public Command {
public:
	KickCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #kick (kicks your target)");
		mHelpMessages.push_back("Usage: #kick <name> (kicks a specific Character)");
	};

	const bool handleCommand(CommandParameters pParameters) {
		Character* character = nullptr;
		
		// Kick target.
		if (pParameters.empty()) {
			if (!mInvoker->hasTarget()) return false;
			if (!mInvoker->targetIsCharacter()) return false;

			character = Actor::cast<Character*>(mInvoker->getTarget());
		}
		// Kick specified Character.
		else if (pParameters.size() == 1) {
			const String characterName = pParameters[0];
			character = ZoneManager::getInstance().findCharacter(characterName);
		}

		if (character) {
			mInvoker->notify("Kicking: " + character->getName());
			character->getConnection()->dropConnection();

			return true;
		}

		return false;
	}
};

/*****************************************************************************************************************************/
class InvulnerableCommand : public Command {
public:
	InvulnerableCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #invul <on/off>");
		mMinimumParameters = 1;
		mMaximumParameters = 1;
		mRequiresTarget = true;
	};

	const bool handleCommand(CommandParameters pParameters) {
		bool invulnerable = pParameters[0] == "on";
		Actor* target = mInvoker->getTarget();
		target->setInvulnerable(invulnerable);
		String msg = invulnerable ? "now" : "no longer";
		mInvoker->notify(target->getName() + " is " + msg + " invulnerable!");
		return true;
	}
};

/*****************************************************************************************************************************/
class InspectCommand : public Command {
public:
	InspectCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #inspect");
		mMinimumParameters = 0;
		mMaximumParameters = 0;
		mRequiresTarget = true;
	};

	const bool handleCommand(CommandParameters pParameters) {
		if (mInvoker->targetIsNPC()) { return inspectNPC(Actor::cast<NPC*>(mInvoker->getTarget())); }
		if (mInvoker->targetIsCharacter()) { return inspectCharacter(Actor::cast<Character*>(mInvoker->getTarget())); }

		return false;
	}

	String printItem(Item* pItem) {
		StringStream ss;
		ss << "[" << pItem->getSlot() << "] " << pItem->getLink();
		if (pItem->isStackable()) {
			ss << " (" << pItem->getStacks() << ")";
		}
		return ss.str();
	}

	const bool inspectCharacter(Character* pCharacter) {
		mInvoker->notify("Inspecting " + pCharacter->getName());

		auto f = [this](Item* pItem) {
			StringStream ss;			
			if (pItem) {
				ss << printItem(pItem);
				if (pItem->isContainer() && pItem->isEmpty() == false) {
					std::list<Item*> contents;
					pItem->getContents(contents);
					for (auto i : contents) {
						ss << printItem(i);
					}
				}
			}
			return ss.str();
		};

		// Worn.
		for (auto i = 0; i <= SlotID::AMMO; i++) {
			mInvoker->notify(f(pCharacter->getInventory()->getItem(i)));
		}

		// Main.
		for (uint32 i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
			auto item = pCharacter->getInventory()->getItem(i);
			mInvoker->notify(f(item));
		}

		// Cursor
		auto cursorItems = pCharacter->getInventory()->getCursor();
		for (auto i : cursorItems) {
			mInvoker->notify(f(i));
		}

		// Currency.


		return true;
	}

	const bool inspectNPC(NPC* pNPC) {
		mInvoker->notify("== Items ==");
		for (auto i : pNPC->getLootItems()) {
			mInvoker->notify(i->getName());
		}

		mInvoker->notify("== Currency ==");
		const int32 platinum = pNPC->getPlatinum();
		const int32 gold = pNPC->getGold();
		const int32 silver = pNPC->getSilver();
		const int32 copper = pNPC->getCopper();
		StringStream ss;
		ss << "P(" << platinum << ") G(" << gold << ") S" << silver << ") C(" << copper << ")";
		mInvoker->notify(ss.str());

		return true;
	}
};

/*****************************************************************************************************************************/
class WeatherCommand : public Command {
public:
	WeatherCommand(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
		mHelpMessages.push_back("Usage: #weather");
		mMinimumParameters = 3;
		mMaximumParameters = 3;
	};

	const bool handleCommand(CommandParameters pParameters) {
		uint32 a = 0;
		if (!convertParameter(0, a)) { return false; }
		uint32 b = 0;
		if (!convertParameter(1, b)) { return false; }
		uint32 c = 0;
		if (!convertParameter(2, c)) { return false; }

		auto packet = Payload::Zone::Weather::construct(a, b, c);
		mInvoker->getConnection()->sendPacket(packet);
		delete packet;

		return true;

		/*
		
		0,0,0 = clear

		0,2,1 = rain
		1,2,1 = snow (star snow flakes)
		2,2,1 = snow (round snow flakes)
		3,2,1 = clear

		2,1,1 = patchy snow

		1,31,1 = snow, seems heavy

		*/
	}
};

///*****************************************************************************************************************************/
//class YOURCOMMAND : public Command {
//public:
//	YOURCOMMAND(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : Command(pMinimumStatus, pAliases, pLogged) {
//		mHelpMessages.push_back("Usage: ");
//	};
//
//	const bool handleCommand(CommandParameters pParameters) {
//	}
//};

void CommandHandler::initialise() {
	mCommands.push_back(new WorldLockCommand(255, { "lock" }));

	mCommands.push_back(new ZoneCommand(100, { "zone", "z" }));
	mCommands.push_back(new WarpCommand(100, { "warp", "goto", "go" }));
	mCommands.push_back(new GMCommand(100, { "gm" }));
	mCommands.push_back(new ZoneListCommand(100, { "zonelist", "zlist" }));
	mCommands.push_back(new ZoneSearchCommand(100, { "zonesearch", "zs", "findzone", "fz" }));

	mCommands.push_back(new GuildSearchCommand(100, { "guildsearch", "gs", "findguild", "fg" }));
	mCommands.push_back(new GuildInformationCommand(100, { "ginfo" }));
	mCommands.push_back(new GuildPromoteCommand(0, {"guildpromote", "gpromote"}));
	mCommands.push_back(new GuildDemoteCommand(0, { "guilddemote", "gdemote" }));

	mCommands.push_back(new AddExperienceCommand(100, { "+xp", "+exp" "addexp" }));
	mCommands.push_back(new RemoveExperienceCommand(100, { "-xp", "-exp" "remexp" }));
	mCommands.push_back(new LevelCommand(100, { "level", "setlevel" "lvl" }));
	mCommands.push_back(new StatsCommand(100, { "setstat" }));

	mCommands.push_back(new WearChangeCommand(100, { "wc" }));
	mCommands.push_back(new LocationCommand(100, { "loc" }, false));

	mCommands.push_back(new SurnameCommand(100, { "surname" }));

	mCommands.push_back(new FindSpellCommand(100, { "findspell", "fs" }));

	mCommands.push_back(new SetSkillCommand(100, { "setskill" }));
	mCommands.push_back(new GetSkillCommand(100, { "getskill" }));
	mCommands.push_back(new SkillListCommand(100, { "skills" }));

	mCommands.push_back(new SetLanguageCommand(100, { "setlanguage" }));
	mCommands.push_back(new GetLanguageCommand(100, { "getlanguage" }));
	mCommands.push_back(new LanguageListCommand(100, { "languages" }));

	mCommands.push_back(new PopulateCommand(100, { "pop" }));
	mCommands.push_back(new DepopulateCommand(100, { "depop" }));
	mCommands.push_back(new RepopulateCommand(100, { "repop" }));

	mCommands.push_back(new KillCommand(100, { "kill" }));

	mCommands.push_back(new SummonItemCommand(100, { "si" }));
	mCommands.push_back(new SummonRandomItemCommand(100, { "sri" }));

	mCommands.push_back(new SummonCommand(100, { "summon" }));
	mCommands.push_back(new KickCommand(100, { "kick" }));
	mCommands.push_back(new InvulnerableCommand(100, { "invul" }));
	mCommands.push_back(new InspectCommand(100, { "inspect" }));
	mCommands.push_back(new WeatherCommand(100, { "weather" }));
}

void CommandHandler::command(Character* pCharacter, String pCommandMessage) {
	// Break up the message.
	std::vector<String> elements = Utility::split(pCommandMessage, ' ');
	// Extract command name.
	String commandName = elements[0].substr(1, elements[0].length() - 1);
	// Remove command name (#zone)
	elements.erase(elements.begin());

	Command* command = findCommand(commandName);
	if (command) {
		// Check: Invoker has the required status.
		if (command->getMinimumStatus() >= pCharacter->getStatus()) { return; }

		command->setInvoker(pCharacter);

		// Check: Invoker wants help.
		if (elements.size() == 1 && elements[0][0] == HELP_TOKEN) {
			command->helpMessage();
			command->setInvoker(nullptr);
			return;
		}

		// Check: Parameter range.
		if (elements.size() < command->getMinimumParameters() || elements.size() > command->getMaximumParameters()) {
			command->invalidParameters(elements);
			command->setInvoker(nullptr);
			return;
		}

		// Check: Target required.
		if (command->getRequiresTarget() && !pCharacter->hasTarget()) {
			pCharacter->notify("Command requires a target!");
			command->setInvoker(nullptr);
			return;
		}

		// Check: Log Command usage if required.
		if (command->isLogged())
			_logCommand(pCharacter, pCommandMessage);

		command->setParameters(elements);
		command->handleCommand(elements);
		command->setInvoker(nullptr);
		command->clearParameters();
	}
	else {
		// Hack/Test commands can be handled in here.
		_logCommand(pCharacter, pCommandMessage);
		_handleCommand(pCharacter, commandName, elements);
	}
}

void CommandHandler::_handleCommand(Character* pCharacter, String pCommandName, std::vector<String> pParameters) {

	// #heal
	if (pCommandName == "heal") {
		pCharacter->healPercentage(100);
	}
	// #damage <amount>
	else if (pCommandName == "damage" && pParameters.size() == 1 && pCharacter->hasTarget()) {
		unsigned int damage = 0;
		if (Utility::stou32Safe(damage, pParameters[0])) {
			auto packet = Payload::Zone::Damage::construct(pCharacter->getTarget()->getSpawnID(), pCharacter->getSpawnID(), damage, 1, 0);
			pCharacter->getConnection()->sendPacket(packet);
			safe_delete(packet);

		}
	}
	else if (pCommandName == "appearance") {
		if (pParameters.size() == 2) {
			uint32 appType = 0;
			uint32 appParameter = 0;
			if (Utility::stou32Safe(appType, pParameters[0]) && Utility::stou32Safe(appParameter, pParameters[1])) {
				pCharacter->getConnection()->sendAppearance(appType, appParameter);
			}
		}
	}
	// #anim <number>
	else if (pCommandName == "anim") {
		if (pParameters.size() == 2 && pCharacter->hasTarget()) {
			uint32 v0 = 0;
			uint32 v1 = 0;
			if (Utility::stou32Safe(v0, pParameters[0]) && Utility::stou32Safe(v1, pParameters[1])) {
				//pCharacter->doAnimation(animationID);
				pCharacter->getZone()->handleAnimation(pCharacter->getTarget(), v0, v1, true);
			}
		}
	}
	else if (pCommandName == "npc") {
		NPC* npc = new NPC();
		//uint8 at = 0;
		//Utility::stoSafe(at, pParameters[0]);
		//uint8 c = 0;
		//Utility::stoSafe(c, pParameters[0]);
		npc->setZone(pCharacter->getZone());
		npc->initialise();
		npc->setPosition(pCharacter->getPosition());
		//npc->setClass(c);
		//npc->setActorType(static_cast<ActorType>(at));
		//npc->setAATitle(static_cast<AATitle>(aa));
		//npc->setis
		pCharacter->getZone()->addActor(npc);
		pCharacter->notify(std::to_string(npc->getSpawnID()));
	}
	else if (pCommandName == "sn") {
		using namespace Payload::Zone;
		auto outPacket = new EQApplicationPacket(OP_GMLastName, SurnameUpdate::size());
		auto payload = SurnameUpdate::convert(outPacket->pBuffer);

		strcpy(payload->mCharaterName, pParameters[0].c_str());
		strcpy(payload->mGMName, pCharacter->getName().c_str());
		strcpy(payload->mLastName, pParameters[1].c_str());

		Utility::stoSafe(payload->mUnknown0[0], pParameters[2]);
		Utility::stoSafe(payload->mUnknown0[1], pParameters[3]);
		Utility::stoSafe(payload->mUnknown0[2], pParameters[4]);
		Utility::stoSafe(payload->mUnknown0[3], pParameters[5]);

		pCharacter->getConnection()->sendPacket(outPacket);
	}
	else if (pCommandName == "rn") {
		auto outPacket = new EQApplicationPacket(OP_MobRename, sizeof(MobRename_Struct));
		memset(outPacket->pBuffer, 0, sizeof(outPacket->pBuffer));
		auto payload = reinterpret_cast<MobRename_Struct*>(outPacket->pBuffer);
		strcpy(payload->old_name, pParameters[0].c_str());
		strcpy(payload->old_name_again, pParameters[1].c_str());
		strcpy(payload->new_name, pParameters[2].c_str());

		Utility::stoSafe(payload->unknown192, pParameters[3]);
		Utility::stoSafe(payload->unknown196, pParameters[4]);
		pCharacter->getConnection()->sendPacket(outPacket);

		safe_delete(outPacket);
	}
	else if (pCommandName == "title") {
		if (!pCharacter->hasTarget()) {
			return;
		}
		Actor* changeActor = pCharacter->getTarget();

		auto outPacket = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));
		auto payload = reinterpret_cast<SetTitleReply_Struct*>(outPacket->pBuffer);
		//payload->is_suffix = 1;
		Utility::stoSafe(payload->is_suffix, pParameters[1]);
		strcpy(payload->title, pParameters[0].c_str());
		payload->entity_id = changeActor->getSpawnID();

		pCharacter->getConnection()->sendPacket(outPacket);
		safe_delete(outPacket);

		//struct SetTitleReply_Struct {
		//	uint32	is_suffix;	//guessed: 0 = prefix, 1 = suffix
		//	char	title[32];
		//	uint32	entity_id;
		//};


	}
	else if (pCommandName == "sa") {
		EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* appearance = reinterpret_cast<SpawnAppearance_Struct*>(outPacket->pBuffer);
		appearance->spawn_id = pCharacter->getTarget()->getSpawnID();
		Utility::stoSafe(appearance->type, pParameters[0]);
		Utility::stoSafe(appearance->parameter, pParameters[1]);

		pCharacter->getConnection()->sendPacket(outPacket);
		safe_delete(outPacket);
	}
	else if (pCommandName == "setskill") {
		uint32 skillID = 0;
		uint32 skillValue = 0;
		Utility::stoSafe(skillID, pParameters[0]);
		Utility::stoSafe(skillValue, pParameters[1]);

		pCharacter->getConnection()->sendSkillValue(skillID, skillValue);
	}
	else if (pCommandName == "popup") {
		Utility::PopupHelper helper;

		helper.startColour(Utility::PopupHelper::RED);
		helper.writeBr("=*=*=*=*=*=*=*=*=*=*=*=*=*=*=");
		helper.endColour();
		helper.startColour(Utility::PopupHelper::GREEN);
		helper.writeBr("=*=*=*=*=*=*=*=*=*=*=*=*=*=*=");
		helper.endColour();
		helper.startColour(Utility::PopupHelper::YELLOW);
		helper.writeBr("=*=*=*=*=*=*=*=*=*=*=*=*=*=*=");
		helper.endColour();


		pCharacter->getConnection()->sendPopup("TEST", helper.getText());
	}
	else if (pCommandName == "corpse") {
		uint8 response = 0;
		uint8 unk0 = 0;
		uint8 unk1 = 0;
		uint8 unk2 = 0;

		Utility::stoSafe(response, pParameters[0]);
		Utility::stoSafe(unk0, pParameters[1]);
		Utility::stoSafe(unk1, pParameters[2]);
		Utility::stoSafe(unk2, pParameters[3]);

		EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoneyOnCorpse, sizeof(moneyOnCorpseStruct));
		moneyOnCorpseStruct* d = (moneyOnCorpseStruct*)outapp->pBuffer;
		d->response = response;
		d->unknown1 = unk0;
		d->unknown2 = unk1;
		d->unknown3 = unk2;

		// 3 = LOOT
		// 4 = "You cannot loot while a hostile create is aware of your presence"
		// 5 = "You are too far away to loot that corpse"
		// 6 = Nothing
		// 7 = Nothing
		// 8 = LOOT
		// 9 = LOOT
		// 10 = LOOT

		pCharacter->getConnection()->sendPacket(outapp);
		safe_delete(outapp);
	}
	else if (pCommandName == "trade") {
		uint32 fromSpawnID = 0;
		Utility::stoSafe(fromSpawnID, pParameters[0]);
		pCharacter->getConnection()->sendTradeRequest(fromSpawnID);
	}
	else if (pCommandName == "linky") {
		std::stringstream ss;
		ss << "\x12";
		ss << std::setw(1) << std::hex << 0; // Unknown ?
		ss << std::setw(5) << std::hex << std::setfill('0') << 10001; // Item ID
		ss << std::setw(5) << std::hex << std::setfill('0') << 0; // Augment 0
		ss << std::setw(5) << std::hex << std::setfill('0') << 0; // Augment 1
		ss << std::setw(5) << std::hex << std::setfill('0') << 0; // Augment 2
		ss << std::setw(5) << std::hex << std::setfill('0') << 0; // Augment 3
		ss << std::setw(5) << std::hex << std::setfill('0') << 0; // Augment 4
		ss << std::setw(1) << std::hex << std::setfill('0') << 1; // Evolving
		ss << std::setw(4) << std::hex << std::setfill('0') << 0; // Lore Group
		ss << std::setw(1) << std::hex << std::setfill('0') << 1; // Evolve Level
		ss << std::setw(5) << std::hex << std::setfill('0') << 0; // Unknown ?
		ss << std::setw(8) << std::hex << std::setfill('0') << 0; // Hash
		ss << "Hello World";
		ss << "\x12";

		pCharacter->getConnection()->sendMessage(MessageType::White, ss.str());


	}
	else if (pCommandName == "info") {
		auto target = pCharacter->getTarget();
		if (target) {
			pCharacter->notify(target->getName() + " ActorID = " + std::to_string(target->getSpawnID()));
		}
	}

	else if (pCommandName == "del") {
		uint32 stacks = 0;
		Utility::stoSafe(stacks, pParameters[0]);
		pCharacter->getConnection()->sendMoveItem(SlotID::CURSOR, SlotID::SLOT_DELETE, stacks);
	}
	else if (pCommandName == "time") {
		uint8 hour = 0;
		Utility::stoSafe(hour, pParameters[0]);

		auto packet = Payload::Zone::Time::create();
		auto payload = Payload::Zone::Time::convert(packet);
		payload->mHour = hour;

		pCharacter->getConnection()->sendPacket(packet);
		delete packet;
	}
	else {
		pCharacter->message(MessageType::Yellow, "Unknown command.");
	}
}

Command* CommandHandler::findCommand(String pCommandName) {
	for (auto i : mCommands) {
		for (auto j : i->getAliases()) {
			if (j == pCommandName)
				return i;
		}
	}

	return nullptr;
}

CommandHandler::~CommandHandler() {
	mCommands.remove_if(Utility::containerEntryDelete<Command*>);
}

void CommandHandler::_logCommand(Character* pCharacter, String pCommandMessage) {
	StringStream ss;
	ss << "[Command] Invoker(" << pCharacter->getName() << ")";
	if (pCharacter->hasTarget())
		ss << " Target(" << pCharacter->getTarget()->getName() << ")";

	ss << " Text(" << pCommandMessage << ")";

	Log::info(ss.str());
}

void Command::invalidParameters(CommandParameters pParameters) {
	mInvoker->getConnection()->sendMessage(MessageType::Red, "Command parameters were invalid.");
}

void Command::conversionError(String& pParameter) {
	mInvoker->getConnection()->sendMessage(MessageType::Red, "Command parameter conversion failed for" + pParameter);
}

void Command::helpMessage() {
	if (mHelpMessages.size() == 0){
		mInvoker->getConnection()->sendMessage(MessageType::Yellow, "This command has no help message.");
		return;
	}

	for (auto i : mHelpMessages)
		mInvoker->getConnection()->sendMessage(MessageType::Yellow, i);
}
