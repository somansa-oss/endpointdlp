cmake_minimum_required(VERSION 3.10)

# set the project name
project(PISupervisor)

set(CMAKE_VERBOSE_MAKEFILE "YES")

#target_include_directories(Tutorial PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#                           )

ADD_DEFINITIONS ( -DLINUX )

LINK_LIBRARIES ( -pthread -ldl -llsf-media )

# add the executable
# not supported for Linux:
#   src/PIFullDiskAccess.cpp
#   src/PIDRDevice.cpp
#   src/PIWallMan.cpp
#   src/PIWallController.cpp
#   linux/src/PWDiskStub.cpp
#   linux/src/ProtectCheck.cpp          
#   linux/src/PISecFilterDeviceStub.cpp     
#   linux/src/PIWLanCheck.cpp               linux/src/PWKctl.cpp
add_executable(PISupervisor src/PISuperivorMain.cpp  
        src/PIAction.cpp                    src/PIActionInitializeData.cpp          src/PIActionRegister.cpp      
        src/PIClientStub.cpp                src/PIDeviceMan.cpp                     src/PIEventHandler.cpp
        src/PISuperivorMain.cpp
        src/PIActionApplyPolicy.cpp         src/PIActionOnChangeOnOfflineMode.cpp   src/PIActionReloadPolicy.cpp  
        src/PIConfigMan.cpp                 src/PIDeviceManTest.cpp                 src/PITickCount.cpp
        src/PIActionCheckAgent.cpp          src/PIActionOpenClientIPC.cpp           src/PIActionUnregister.cpp    
        src/PIDiskSize.cpp                  src/PIPolicyMan.cpp                     
        src/PIActionControlDevice.cpp       src/PIActionProcessDeviceLog.cpp        src/PIAgentStub.cpp           
        src/PIDeviceController.cpp          src/PIDocument.cpp                      src/PISQLiteStub.cpp
        src/PIActionHeartbeatCheck.cpp      src/PIActionProcessTTL.cpp              src/PIApplicationControl.cpp  
        src/PIDeviceLog.cpp                 src/PIEvent.cpp                         src/PIScheduler.cpp
        common/src/DLPApplication.cpp       common/src/LogWriter.cpp                common/src/Markup.cpp
        common/src/PICommunicator.cpp  
        common/src/PIPacket.cpp             common/src/PIUtility.cpp                common/src/ShareFolderCtrl.cpp
        common/src/DLPPolicy.cpp            common/src/PCSecurity.cpp               common/src/PIObject.cpp
        common/src/PITickCountParent.cpp    common/src/PIXmlParser.cpp 
        sqlite/src/sqlite3.c 
        linux/src/SysControl.cpp            linux/src/KernControl.cpp               linux/src/PIUSBDrive.cpp          
        linux/src/UsbFinder.cpp
        linux/src/KextManager.cpp           linux/src/PIKernelStub.cpp              linux/src/PISecSmartDrvStub.cpp 
        apple/src/ESFControl.cpp            apple/src/ESFManager.cpp                apple/src/PIESFStub.cpp
        )

        #INCLUDE_DIRECTORIES ( include common/include sqlite/include linux/include )
target_include_directories(PISupervisor PUBLIC
    . include common/include sqlite/include linux/include apple/include )
