set(ProjectName "plugin_server_www_logic")
file(GLOB plugin_server_www_logic_cc
	*.cc)

file(GLOB plugin_server_www_logic_h 
	*.h)

add_library(plugin_server_www_logic SHARED
	${plugin_server_www_logic_cc}
	${plugin_server_www_logic_h}
)

set_target_properties( plugin_server_www_logic PROPERTIES OUTPUT_NAME "logic")
set_target_properties( plugin_server_www_logic PROPERTIES PREFIX "")
set_target_properties( plugin_server_www_logic PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/www
	RUNTIME_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/www
	LIBRARY_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/www )

add_definitions(
	-DELPP_NO_DEFAULT_LOG_FILE
)
