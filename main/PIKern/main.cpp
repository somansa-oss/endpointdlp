#ifdef LINUX
#include <string>
#include <sstream>
#else
#import <Foundation/Foundation.h>
#import <EndpointSecurity/EndpointSecurity.h>
#import <bsm/libbsm.h>
#import <signal.h>
#import <mach/mach_time.h>
#endif

#include <stdio.h>
#include <sys/mount.h>

#ifdef LINUX
    #include "../../PISupervisor/apple/include/KernelProtocol.h"
#else
    #include "../../PISupervisor/PISupervisor/apple/include/KernelProtocol.h"
#endif

#include "DataType.h"
#include "KernelCommand.h"
#include "kernel_control.h"
#include "PISecSmartDrv.h"
#include "PIActionSupervisorIPC.h"

#include "PIESF.h"

boolean_t supervisorWorking()
{
    char buffer[1024] = { 0, };
    std::string command, temp, result;
    command = "launchctl list | grep pisupervisor";
    
    FILE* pipe = popen(command.c_str(), "r");
    
    if (NULL != pipe)
    {
        while(!feof(pipe))
        {
            if(fgets(buffer, 1024, pipe) != NULL)
            {
                result += buffer;
            }
        }
        pclose(pipe);
    }
    
    std::istringstream is(temp);
    std::string token;
    std::getline(is, token);
    
    if( std::string::npos != token.find("pisupervisor") )
    {
        return true;
    }
    return false;
}

int main(int argc, const char * argv[])
{
    PISecSmartDrv_start(NULL, NULL);
    
    CPIActionSupervisorIPC::getInstance().run("");
    
    CPIESF::getInstance().run();
    
    while(true)
    {
        if (CPIESF::getInstance().isActive() == false)
            break;
        sleep(1);
    }
    
    PISecSmartDrv_stop(NULL, NULL);

    return 0;
}
