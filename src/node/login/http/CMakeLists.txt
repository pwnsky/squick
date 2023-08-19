set(ProjectName "login_http")
file(GLOB soruces *.cc
	${SolutionDir}/third_party/common/sha256.cpp
)
file(GLOB headers *.h
	${SolutionDir}/third_party/common/sha256.h
)

add_library(${ProjectName} SHARED ${soruces} ${headers})
LinkSquick(${ProjectName})
SquickOut(${ProjectName} "node/login" ${SQUICK_BUILD_PLUGIN_SERVER_PATH}/login "http")