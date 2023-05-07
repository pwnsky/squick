set(ProjectName "lobby_mongo")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})

	
if (APPLE)
	
elseif(UNIX)
	target_link_libraries(${ProjectName} libmongocxx.so)
else()
	target_link_libraries(${ProjectName} mongocxx.lib)
	target_link_libraries(${ProjectName} bsoncxx.lib)
endif()

LinkSquick(${ProjectName})
SquickOut(${ProjectName} "node/lobby" ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/lobby "mongo")
