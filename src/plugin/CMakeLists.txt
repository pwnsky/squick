add_subdirectory(core)
add_subdirectory(node)

if( MODE STREQUAL "dev" )
    add_subdirectory(tutorial)
endif()
