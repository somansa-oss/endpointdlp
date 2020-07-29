//
//  ProtectCheck.h
//  PICM
//
//  Created by somansa on 2017. 7. 26..
//  Copyright © 2017년 somansa. All rights reserved.
//

#ifndef _PROTECT_CHECK_H_
#define _PROTECT_CHECK_H_

//#include <stdio.h>
//#include <string>
#include "KernelDataType.h"


#define MAX_COMMAND  1024


#define SPT_FILEPATH          "/usr/local/Privacy-i"
#define SPT_PROC_PISupervisor "PISupervisor"
#define SPT_PROC_PICocoa      "PICocoa"
#define SPT_PROC_PIFinderSync "PIFinderSync"
#define SPT_PROC_PIAgent      "java"
#define SPT_PROC_WebAgent     "java"
#define SPT_PROC_CMD_PIAgent  "ps -ax | grep java | awk '/PIAGENTA/ {print $1}'  "
#define SPT_PROC_CMD_WebAgent "ps -ax | grep java | awk '/ServeriWebAgent/ {print $1}' "

#define SPT_PROC_CMD_PIAgentChecker "ps -ax | grep java | awk '/PIAGENTSTARTER/ {print $1}' "
#define SPT_PROC_CMD_PIAutoScan     "ps -ax | grep java | awk '/PIAUTOSCANA/ {print $1}' "
#define SPT_PROC_CMD_PIClient       "ps -ax | grep java | awk '/PICLIENTA/ {print $1}' "
#define SPT_PROC_CMD_PIDelete       "ps -ax | grep java | awk '/PIDELETEA/ {print $1}' "
#define SPT_PROC_CMD_PIEncrypt      "ps -ax | grep java | awk '/PIENCRYPTA/ {print $1}' "
#define SPT_PROC_CMD_PIScanWorker   "ps -ax | grep java | awk '/PISCANWORKER/ {print $1}' "
#define SPT_PROC_CMD_PISelfPatch    "ps -ax | grep java | awk '/PISELFPATCHA/ {print $1}' "
#define SPT_PROC_CMD_PIUpdate       "ps -ax | grep java | awk '/PIUPDATEA/ {print $1}' "

// User Service
#define SPT_PLIST_PISupervisor "/Library/LaunchDaemons/com.somansa.privacyi.pisupervisor.plist"
#define SPT_PLIST_PIAgent      "/Library/LaunchDaemons/com.somansa.privacyi.piagent.plist"
#define SPT_PLIST_WebAgent     "/Library/LaunchDaemons/com.somansa.privacyi.webagent.plist"
#define SPT_PLIST_PICocoa      "/Library/LaunchAgents/com.somansa.privacyi.PICocoa.plist"
#define SPT_PLIST_PIFinderSync "/Applications/PICocoa.app/Contents/PlugIns/PIFinderSync.appex"


class ProtectCheck
{
public:
    ProtectCheck();
    ~ProtectCheck();
    
public:
    std::string ReadCommandOutput(std::string strCommand);
    ULONG GetProcessIDCommand( const char* pczCommand );
    ULONG GetProcessID( const char* pczProcName );
    bool GetProcessPath( ULONG nPID, char* pczOutPath, uint32_t nMaxOutPath );
    

    
};








#endif /* _PROCESS_CHECK_H_ */
