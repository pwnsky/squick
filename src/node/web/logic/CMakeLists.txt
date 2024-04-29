set(ProjectName "web_logic")
file(GLOB soruces *.cc
	${SolutionDir}/third_party/common/sha256.cpp
)
file(GLOB headers *.h
	${SolutionDir}/third_party/common/sha256.h
)

add_library(${ProjectName} SHARED ${soruces} ${headers})
LinkStruct(${ProjectName})
SquickOut(${ProjectName} "node/web" ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/web "logic")