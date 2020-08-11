cmake_minimum_required(VERSION 3.10)

project(PIKern)

set(CMAKE_VERBOSE_MAKEFILE "YES")

ADD_DEFINITIONS ( -DLINUX )

LINK_LIBRARIES ( -pthread -ldl )

add_executable(get_mount_info         get_mount_info.cpp
)

target_include_directories(get_mount_info PUBLIC
    .  )

#target_link_libraries(PIKern pisupervisor_common_lib pisupervisor_src_lib )
