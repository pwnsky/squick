set(ProjectName "squick_core")

file(GLOB soruces
	*.cc)
file(GLOB headers
	*.h)

add_library(${ProjectName} SHARED ${soruces} ${headers})
if (APPLE)
	
elseif(UNIX)
	target_link_libraries(${ProjectName} -ldl)
else()
	add_definitions( -DWIN_DLL_EXPORT=__declspec\(dllexport\) ) # memcout 导出
	#add_definitions( -DELPP_EXPORT_SYMBOLS ) # easylogging++ 使用
endif()

SquickOut(${ProjectName} "squick" ${SQUICK_RUNTIME_LIB_PATH} "squick_core")
add_definitions(-DSQUICK_NONCLIENT_BUILD)
