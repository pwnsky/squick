set(ProjectName "proxy_logic")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})
LinkStruct(${ProjectName})
LinkCore(${ProjectName})
SquickOut(${ProjectName} "node/proxy" ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/proxy "logic")