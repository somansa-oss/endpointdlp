cmake_minimum_required(VERSION 3.10)

set(CMAKE_VERBOSE_MAKEFILE "YES")

ADD_DEFINITIONS ( -DLINUX )

add_library(pisupervisor_common_lib
		PIObject.cpp		PIPacket.cpp		PICommunicator.cpp
		LogWriter.cpp		PIUtility.cpp
)

target_include_directories(pisupervisor_common_lib PUBLIC
    . ../include ../../include ../../apple/include )
