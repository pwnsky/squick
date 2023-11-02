set(ProjectName "struct")

file(GLOB soruces *.cc)
file(GLOB headers *.h)

# 采用静态编译塞入可执行文件里，由于可执行程序内没有直接调用里面的函数，不存在关系，所以就不能链接关系。
add_library(${ProjectName} SHARED ${soruces} ${headers})
add_definitions(
	-D_THREAD_SAFE
)

if (APPLE)
	target_link_libraries(${ProjectName} libprotobuf.dylib)
elseif(UNIX)
	target_link_libraries(${ProjectName} libprotobuf.so)
else()
	target_link_libraries(${ProjectName} libprotobufd.dll)
	add_definitions( -DWIN_DLL_EXPORT=__declspec\(dllexport\) )
endif()

SquickOut(${ProjectName} "" ${SQUICK_RUNTIME_LIB_PATH} ${ProjectName})
