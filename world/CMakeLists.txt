CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(world_sources
	client.cpp
	cliententry.cpp
	clientlist.cpp
	CMakeLists.txt
	console.cpp
	EQLConfig.cpp
	LauncherLink.cpp
	LauncherList.cpp
	lfplist.cpp
	LoginServer.cpp
	LoginServerList.cpp
	net.cpp
	ucs.cpp
	wguild_mgr.cpp
	WorldConfig.cpp
	worlddb.cpp
	zonelist.cpp
	zoneserver.cpp
	Utility.cpp
	ZoneManager.cpp
	Zone.cpp
	Character.cpp
	World.cpp
)

SET(world_headers
	client.h
	cliententry.h
	clientlist.h
	CMakeLists.txt
	console.h
	EQLConfig.h
	LauncherLink.h
	LauncherList.h
	lfplist.h
	LoginServer.h
	LoginServerList.h
	SoFCharCreateData.h
	ucs.h
	wguild_mgr.h
	WorldConfig.h
	worlddb.h
	WorldTCPConnection.h
	zonelist.h
	zoneserver.h
	Utility.h
	ZoneManager.h
	Zone.h
	Character.h
	World.h
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
