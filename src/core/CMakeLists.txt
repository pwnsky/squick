set(ProjectName "core")

file(GLOB soruces
	*.cc
    ${SolutionDir}/third_party/easyloggingpp/*.cc
    )
file(GLOB headers
	*.h
    ${SolutionDir}/third_party/easyloggingpp/*.h
    )

add_library(${ProjectName} SHARED ${soruces} ${headers})

add_definitions( -DELPP_EXPORT_SYMBOLS -DAUTO_INITIALIZE_EASYLOGGINGPP) # easylogging++ 使用

if (APPLE)
	
elseif(UNIX)
	target_link_libraries(${ProjectName} -ldl)
else()
	add_definitions( -DWIN_DLL_EXPORT=__declspec\(dllexport\) ) # memcout 导出
endif()

SquickOut(${ProjectName} "squick" ${SQUICK_RUNTIME_LIB_PATH} ${ProjectName})
add_definitions(-DSQUICK_NONCLIENT_BUILD)
