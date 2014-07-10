CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SOURCE_GROUP(Data FILES Data.h DataStore.h DataStore.cpp DataProvider.h MySQLDataProvider.h MySQLDataProvider.cpp)
SET(world_sources
	WorldClientConnection.cpp
	CMakeLists.txt
	LoginServerConnection.cpp
	net.cpp
	UCSConnection.cpp
	WorldConfig.cpp
	worlddb.cpp
	Utility.cpp
	ZoneManager.cpp
	Zone.cpp
	Character.cpp
	World.cpp
	DataStore.cpp
	MySQLDataProvider.cpp
	AccountManager.cpp
	DatabaseConnection.cpp
)

SET(world_headers
	WorldClientConnection.h
	CMakeLists.txt
	LoginServerConnection.h
	SoFCharCreateData.h
	UCSConnection.h
	WorldConfig.h
	worlddb.h
	WorldTCPConnection.h
	Utility.h
	ZoneManager.h
	Zone.h
	Character.h
	World.h
	DataStore.h
	DataProvider.h
	MySQLDataProvider.h
	AccountManager.h
	DatabaseConnection.h
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
