set(ProjectName "core_config")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})
LinkCore(${ProjectName})
SquickOut(${ProjectName} "squick/plugin" ${SQUICK_BUILD_PLUGIN_CORE_PATH} "config")
