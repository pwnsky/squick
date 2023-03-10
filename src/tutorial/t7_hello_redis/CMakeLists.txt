set(ProjectName "t7_hello_redis")
file(GLOB Tutorial7_ROOT_CC
	*.cc)
file(GLOB Tutorial7_ROOT_H
	*.h)

add_library(t7_hello_redis SHARED
	${Tutorial7_ROOT_CC}
	${Tutorial7_ROOT_H})
	
set_target_properties( t7_hello_redis PROPERTIES OUTPUT_NAME "hello_redis")
set_target_properties( t7_hello_redis PROPERTIES PREFIX "")
set_target_properties( t7_hello_redis PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	RUNTIME_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	LIBRARY_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial )
add_definitions(
	-D_USRDLL
	-DELPP_NO_DEFAULT_LOG_FILE
)

