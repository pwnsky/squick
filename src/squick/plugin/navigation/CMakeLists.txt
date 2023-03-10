set(ProjectName "core_navigation")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})

target_include_directories(${ProjectName} PUBLIC
	${SolutionDir}/third_party/navigation/)

target_link_libraries(${ProjectName} recastnavigation)
LinkSquick(${ProjectName})
SquickOut(${ProjectName} "squick/plugin" ${SQUICK_BUILD_PLUGIN_CORE_PATH} "navigation")