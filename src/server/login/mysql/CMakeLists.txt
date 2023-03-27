set(ProjectName "login_mysql")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})

if (APPLE)
	
elseif(UNIX)
	target_link_libraries(${ProjectName} libmysqlcppconn8.so)
else()
	target_link_libraries(${ProjectName} mysqlcppconn8.lib)
	target_link_libraries(${ProjectName} mysqlcppconn.lib)
endif()

LinkSquick(${ProjectName})
SquickOut(${ProjectName} "server/login" ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/login "mysql")