set(ProjectName "t4_hello_async")
file(GLOB Tutorial4_ROOT_CC
	*.cc)
file(GLOB Tutorial4_ROOT_H
	*.h)

add_library(t4_hello_async SHARED
	${Tutorial4_ROOT_CC}
	${Tutorial4_ROOT_H})
	
set_target_properties( t4_hello_async PROPERTIES OUTPUT_NAME "hello_async")
set_target_properties( t4_hello_async PROPERTIES PREFIX "")
set_target_properties( t4_hello_async PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	RUNTIME_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	LIBRARY_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial )
add_definitions(
	-D_USRDLL
	-DELPP_NO_DEFAULT_LOG_FILE
)

