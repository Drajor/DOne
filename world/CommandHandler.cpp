#include "CommandHandler.h"
#include "Character.h"
#include "NPC.h"
#include "Zone.h"
#include "ZoneData.h"
#include "ZoneManager.h"
#include "GuildManager.h"
#include "Utility.h"
#include "LogSystem.h"
#include <vector>
#include <sstream>
#include "ZoneClientConnection.h"
#include "Constants.h"
#include "Limits.h"

#define PACKET_PLAY
#ifdef PACKET_PLAY
#include "../common/eq_packet_structs.h"
#include "../common/EQPacket.h"
#include "Payload.h"
#endif

// Thank you: http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
std::vector<String> &split(const String &s, char delim, std::vector<String> &elems) {
	StringStream ss(s);
	String item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

// Thank you: http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
std::vector<String> split(const String &s, char delim) {
	std::vector<String> elems;
	split(s, delim, elems);
	return elems;
}

/*****************************************************************************************************************************/
class ZoneCommand : public Command {
public:
	ZoneCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #zone <Zone ID> <Zone Instance ID>";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		// Check: Parameters
		if (pParameters.size() != 2) {
			invalidParameters(pCharacter, pParameters);
			return;
		}

		uint32 zoneID = 0;
		uint32 instanceID = 0;
		if (Utility::stou32Safe(zoneID, pParameters[0]) && Utility::stou32Safe(instanceID, pParameters[1])) {
			pCharacter->getConnection()->sendRequestZoneChange(zoneID, instanceID);
		}
	}
};

/*****************************************************************************************************************************/
class WarpCommand : public Command {
public:
	WarpCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #warp <X> <Y> <Z>";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		if (pParameters.empty()) {
			Actor* target = pCharacter->getTarget();
			if (!target || !target->isCharacter()) { return; }
			Character* characterTarget = static_cast<Character*>(target);
			pCharacter->message(MessageType::Yellow, "Moving to " + characterTarget->getName() + ".");
			pCharacter->getZone()->moveCharacter(pCharacter, characterTarget->getX(), characterTarget->getY(), characterTarget->getZ());
			return;
		}
		// Check: Parameters
		if (pParameters.size() != 3) {
			invalidParameters(pCharacter, pParameters);
			return;
		}

		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		if (Utility::stofSafe(x, pParameters[0]) && Utility::stofSafe(y, pParameters[1]) && Utility::stofSafe(z, pParameters[2])) {
			pCharacter->getZone()->moveCharacter(pCharacter, x, y, z);
		}
		else {
			pCharacter->message(MessageType::Red, "There was a problem with your parameters.");
		}
	}
};

/*****************************************************************************************************************************/
class GMCommand : public Command {
public:
	GMCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #gm on / #gm off";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		// Check: Parameters
		if (pParameters.size() != 1) {
			invalidParameters(pCharacter, pParameters);
			return;
		}

		bool gm = pParameters[0] == "on";
		pCharacter->setIsGM(gm);
		pCharacter->getZone()->notifyCharacterGM(pCharacter);
	}
};

/*****************************************************************************************************************************/
class ZoneListCommand : public Command {
public:
	ZoneListCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #zonelist";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		ZoneSearchResult result = ZoneManager::getInstance().getAllZones();
		for (auto i : result) {
			StringStream ss; ss << "[Zone] " << i.mName << "(" << i.mID << "," << i.mInstanceID << ") Players: " << i.mNumCharacters;
			pCharacter->getConnection()->sendMessage(MessageType::Aqua, ss.str());
		}
	}
};

/*****************************************************************************************************************************/
class AddExperienceCommand : public Command {
public:
	AddExperienceCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #addexp <number>";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		// Check: Parameters
		if (pParameters.size() != 1) {
			invalidParameters(pCharacter, pParameters);
			return;
		}

		uint32 expAdd = 0;
		if (Utility::stou32Safe(expAdd, pParameters[0])) {
			pCharacter->addExperience(expAdd);
		}
	}
};

/*****************************************************************************************************************************/
class RemoveExperienceCommand : public Command {
public:
	RemoveExperienceCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #remexp <number>";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		// Check: Parameters
		if (pParameters.size() != 1) {
			invalidParameters(pCharacter, pParameters);
			return;
		}

		uint32 expRemove = 0;
		if (Utility::stou32Safe(expRemove, pParameters[0])) {
			pCharacter->removeExperience(expRemove);
		}
	}
};

/*****************************************************************************************************************************/
class LocationCommand : public Command {
public:
	LocationCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #loc";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		StringStream ss;
		ss << "Your location is " << pCharacter->getX() << ", " << pCharacter->getY() << ", " << pCharacter->getZ();
		pCharacter->message(MessageType::White, ss.str());
	}
};

/*****************************************************************************************************************************/
class LevelCommand : public Command {
public:
	LevelCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #level <number>";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		// Check: Parameters
		if (pParameters.size() != 1) {
			invalidParameters(pCharacter, pParameters);
			return;
		}

		uint32 level = 0;
		if (Utility::stou32Safe(level, pParameters[0])) {
			pCharacter->setLevel(static_cast<uint8>(level));
		}
	}
};

/*****************************************************************************************************************************/
class StatsCommand : public Command {
public:
	StatsCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage Example: #setstat str 10";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		// Check: Parameters
		if (pParameters.size() != 2) {
			invalidParameters(pCharacter, pParameters);
			return;
		}

		String statStr = pParameters[0];
		String statName = "unknown";
		uint32 value = 0;
		Statistic statistic;
		if (!Utility::stou32Safe(value, pParameters[1])) {
			return;
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
			return;
		}

		StringStream ss;
		ss << "Changing " << pCharacter->getName() << "'s " << statName << " from " << pCharacter->getBaseStatistic(statistic) << " to " << value;
		pCharacter->message(MessageType::LightGreen, ss.str());
		pCharacter->setBaseStatistic(statistic, value);
	}
};

/*****************************************************************************************************************************/
class ZoneSearchCommand : public Command {
public:
	ZoneSearchCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #zs <text>";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		// Check: Parameters
		if (pParameters.size() != 1) {
			invalidParameters(pCharacter, pParameters);
			return;
		}

		ZoneDataSearchResults results = ZoneData::getInstance().searchByName(pParameters[0]);
		pCharacter->message(MessageType::Yellow, "Search found " + std::to_string(results.size()) + " results.");
		for (auto i : results){
			pCharacter->message(MessageType::Yellow, "[Zone " + std::to_string(i.mID) + "] " + i.mShortName + " | " + i.mLongName );
		}
	}
};

/*****************************************************************************************************************************/
class GuildSearchCommand : public Command {
public:
	GuildSearchCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #guildsearch <optional text>";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		// Get all guilds.
		if (pParameters.size() == 0) {
			GuildSearchResults results = GuildManager::getInstance().getAllGuilds();
			for (auto i : results){
				pCharacter->message(MessageType::Yellow, "[Guild " + std::to_string(i.mID) + "] " + i.mName);
			}
		}
		// Search guilds.
		else if (pParameters.size() == 1) {
		}
		else {
			invalidParameters(pCharacter, pParameters);
			return;
		}
	}
};

/*****************************************************************************************************************************/
class GuildInformationCommand : public Command {
public:
	GuildInformationCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #ginfo";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		if (pCharacter->hasGuild()) {
			static const String RankNames[] = { "Member", "Officer", "Leader" };
			pCharacter->message(MessageType::Yellow, RankNames[pCharacter->getGuildRank()] + " of " + std::to_string(pCharacter->getGuildID()));
		}
		else {
			pCharacter->message(MessageType::Yellow, "No Guild");
		}
	}
};

/*****************************************************************************************************************************/
// Allows for cross zone and offline guild promotion.
class GuildPromoteCommand : public Command {
public:
	GuildPromoteCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #guildpromote <name>";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		if (pParameters.size() == 1 && Limits::Character::nameLength(pParameters[0]) ) {
			if (pCharacter->hasGuild() && pCharacter->getGuildRank() == GuildRanks::Leader) {
				GuildManager::getInstance().handlePromote(pCharacter, pParameters[0]);
			}
			else {
				pCharacter->message(MessageType::Yellow, "No guild or not guild leader.");
			}
		}
	}
};

/*****************************************************************************************************************************/
// Allows for cross zone and offline guild demotion.
class GuildDemoteCommand : public Command {
public:
	GuildDemoteCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #guilddemote <name>";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		if (pParameters.size() == 1 && Limits::Character::nameLength(pParameters[0])) {
			if (pCharacter->hasGuild() && pCharacter->getGuildRank() == GuildRanks::Leader) {
				GuildManager::getInstance().handleDemote(pCharacter, pParameters[0]);
			}
			else {
				pCharacter->message(MessageType::Yellow, "No guild or not guild leader.");
			}
		}
	}
};

class WearChangeCommand : public Command {
public:
	WearChangeCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #wc <slot> <material> <colour>";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		if (pParameters.size() == 3) {
			uint32 slotID = 0;
			uint32 materialID = 0;
			uint32 colour = 0;
			bool ok = Utility::stou32Safe(slotID, pParameters[0]) && Utility::stou32Safe(materialID, pParameters[1]) && Utility::stou32Safe(colour, pParameters[2]);
			if (ok) {
				pCharacter->getConnection()->sendWearChange(pCharacter->getSpawnID(), slotID, materialID, colour);
			}
		}
	}
};

class SurnameCommand : public Command {
public:
	SurnameCommand(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
		mHelpMessage = "Usage: #surname <name>";
	};

	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
		if (pParameters.size() == 1) {
			const bool hasTarget = pCharacter->hasTarget();

			// Check: Has a target.
			if (!hasTarget) {
				pCharacter->notify("You must target an actor.");
				return;
			}
			Actor* changeActor = pCharacter->getTarget();

			// Check: Length.
			if (!Limits::Character::surnameLengthClient(pParameters[0])) {
				pCharacter->notify("Length invalid");
				return;
			}
			changeActor->setLastName(pParameters[0]);
			changeActor->getZone()->handleSurnameChange(changeActor);
		}
	}
};


///*****************************************************************************************************************************/
//class YOURCOMMAND : public Command {
//public:
//	YOURCOMMAND(uint8 pMinimumStatus, std::list<String> pAliases) : Command(pMinimumStatus, pAliases) {
//		mHelpMessage = "Usage: ";
//	};
//
//	void handleCommand(Character* pCharacter, CommandParameters pParameters) {
//		// Check: Parameters
//		if (pParameters.size() != 1) {
//			invalidParameters(pCharacter, pParameters);
//			return;
//		}
//	}
//};

void CommandHandler::initialise() {
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
	mCommands.push_back(new LocationCommand(100, { "loc" }));

	mCommands.push_back(new SurnameCommand(100, { "surname" }));
}

void CommandHandler::command(Character* pCharacter, String pCommandMessage) {
	// Break up the message.
	std::vector<String> elements = split(pCommandMessage, ' ');
	// Extract command name.
	String commandName = elements[0].substr(1, elements[0].length() - 1);
	// Remove command name (#zone)
	elements.erase(elements.begin());

	Command* command = findCommand(commandName);
	if (command) {
		// Check status and return silently if required.
		if (command->getMinimumStatus() > pCharacter->getStatus()) { return; }

		// Check if the user wants help
		if (elements.size() == 1 && elements[0][0] == HELP_TOKEN) {
			command->helpMessage(pCharacter);
			return;
		}

		command->handleCommand(pCharacter, elements);
	}
	else {
		// Hack/Test commands can be handled in here.
		_handleCommand(pCharacter, commandName, elements);
	}
}

void CommandHandler::_handleCommand(Character* pCharacter, String pCommandName, std::vector<String> pParameters) {

	// #heal
	if (pCommandName == "heal") {
		pCharacter->healPercentage(100);
	}
	// #damage <amount>
	else if (pCommandName == "damage" && pParameters.size() == 1) {
		unsigned int damage = 0;
		if (Utility::stou32Safe(damage, pParameters[0])) {
			pCharacter->damage(damage);
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
		if (pParameters.size() == 1) {
			uint32 animationID = 0;
			if (Utility::stou32Safe(animationID, pParameters[0])) {
				pCharacter->doAnimation(animationID);
			}
		}
	}
	else if (pCommandName == "npc") {
		NPC* npc = new NPC();
		npc->setZone(pCharacter->getZone());
		npc->initialise();
		npc->setPosition(pCharacter->getPosition());
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

void Command::invalidParameters(Character* pCharacter, CommandParameters pParameters) {
	pCharacter->getConnection()->sendMessage(MessageType::Red, "Command parameters were invalid.");
}

void Command::helpMessage(Character* pCharacter) {
	if (mHelpMessage.empty()){
		pCharacter->getConnection()->sendMessage(MessageType::Yellow, "This command has no help message.");
		return;
	}

	pCharacter->getConnection()->sendMessage(MessageType::Yellow, mHelpMessage);
}
