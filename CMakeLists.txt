cmake_minimum_required(VERSION 3.1.0)

if (WIN32)
   set(VCOS_PLATFORM win32)
else ()
   set(VCOS_PLATFORM pthreads)
   add_definitions(-Wall -Werror)
endif ()

link_directories( /opt/vc/lib )
include_directories( /opt/vc/include
                     /opt/vc/include/interface/vcos
                     /opt/vc/include/interface/vcos/${VCOS_PLATFORM} )

add_executable(fanctl fanctl.c)
target_link_libraries(fanctl mraa vcos vchiq_arm vchostif pthread)

configure_file(systemd/fanctl.service.cmake ${CMAKE_CURRENT_BINARY_DIR}/systemd/fanctl.service)
install(TARGETS fanctl RUNTIME DESTINATION bin)
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/systemd/fanctl.service
	 DESTINATION /etc/systemd/system
	 COMPONENT init )
