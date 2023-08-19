add_subdirectory(core)

set(ProjectName "tester")
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
SquickOut(${ProjectName} "tools/tester" ${SQUICK_BUILD_EXE_PATH} "tester")