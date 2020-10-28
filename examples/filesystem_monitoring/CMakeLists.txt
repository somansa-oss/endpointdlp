cmake_minimum_required(VERSION 3.10)

project(PIKern)

set(CMAKE_VERBOSE_MAKEFILE "YES")

ADD_DEFINITIONS ( -DLINUX )

LINK_LIBRARIES ( -pthread -ldl -lgio-2.0  -lgobject-2.0 -lglib-2.0 -ludev -Wall )

add_executable(get_mount_info         get_mount_info.cpp
)

add_executable(fs_monitor            fs_monitor.c
)

add_executable(mount_monitor         mount_monitor.c
)

add_executable(get_device_info	get_device_info.cpp )

add_executable(inotify_watch	inotify_watch.c )

target_include_directories(mount_monitor PUBLIC
	.  
	/usr/include/libmount
	/usr/include/blkid
	/usr/include/glib-2.0
	/usr/lib/x86_64-linux-gnu/glib-2.0/include
)
