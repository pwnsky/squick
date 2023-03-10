file(GLOB tnavigation_ROOT_Cpp *.cpp)
file(GLOB tnavigation_ROOT_Hpp *.hpp)
file(GLOB tnavigation_ROOT_H *.h)

set(ThirdPartyLibPath ../build/lib)

add_compile_options(-fPIC)
add_library(recastnavigation 
	${tnavigation_ROOT_Cpp} 
	${tnavigation_ROOT_H} 
	${tnavigation_ROOT_Hpp})

add_definitions(-DNF_NONCLIENT_BUILD)
set_target_properties( recastnavigation PROPERTIES OUTPUT_NAME "recastnavigation")

if( WIN32 )
    set_target_properties( recastnavigation PROPERTIES PREFIX "")
endif()

set_target_properties( recastnavigation PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY .
	RUNTIME_OUTPUT_DIRECTORY .
	LIBRARY_OUTPUT_DIRECTORY . )