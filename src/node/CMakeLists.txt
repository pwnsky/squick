cmake_minimum_required(VERSION 2.8)

add_subdirectory(db_proxy)
add_subdirectory(login)
add_subdirectory(lobby)
add_subdirectory(game)
add_subdirectory(master)
add_subdirectory(proxy)
add_subdirectory(world)
add_subdirectory(gameplay_manager)
add_subdirectory(gameplay)
add_subdirectory(robot)

set(ProjectName "squick_exe")
file(GLOB soruces *.cc)
file(GLOB headers *.h)

add_executable(${ProjectName} ${soruces} ${headers})

if (APPLE)

elseif(UNIX)
target_link_libraries( ${ProjectName} -pthread libprotobuf.so)
else()
	target_link_libraries(${ProjectName} libprotobufd.dll)
endif()

LinkSquick(${ProjectName})
SquickOut(${ProjectName} "server" ${SQUICK_BUILD_EXE_PATH} "squick")