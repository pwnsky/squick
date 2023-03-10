set(ProjectName "t3_hello_event")
file(GLOB Tutorial3_ROOT_CC
	*.cc)
file(GLOB Tutorial3_ROOT_H
	*.h)

add_library(t3_hello_event SHARED
	${Tutorial3_ROOT_CC}
	${Tutorial3_ROOT_H})
	
set_target_properties( t3_hello_event PROPERTIES OUTPUT_NAME "hello_event")
set_target_properties( t3_hello_event PROPERTIES PREFIX "")
set_target_properties( t3_hello_event PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	RUNTIME_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	LIBRARY_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial )
add_definitions(
	-D_USRDLL
	-DELPP_NO_DEFAULT_LOG_FILE
)

