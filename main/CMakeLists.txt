cmake_minimum_required(VERSION 3.10)

project(PIKern)

set(CMAKE_VERBOSE_MAKEFILE "YES")

#ADD_DEFINITIONS ( -DLINUX -DWSL )

ADD_DEFINITIONS ( -DLINUX )

LINK_LIBRARIES ( -pthread -ldl )

add_subdirectory( PIKern )

add_subdirectory( PISupervisor/src  )

add_subdirectory( PISupervisor/common/src  )
