cmake_minimum_required(VERSION 3.10)

project(PIKern)

set(CMAKE_VERBOSE_MAKEFILE "YES")

ADD_DEFINITIONS ( -DLINUX )

LINK_LIBRARIES ( -pthread -ldl )

add_executable(PIKern         KauthEventFunc.cpp
		kauth_listener.cpp		kernel_control.cpp
		KextDeviceNotify.cpp	main.cpp				PIActionSupervisorIPC.cpp
		PIClientStub.cpp		PIDocument.cpp			PIESF.cpp
		PISecSmartDrv.c			SmartCmd.cpp			system_control.cpp
)


target_include_directories(PIKern PUBLIC
    . include ../PISupervisor/include ../PISupervisor/common/include ../PISupervisor/apple/include )

target_link_libraries(PIKern pisupervisor_common_lib pisupervisor_src_lib )
