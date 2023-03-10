set(ProjectName "t1_hello_plugin")
file(GLOB Tutorial2_ROOT_CC
	*.cc)
file(GLOB Tutorial2_ROOT_H
	*.h)

add_library(t2_hello_property SHARED
	${Tutorial2_ROOT_CC}
	${Tutorial2_ROOT_H})
	
set_target_properties( t2_hello_property PROPERTIES OUTPUT_NAME "hello_property")
set_target_properties( t2_hello_property PROPERTIES PREFIX "")
set_target_properties( t2_hello_property PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	RUNTIME_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	LIBRARY_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial )
add_definitions(
	-D_USRDLL
	-DELPP_NO_DEFAULT_LOG_FILE
)

