set(ProjectName "t6_hello_navigation")
file(GLOB Tutorial6_ROOT_CC
	*.cc)
file(GLOB Tutorial6_ROOT_H
	*.h)

add_library(t6_hello_navigation SHARED
	${Tutorial6_ROOT_CC}
	${Tutorial6_ROOT_H})
	
set_target_properties( t6_hello_navigation PROPERTIES OUTPUT_NAME "hello_navigation")
set_target_properties( t6_hello_navigation PROPERTIES PREFIX "")
set_target_properties( t6_hello_navigation PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	RUNTIME_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	LIBRARY_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial )
add_definitions(
	-D_USRDLL
	-DELPP_NO_DEFAULT_LOG_FILE
)

