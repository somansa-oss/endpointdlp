cmake_minimum_required(VERSION 3.10)

project(PIKern)

set(CMAKE_VERBOSE_MAKEFILE "YES")

ADD_DEFINITIONS ( -DLINUX )

LINK_LIBRARIES ( -pthread -ldl )

add_executable(get_mount_info         get_mount_info.cpp
)

add_executable(fs_monitor            fs_monitor.c
)

add_executable(mount_monitor         mount_monitor.c
)

target_include_directories(get_mount_info PUBLIC
    .  )
