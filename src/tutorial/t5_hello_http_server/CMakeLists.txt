set(ProjectName "t5_hello_http_server")
file(GLOB Tutorial5_ROOT_CC
	*.cc)
file(GLOB Tutorial5_ROOT_H
	*.h)

add_library(t5_hello_http_server SHARED
	${Tutorial5_ROOT_CC}
	${Tutorial5_ROOT_H})
	
set_target_properties( t5_hello_http_server PROPERTIES OUTPUT_NAME "hello_http_server")
set_target_properties( t5_hello_http_server PROPERTIES PREFIX "")
set_target_properties( t5_hello_http_server PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	RUNTIME_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	LIBRARY_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial )
add_definitions(
	-D_USRDLL
	-DELPP_NO_DEFAULT_LOG_FILE
)

