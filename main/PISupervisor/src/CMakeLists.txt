cmake_minimum_required(VERSION 3.10)

set(CMAKE_VERBOSE_MAKEFILE "YES")

ADD_DEFINITIONS ( -DLINUX )

add_library(pisupervisor_src_lib
		PIAction.cpp		
)

target_include_directories(pisupervisor_src_lib PUBLIC
    . ../include  )
