CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SOURCE_GROUP(Test FILES Testing.h Test_AccountManager.h)
SOURCE_GROUP(Data FILES Data.h IDataStore.h XMLDataStore.h XMLDataStore.cpp)
SOURCE_GROUP(Zone FILES ZoneManager.h ZoneManager.cpp Zone.h Zone.cpp Actor.h Actor.cpp Character.h Character.cpp NPC.h NPC.cpp ZoneData.h ZoneData.cpp NPCFactory.h NPCFactory.cpp)
SOURCE_GROUP(Zone\\Hate FILES HateControllerFactory.h HateControllerFactory.cpp HateController.h HateController.cpp)
SOURCE_GROUP(Zone\\Experience FILES ExperienceCalculator.h ExperienceCalculator.cpp ExperienceController.h ExperienceController.cpp)
SOURCE_GROUP(Zone\\Group FILES GroupConstants.h Group.h Group.cpp GroupManager.h GroupManager.cpp)
SOURCE_GROUP(Zone\\Raid FILES RaidManager.h Raid.h)
SOURCE_GROUP(Zone\\Guild FILES Guild.h Guild.cpp GuildManager.h GuildManager.cpp)
SOURCE_GROUP(Zone\\Command FILES CommandHandler.h CommandHandler.cpp)
SOURCE_GROUP(Zone\\ZoneConnection FILES ZoneConnection.h ZoneConnection.cpp ZoneConnectionManager.h ZoneConnectionManager.cpp)
SOURCE_GROUP(Zone\\Spells FILES SpellContants.h SpellDataStore.h SpellDataStore.cpp)
SOURCE_GROUP(UCS FILES UCS.h UCS.cpp UCSClientConnection.h UCSClientConnection.cpp)
SOURCE_GROUP(HateController FILES HateControllerFactory.h HateControllerFactory.cpp)

SET(world_sources
	CMakeLists.txt
	WorldConnection.cpp
	LoginServerConnection.cpp
	main.cpp
	Utility.cpp
	ZoneManager.cpp
	Zone.cpp
	Character.cpp
	World.cpp
	XMLDataStore.cpp
	AccountManager.cpp
	LogSystem.cpp
	CommandHandler.cpp
	Profile.cpp
	ZoneConnection.cpp
	ZoneData.cpp
	GroupManager.cpp
	GuildManager.cpp
	RaidManager.cpp
	Limits.cpp
	NPC.cpp
	Scene.cpp
	Actor.cpp
	Settings.cpp
	UCS.cpp
	UCSClientConnection.cpp
	TitleManager.cpp
	SpellDataStore.cpp
	SpawnPoint.cpp
	SpawnPointManager.cpp
	Item.cpp
	Inventory.cpp
	ItemGenerator.cpp
	NPCFactory.cpp
	CombatSystem.cpp
	LootAllocator.cpp
	EventDispatcher.cpp
	ItemDataStore.cpp
	ItemFactory.cpp
	Transmutation.cpp
	AlternateCurrencyManager.cpp
	ShopDataStore.cpp
	Random.cpp
	DataValidation.cpp
	SpawnGroup.cpp
	HateControllerFactory.cpp
	HateController.cpp
	ExperienceCalculator.cpp
	ExperienceController.cpp
	LootController.cpp
	ServiceLocator.cpp
	ZoneConnectionManager.cpp
	Account.cpp
	Payload.cpp
	CharacterFactory.cpp
	Guild.cpp
	Group.cpp
)

SET(world_headers
	WorldConnection.h
	LoginServerConnection.h
	Utility.h
	ZoneManager.h
	Zone.h
	Character.h
	World.h
	IDataStore.h
	XMLDataStore.h
	AccountManager.h
	Constants.h
	LogSystem.h
	CommandHandler.h
	Profile.h
	ZoneConnection.h
	ClientAuthentication.h
	ZoneData.h
	GroupConstants.h
	Group.h
	GroupManager.h
	GuildManager.h
	Guild.h
	RaidManager.h
	Limits.h
	Actor.h
	NPC.h
	Payload.h
	Data.h
	Scene.h
	Vector3.h
	SpawnPoint.h
	SpawnPointManager.h
	Settings.h
	UCS.h
	UCSClientConnection.h
	Singleton.h
	TitleManager.h
	SpellContants.h
	SpellDataStore.h
	ItemData.h
	Item.h
	Inventory.h
	ItemGenerator.h
	NPCFactory.h
	Timer.h
	TimeUtility.h
	CombatSystem.h
	LootAllocator.h
	EventDispatcher.h
	ItemDataStore.h
	ItemFactory.h
	Bonuses.h
	Transmutation.h
	Testing.h
	Test_AccountManager.h
	AlternateCurrencyManager.h
	ShopDataStore.h
	Random.h
	DataValidation.h
	SpawnGroup.h
	Types.h
	HateControllerFactory.h
	HateController.h
	ExperienceCalculator.h
	ExperienceController.h
	LootController.h
	Raid.h
	ServiceLocator.h
	ZoneConnectionManager.h
	Account.h
	CharacterFactory.h
)

ADD_EXECUTABLE(world ${world_sources} ${world_headers})

INSTALL(TARGETS world RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

ADD_DEFINITIONS(-DWORLD)

TARGET_LINK_LIBRARIES(world Common ${PERL_LIBRARY} debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY})

IF(MSVC)
	SET_TARGET_PROPERTIES(world PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(world "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(world "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	TARGET_LINK_LIBRARIES(world "${CMAKE_DL_LIBS}")
	TARGET_LINK_LIBRARIES(world "z")
	TARGET_LINK_LIBRARIES(world "m")
	IF(NOT DARWIN)
		TARGET_LINK_LIBRARIES(world "rt")
	ENDIF(NOT DARWIN)
	TARGET_LINK_LIBRARIES(world "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/Bin)
