set(ProjectName "db_proxy_mongo")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})

	
if (APPLE)
	target_link_libraries(${ProjectName} libmongocxx.dylib)
	target_link_libraries(${ProjectName} bsoncxx.dylib)
elseif(UNIX)
	target_link_libraries(${ProjectName} libmongocxx.so)
	target_link_libraries(${ProjectName} bsoncxx.so)
else()
	target_link_libraries(${ProjectName} mongocxx.lib)
	target_link_libraries(${ProjectName} bsoncxx.lib)
endif()

LinkSquick(${ProjectName})
SquickOut(${ProjectName} "node/db_proxy" ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/db_proxy "mongo")
