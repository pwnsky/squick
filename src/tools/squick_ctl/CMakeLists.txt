set(ProjectName "squick_ctl")

include_directories(
	minizip
	tinyxml2
	zlib)

set(SolutionDir ${PROJECT_SOURCE_DIR}/../../..)
set(CMAKE_CXX_STANDARD 17)

include_directories(
	${SolutionDir}
	${SolutionDir}/src/
	${SolutionDir}/third_party/
	${SolutionDir}/third_party/build/include
	${SolutionDir}/third_party/zlib
	)

file(GLOB squick_ctl_cc *.cc)
file(GLOB squick_ctl_h *.h)


file(GLOB squick_ctl_excel_minizip_c ${SolutionDir}/third_party/minizip/ioapi.c ${SolutionDir}/third_party/minizip/ioapi_mem.c ${SolutionDir}/third_party/minizip/unzip.c)
file(GLOB squick_ctl_excel_minizip_h ${SolutionDir}/third_party//minizip/*.h)
source_group("minizip" FILES ${squick_ctl_excel_minizip_c})
source_group("minizip" FILES ${squick_ctl_excel_minizip_h})


file(GLOB squick_ctl_excel_zlib_c ${SolutionDir}/third_party/zlib/*.c)
file(GLOB squick_ctl_excel_zlib_h ${SolutionDir}/third_party/zlib/*.h)
source_group("zlib" FILES ${squick_ctl_excel_zlib_c})
source_group("zlib" FILES ${squick_ctl_excel_zlib_h})

file(GLOB squick_ctl_excel_cc ./excel/*.cc)
file(GLOB squick_ctl_excel_h ./excel/*.h)
source_group("excel" FILES ${squick_ctl_excel_cc})
source_group("excel" FILES ${squick_ctl_excel_h})

add_executable(${ProjectName}
	${squick_ctl_cc}
	${squick_ctl_h}

	${squick_ctl_excel_cc}
	${squick_ctl_excel_h}
	${squick_ctl_excel_minizip_c} 
	${squick_ctl_excel_minizip_h}
	${squick_ctl_excel_zlib_c} 
	${squick_ctl_excel_zlib_h}

	)


# 工程输出
macro(SquickOut proName outFolder outPath outName)
set_target_properties( ${proName} PROPERTIES OUTPUT_NAME ${outName} )
set_target_properties( ${proName} PROPERTIES PREFIX "")
set_target_properties( ${proName} PROPERTIES 
	FOLDER ${outFolder}
	RUNTIME_OUTPUT_DIRECTORY ${outPath}
	RUNTIME_OUTPUT_DIRECTORY ${outPath}
	ARCHIVE_OUTPUT_DIRECTORY ${outPath}
	ARCHIVE_OUTPUT_DIRECTORY ${outPath}
	LIBRARY_OUTPUT_DIRECTORY ${outPath}
	LIBRARY_OUTPUT_DIRECTORY ${outPath}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${outPath}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${outPath}
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG  ${outPath}
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE  ${outPath}
	LIBRARY_OUTPUT_DIRECTORY_DEBUG ${outPath}
	LIBRARY_OUTPUT_DIRECTORY_RELEASE ${outPath}
)
endmacro(SquickOut)

SquickOut(${ProjectName} ${ProjectName} ${SolutionDir}/tools/bin ${ProjectName})
add_definitions(
	-D_CRT_SECURE_NO_WARNINGS
	-D_CRT_NONSTDC_NO_DEPRECATE
)
