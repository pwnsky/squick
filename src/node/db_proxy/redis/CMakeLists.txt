set(ProjectName "db_proxy_redis")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})

	
if (APPLE)
	
elseif(UNIX)
	target_link_libraries(${ProjectName} hiredisd.so)
else()
	target_link_libraries(${ProjectName} hiredisd.lib)
	target_link_libraries(${ProjectName} redis++.lib)
endif()

LinkSquick(${ProjectName})
SquickOut(${ProjectName} "node/db_proxy" ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/db_proxy "redis")
