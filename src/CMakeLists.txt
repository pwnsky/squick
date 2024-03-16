set_property(GLOBAL PROPERTY USE_FOLDERS On)

set(SolutionDir ${PROJECT_SOURCE_DIR}/..)
message(STATUS "SolutionDir=" ${SolutionDir})

# 链接Squick必要动态链接库
macro(LinkSquick proName)
	add_dependencies(${proName} core struct)
	target_link_libraries(${proName} core struct)
endmacro(LinkSquick)


# 工程输出
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


# 可执行文件生成路径
set(SQUICK_BUILD_EXE_PATH ${SolutionDir}/bin)
# 可执行文件运行依赖动态链接库存放路径
set(SQUICK_RUNTIME_LIB_PATH ${SolutionDir}/bin)
# 核心插件路径
set(SQUICK_BUILD_PLUGIN_CORE_PATH ${SolutionDir}/bin/plugin/core)
# 服务器插件路径
set(SQUICK_BUILD_PLUGIN_SERVER_PATH ${SolutionDir}/bin/plugin/node)
# 拓展插件路径
set(SQUICK_BUILD_PLUGIN_EXTEND_PATH ${SolutionDir}/bin/plugin/extend)
# 测试插件路径
set(SQUICK_BUILD_PLUGIN_SQKCLI_PATH ${SolutionDir}/bin/plugin/sqkcli)
# Tutorial plugins build path
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

add_subdirectory(struct)
add_subdirectory(squick)
add_subdirectory(node)
add_subdirectory(tools/sqkcli)
add_subdirectory(tutorial)

if( EXTRA STREQUAL "EXTRA" )
	
endif()

message("Start to build squick")


set(ProjectName "startup")
file(GLOB soruces *.cc)
file(GLOB headers *.h)

add_executable(${ProjectName} ${soruces} ${headers})

if (APPLE)

elseif(UNIX)
target_link_libraries( ${ProjectName} -pthread libprotobuf.so)
else()
	target_link_libraries(${ProjectName} libprotobufd.dll)
	set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${ProjectName} )
	set_property(TARGET ${ProjectName} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${SolutionDir}/bin")
endif()


LinkSquick(${ProjectName})
SquickOut(${ProjectName} "" ${SQUICK_BUILD_EXE_PATH} "squick")
