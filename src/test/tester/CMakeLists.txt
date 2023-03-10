set(ProjectName "plugin_core_test")
file(GLOB plugin_core_test_cc
	*.cc)
	
file(GLOB plugin_core_test_h 
	*.h)

add_library(plugin_core_test SHARED
	${plugin_core_test_cc}
	${plugin_core_test_h})
	
set_target_properties( plugin_core_test PROPERTIES OUTPUT_NAME "test")
set_target_properties( plugin_core_test PROPERTIES PREFIX "")
set_target_properties( plugin_core_test PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_TESTER_PATH}
	RUNTIME_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_TESTER_PATH}
	LIBRARY_OUTPUT_DIRECTORY ${SQUICK_BUILD_PLUGIN_TESTER_PATH} )