cmake_minimum_required(VERSION 2.8)

add_subdirectory(struct)
add_subdirectory(core)
add_subdirectory(plugin)

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
SquickOut(${ProjectName} "squick" ${SQUICK_BUILD_EXE_PATH} "squick")
