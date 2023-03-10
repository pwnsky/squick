set(ProjectName "t1_hello_plugin")
file(GLOB Tutorial1_ROOT_Cpp
	*.cc)
file(GLOB Tutorial1_ROOT_Hpp 
	*.h)

add_library(t1_hello_plugin SHARED
	${Tutorial1_ROOT_Cpp}
	${Tutorial1_ROOT_Hpp})
	
set_target_properties( t1_hello_plugin PROPERTIES OUTPUT_NAME "hello_plugin")
set_target_properties( t1_hello_plugin PROPERTIES PREFIX "")
set_target_properties( t1_hello_plugin PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	RUNTIME_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial
	LIBRARY_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/others/tutorial )
add_definitions(
	-D_USRDLL
	-DELPP_NO_DEFAULT_LOG_FILE
)

