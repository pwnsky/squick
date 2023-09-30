set(ProjectName "db_proxy_clickhouse")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})


include_directories( ${SolutionDir}/third_party/clickhouse-cpp/contrib/absl )
	
if (APPLE)
	
elseif(UNIX)
	target_link_libraries(${ProjectName} clickhouse-cpp-lib)
else()
	target_link_libraries(${ProjectName} clickhouse-cpp-lib)
	target_link_libraries(${ProjectName} ws2_32)
	target_link_libraries(${ProjectName} lz4)
	target_link_libraries(${ProjectName} absl_int128)
	target_link_libraries(${ProjectName} cityhash)
endif()

LinkSquick(${ProjectName})
SquickOut(${ProjectName} "node/db_proxy" ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/db_proxy "clickhouse")
