set(ProjectName "core_net")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})

add_definitions(
	-D_THREAD_SAFE
)

if (APPLE)
	
elseif(UNIX)
	target_link_libraries(${ProjectName} libevent.a)
	target_link_libraries(${ProjectName} libevent_core.a)
	target_link_libraries(${ProjectName} libevent_extra.a)
else()
	target_link_libraries(${ProjectName} event.lib)
	target_link_libraries(${ProjectName} event_core.lib)
	target_link_libraries(${ProjectName} event_extra.lib)
	target_link_libraries(${ProjectName} event_extra.lib)
	target_link_libraries(${ProjectName} libprotobufd.dll)
	target_link_libraries(${ProjectName} ws2_32)
endif()

LinkSquick(${ProjectName})
SquickOut(${ProjectName} "squick/plugin" ${SQUICK_BUILD_PLUGIN_CORE_PATH} "net")