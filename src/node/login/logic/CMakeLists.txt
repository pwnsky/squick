set(ProjectName "login_logic")
file(GLOB soruces *.cc
	${SolutionDir}/third_party/common/sha256.cpp
)
file(GLOB headers *.h
	${SolutionDir}/third_party/common/sha256.h
)

add_library(${ProjectName} SHARED ${soruces} ${headers})
LinkStruct(${ProjectName})
LinkCore(${ProjectName})
SquickOut(${ProjectName} "node/login" ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/login "logic")