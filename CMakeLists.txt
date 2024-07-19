cmake_minimum_required(VERSION 3.6)

project("squick")

set_property(GLOBAL PROPERTY USE_FOLDERS On)

set(SolutionDir ${PROJECT_SOURCE_DIR})
message(STATUS "SolutionDir=" ${SolutionDir})

# Link basic liberay
macro(LinkStruct proName)
	add_dependencies(${proName} struct)
	target_link_libraries(${proName} struct)
endmacro(LinkStruct)

macro(LinkCore proName)
	add_dependencies(${proName} core)
	target_link_libraries(${proName} core)
endmacro(LinkCore)

# Squick project out, Included execute project or plugin project
macro(SquickOut proName outFolder outPath outName)
set_target_properties( ${proName} PROPERTIES OUTPUT_NAME ${outName} )
set_target_properties( ${proName} PROPERTIES PREFIX "")

string(COMPARE EQUAL "${outFolder}" "" result)
if(result)
  message("...")
else()
set_target_properties( ${proName} PROPERTIES FOLDER ${outFolder} )
endif()

set_target_properties( ${proName} PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY ${outPath}
	RUNTIME_OUTPUT_DIRECTORY ${outPath}
	ARCHIVE_OUTPUT_DIRECTORY ${outPath}
	ARCHIVE_OUTPUT_DIRECTORY ${outPath}
	LIBRARY_OUTPUT_DIRECTORY ${outPath}
	LIBRARY_OUTPUT_DIRECTORY ${outPath}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${outPath}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${outPath}
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG  ${outPath}
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE  ${outPath}
	LIBRARY_OUTPUT_DIRECTORY_DEBUG ${outPath}
	LIBRARY_OUTPUT_DIRECTORY_RELEASE ${outPath}
)
endmacro(SquickOut)


# The path for builded executed files
set(SQUICK_BUILD_EXE_PATH ${SolutionDir}/bin)
# The path for builded executed files dependency
set(SQUICK_RUNTIME_LIB_PATH ${SolutionDir}/bin)
# The path for builded plugin files
set(SQUICK_BUILD_PLUGIN_CORE_PATH ${SolutionDir}/bin/plugin/core)
# The path for builded node logic plugin files
set(SQUICK_BUILD_PLUGIN_SERVER_PATH ${SolutionDir}/bin/plugin/node)
# The path for builded extern plugin files
set(SQUICK_BUILD_PLUGIN_EXTEND_PATH ${SolutionDir}/bin/plugin/extend)
# The path for builded test plugin files
set(SQUICK_BUILD_PLUGIN_SQKCLI_PATH ${SolutionDir}/bin/plugin/sqkcli)
# The path for builded tutorial plugin files
set(SQUICK_BUILD_PLUGIN_TUTORIAL_PATH ${SolutionDir}/bin/plugin/tutorial)

include_directories(
	${SolutionDir}
	${SolutionDir}/src/
	${SolutionDir}/third_party/
	${SolutionDir}/third_party/build/include
	${SolutionDir}/third_party/zlib
	${SolutionDir}/third_party/lua/src
	)

set(ThirdPartyLibPath
	${SolutionDir}/third_party/build/lib
	${SolutionDir}/third_party/build/bin
)

set(SquickLibPath
	${SolutionDir}/bin/lib
)

if (APPLE)
	include_directories(
		${SolutionDir}/third_party/
		/opt/homebrew/include/
	)
	set(ThirdPartyLibPath
		${ThirdPartyLibPath}
		/opt/homebrew/lib/
	)
elseif(UNIX)
if( BUILD_VERSION STREQUAL "Debug" )
	SET ( CMAKE_CXX_FLAGS "-g" )
endif()
	include_directories(
		${SolutionDir}/third_party/
	)
else()
	SET ( CMAKE_CXX_FLAGS "-D_X64 -D_WINDOWS /EHsc /permissive-" )
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
	include_directories(
		${SolutionDir}/third_party/
	)
	add_definitions(
		-DPROTOBUF_USE_DLLS
	)
	add_definitions(
		-DWIN_DLL_EXPORT=__declspec\(dllimport\)
	)
	add_definitions(
	-DWIN32_LEAN_AND_MEAN -DELPP_WINSOCK2 -DELPP_AS_DLL -DELPP_STL_LOGGING
	)
endif()

set(CMAKE_CXX_STANDARD 23)
if( MODE STREQUAL "dev" )
	add_definitions(-DSQUICK_DEV )
endif()

link_directories(${ThirdPartyLibPath} ${SquickLibPath})
add_definitions(-D_X64 -D_CONSOLE -DSQUICK_DYNAMIC_PLUGIN -DSQUICK_ENABLE_SSL)

add_definitions(-DSQUICK_LOG_OPEN_STDOUT) # Log stdout print

add_subdirectory(src/struct)
add_subdirectory(src/squick)
add_subdirectory(src/node)
add_subdirectory(src/tools/sqkcli)
add_subdirectory(src/tutorial)


if( EXTRA STREQUAL "EXTRA" )
	
endif()

message("Start to build squick")

macro(source_group_by_dir source_files)
    if(MSVC)
        set(honcur_dir ${CMAKE_CURRENT_SOURCE_DIR})
        foreach(honfile ${${source_files}})            
            string(REGEX REPLACE "\(.*\)/.*/.*" \\1 honproject_directory ${honfile})
            INCLUDE_DIRECTORIES(SYSTEM ${honproject_directory})
            #MESSAGE(${honproject_directory})
            
            string(REGEX REPLACE ${honcur_dir}/\(.*\) \\1 honfpath ${honfile})
            string(REGEX REPLACE "\(.*\)/.*" \\1 hongroup_name ${honfpath})
            string(COMPARE EQUAL ${honfpath} ${hongroup_name} honnogroup)
            string(REPLACE "/" "\\" hongroup_name ${hongroup_name})
            
            if(honnogroup)
                set(hongroup_name "\\")
            endif(honnogroup)
            source_group(${hongroup_name} FILES ${honfile})
        endforeach(honfile)
    endif(MSVC)
endmacro(source_group_by_dir)

set(ProjectName "startup")
file(GLOB soruces src/*.cc)
file(GLOB headers src/*.h)
file(GLOB_RECURSE proto ./src/proto/*.proto)
file(GLOB_RECURSE lua ./src/lua/*.lua)
file(GLOB_RECURSE plugin ./config//plugin/*.xml)
file(GLOB_RECURSE config ./config/node/*.json)

source_group_by_dir(proto)
source_group_by_dir(lua)
source_group_by_dir(plugin)
source_group_by_dir(config)

add_executable(${ProjectName} ${soruces} ${headers} ${proto} ${lua} ${plugin} ${config})

if (APPLE)

elseif(UNIX)
target_link_libraries( ${ProjectName} -pthread libprotobuf.so)
else()
	target_link_libraries(${ProjectName} libprotobufd.dll)
	set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${ProjectName} )
	set_property(TARGET ${ProjectName} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${SolutionDir}/bin")
endif()


LinkStruct(${ProjectName})
LinkCore(${ProjectName})
SquickOut(${ProjectName} "" ${SQUICK_BUILD_EXE_PATH} "squick")
