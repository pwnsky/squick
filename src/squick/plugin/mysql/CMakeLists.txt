set(ProjectName "core_mysql")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})
	
if (APPLE)
	
elseif(UNIX)
	target_link_libraries(${ProjectName} mysqlcppconn8.a)
else()
	target_link_libraries(${ProjectName} mysqlcppconn8.lib)
	target_link_libraries(${ProjectName} mysqlcppconn.lib)
endif()

LinkSquick(${ProjectName})
SquickOut(${ProjectName} "squick/plugin" ${SQUICK_BUILD_PLUGIN_CORE_PATH} "mysql")
