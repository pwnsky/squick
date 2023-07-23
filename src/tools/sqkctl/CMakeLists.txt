set(ProjectName "sqkctl")

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

file(GLOB sqkctl_cc *.cc)
file(GLOB sqkctl_h *.h)


file(GLOB sqkctl_excel_minizip_c ${SolutionDir}/third_party/minizip/ioapi.c ${SolutionDir}/third_party/minizip/ioapi_mem.c ${SolutionDir}/third_party/minizip/unzip.c)
file(GLOB sqkctl_excel_minizip_h ${SolutionDir}/third_party/minizip/*.h)
source_group("minizip" FILES ${sqkctl_excel_minizip_c})
source_group("minizip" FILES ${sqkctl_excel_minizip_h})


file(GLOB sqkctl_excel_zlib_c ${SolutionDir}/third_party/zlib/*.c)
file(GLOB sqkctl_excel_zlib_h ${SolutionDir}/third_party/zlib/*.h)
source_group("zlib" FILES ${sqkctl_excel_zlib_c})
source_group("zlib" FILES ${sqkctl_excel_zlib_h})


file(GLOB sqkctl_third_party_common_hpp ${SolutionDir}/third_party/common/*.hpp)
source_group("common" FILES ${sqkctl_third_party_common_hpp})

file(GLOB sqkctl_excel_cc ./excel/*.cc)
file(GLOB sqkctl_excel_h ./excel/*.h)
source_group("excel" FILES ${sqkctl_excel_cc})
source_group("excel" FILES ${sqkctl_excel_h})


file(GLOB sqkctl_init_cc ./init/*.cc)
file(GLOB sqkctl_init_h ./init/*.h)
source_group("init" FILES ${sqkctl_init_cc})
source_group("init" FILES ${sqkctl_init_h})

file(GLOB sqkctl_diff_cc ./diff/*.cc)
file(GLOB sqkctl_diff_h ./diff/*.h)
source_group("diff" FILES ${sqkctl_diff_cc})
source_group("diff" FILES ${sqkctl_diff_h})

file(GLOB sqkctl_add_cc ./add/*.cc)
file(GLOB sqkctl_add_h ./add/*.h)
source_group("add" FILES ${sqkctl_add_cc})
source_group("add" FILES ${sqkctl_add_h})

file(GLOB sqkctl_patch_cc ./patch/*.cc)
file(GLOB sqkctl_patch_h ./patch/*.h)
source_group("patch" FILES ${sqkctl_patch_cc})
source_group("patch" FILES ${sqkctl_patch_h})


file(GLOB sqkctl_update_cc ./update/*.cc)
file(GLOB sqkctl_update_h ./update/*.h)
source_group("update" FILES ${sqkctl_update_cc})
source_group("update" FILES ${sqkctl_update_h})

file(GLOB sqkctl_pull_cc ./pull/*.cc)
file(GLOB sqkctl_pull_h ./pull/*.h)
source_group("pull" FILES ${sqkctl_pull_cc})
source_group("pull" FILES ${sqkctl_pull_h})


add_executable(${ProjectName}
	${sqkctl_cc}
	${sqkctl_h}

	${sqkctl_excel_cc}
	${sqkctl_excel_h}
	${sqkctl_excel_minizip_c} 
	${sqkctl_excel_minizip_h}
	${sqkctl_excel_zlib_c} 
	${sqkctl_excel_zlib_h}

	${sqkctl_init_h}
	${sqkctl_init_cc}

	${sqkctl_diff_h}
	${sqkctl_diff_cc}

	${sqkctl_add_h}
	${sqkctl_add_cc}

	${sqkctl_patch_h}
	${sqkctl_patch_cc}
	
	${sqkctl_third_party_common_hpp}
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
