CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SOURCE_GROUP(Data FILES Data.h DataStore.h DataStore.cpp zone_data.xml)
SOURCE_GROUP(Zone FILES ZoneManager.h ZoneManager.cpp Zone.h Zone.cpp Actor.h Actor.cpp Character.h Character.cpp NPC.h NPC.cpp ZoneData.h ZoneData.cpp SpellDataStore.h SpellDataStore.cpp NPCFactory.h NPCFactory.cpp)
SOURCE_GROUP(UCS FILES UCS.h UCS.cpp UCSClientConnection.h UCSClientConnection.cpp)

SET(world_sources
	CMakeLists.txt
	WorldClientConnection.cpp
	LoginServerConnection.cpp
	main.cpp
	Utility.cpp
	ZoneManager.cpp
	Zone.cpp
	Character.cpp
	World.cpp
	DataStore.cpp
	AccountManager.cpp
	LogSystem.cpp
	CommandHandler.cpp
	Profile.cpp
	ZoneClientConnection.cpp
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
	Item.cpp
	Inventory.cpp
	ItemGenerator.cpp
	StaticItemData.cpp
	NPCFactory.cpp
	CombatSystem.cpp
	LootAllocator.cpp
	EventDispatcher.cpp
	ItemDataStore.cpp
	ItemFactory.cpp
	Transmutation.cpp
	AlternateCurrencyManager.cpp
)

SET(world_headers
	WorldClientConnection.h
	LoginServerConnection.h
	Utility.h
	ZoneManager.h
	Zone.h
	Character.h
	World.h
	DataStore.h
	AccountManager.h
	Constants.h
	LogSystem.h
	CommandHandler.h
	Profile.h
	ZoneClientConnection.h
	ClientAuthentication.h
	ZoneData.h
	GroupManager.h
	GuildManager.h
	RaidManager.h
	Limits.h
	Actor.h
	NPC.h
	Payload.h
	Data.h
	Scene.h
	Vector3.h
	SpawnPoint.h
	Settings.h
	UCS.h
	UCSClientConnection.h
	Singleton.h
	TitleManager.h
	SpellDataStore.h
	ItemData.h
	Item.h
	Inventory.h
	ItemGenerator.h
	StaticItemData.h
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
	AlternateCurrencyManager.h
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
