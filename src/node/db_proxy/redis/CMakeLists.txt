set(ProjectName "db_proxy_redis")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})

	
if (APPLE)
	
elseif(UNIX)
	target_link_libraries(${ProjectName} hiredis)
	target_link_libraries(${ProjectName} redis++)
else()
	target_link_libraries(${ProjectName} hiredisd)
	target_link_libraries(${ProjectName} redis++)
endif()

LinkSquick(${ProjectName})
SquickOut(${ProjectName} "node/db_proxy" ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/db_proxy "redis")
