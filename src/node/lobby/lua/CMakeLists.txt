set(ProjectName "lobby_lua")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})

if (APPLE)

elseif(UNIX)
	target_link_libraries(${ProjectName} liblua.a )
else()
	target_link_libraries(${ProjectName} lua.lib )
endif()

LinkSquick(${ProjectName})
SquickOut(${ProjectName} "node/lobby" ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/lobby "lua")