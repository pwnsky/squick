set(ProjectName "sqkcli_core")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})
LinkSquick(${ProjectName})
SquickOut(${ProjectName} "tools/sqkcli/plugin" ${SQUICK_BUILD_PLUGIN_SQKCLI_PATH} "core")