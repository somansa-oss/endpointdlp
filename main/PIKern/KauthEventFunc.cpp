#include <stdio.h>

#ifdef LINUX
    #include <string>
    #include "../../PISupervisor/apple/include/KernelProtocol.h"
#else
    #include "../../PISupervisor/PISupervisor/apple/include/KernelProtocol.h"
#endif

#include "KauthEventFunc.h"


#include "KextDeviceNotify.h"
#include "kauth_listener.h"
//#include "event_proto.h"
#include "SmartCmd.h"

#include "PIPacket.h"
#include"PICommunicator.h"

#include <arpa/inet.h>


// DataRead
boolean_t
Kauth_Vnode_IsProtectRead(int nPID, char* pczProcName, int nAction, int nVType, vnode_t pVnode, char* pczVnodePath, LOG_PARAM* pLog )
{
    boolean_t bProtect = FALSE;

    //if(!pczProcName || !pVnode || !pczVnodePath || !pLog)
    if(!pczProcName || !pczVnodePath || !pLog)    
    {
        return FALSE;
    }
    
#ifndef LINUX
    bProtect = IsProtect_KextUnload( nPID, pczProcName, pczVnodePath );
    if(TRUE == bProtect)
    {
        printf( "[DLP][%s] Deny. IsProtect_KextUnload pid=%d proc=%s, Path=%s \n", __FUNCTION__, nPID, pczProcName, pczVnodePath );
        return TRUE;
    }
#endif

    if(IsControlDeviceType( pVnode, pczVnodePath ))
    {
        pLog->nAction = ACTION_READ;
        pLog->pczPath1 = pczVnodePath;
        //bProtect = CtrlCheck_Read( nPID, pczProcName, nVType, pczVnodePath, pLog );
        bProtect = IsPolicyExist_BlockRead( nPID, pczProcName, nVType, pczVnodePath, pLog );
        if(TRUE == bProtect)
        {
            // printf( "[DLP][%s] Deny. log=%d, pid=%d proc=%s, Path=%s \n", __FUNCTION__, (int)pLog->bLog, nPID, pczProcName, pczVnodePath );
            return TRUE;
        }
    }
    return FALSE;
}


// Write
boolean_t
Kauth_Vnode_IsProtectWrite( int nPID, char* pczProcName, int nAction, int nVType, vnode_t pVnode, char* pczVnodePath, LOG_PARAM* pLog )
{
    boolean_t bProtect = FALSE;
    
    //if(!pczProcName || !pVnode || !pczVnodePath || !pLog)
    if(!pczProcName || !pczVnodePath || !pLog)
    {
        return FALSE;
    }
    
#ifndef LINUX
    if(TRUE == IsProtect_FilePath( pczVnodePath ))
    {
        return TRUE;
    }
    
    bProtect = IsProtect_Service( pczVnodePath );
    if(TRUE == bProtect)
    {
        // printf( "[DLP][%s] Deny.  IsProtect_Service pid=%d, proc=%s, Vnode=%s \n", __FUNCTION__, nPID, pczProcName, pczVnodePath );
        return TRUE;
    }
#endif
    
    if(IsControlDeviceType( pVnode, pczVnodePath ))
    {
        pLog->nAction = ACTION_WRITE;
        pLog->pczPath1 = pczVnodePath;
        //bProtect = CtrlCheck_Write( nPID, pczProcName, nVType, pczVnodePath, pLog );
        bProtect = IsPolicyExist_BlockWrite(nPID, pczProcName, nVType, pczVnodePath, pLog );
        if(TRUE == bProtect)
        {
            // printf( "[DLP][%s] Deny. log=%d, pid=%d proc=%s, Vnode=%s \n", __FUNCTION__, (int)pLog->bLog, nPID, pczProcName, pczVnodePath );
            return TRUE;
        }
    }
    return FALSE;
}

boolean_t
Kauth_FileOp_FileClose( int nPID, char* pczProcName, vnode_t pVnode, char* pczFilePath, int nFlag, LOG_PARAM* pLog )
{
    boolean_t bDirty = FALSE;
    boolean_t bNoAccess = FALSE;
    //if(!pczProcName || !pVnode || !pczFilePath)
    if(!pczProcName || !pczFilePath)
    {
        return false;
    }
    
    bDirty = (nFlag & KAUTH_FILEOP_CLOSE_MODIFIED) ? TRUE : FALSE;
    if(!bDirty) return false;
    
    if(TRUE == g_DrvKext.PrintCtx.Policy.bPolicy)
    {
        // first setting
        if(0 == strlen(g_czCupsSpoolPath))
        {
            UserModeTo_GetPrintSpoolPath();
            memset(g_czCupsSpoolPath, 0, sizeof(g_czCupsSpoolPath));
            strncpy(g_czCupsSpoolPath, g_czCupsSpoolPathDump, strlen(g_czCupsSpoolPathDump));
            printf( "[DLP][%s] KAUTH_FILEOP_CLOSE first setting pczName=%s, g_czCupsSpoolPath=%s, g_czCupsTempPath=%s, bDirty=%d\n ",
                      __FUNCTION__, pczProcName, g_czCupsSpoolPath, g_czCupsTempPath, bDirty );
        }
        
#ifndef LINUX        
        // "/private/etc/cups/cups-files.conf" modify?
        if(IsCupsdConfigFile( pczFilePath ))
        {
            UserModeTo_GetPrintSpoolPath();
            
            printf( "[DLP][%s] KAUTH_FILEOP_CLOSE CupsdConfigFile pczName=%s, g_czCupsSpoolPathDump=%s, g_czCupsTempPath=%s, bDirty=%d\n ",
                      __FUNCTION__, pczProcName, g_czCupsSpoolPathDump, g_czCupsTempPath, bDirty);
        }
#endif
        
        // temp path write?
        if(0 == strncasecmp( pczProcName, PROC_CUPSD, strlen(PROC_CUPSD)))
        {
            size_t nlen = strlen(g_czCupsTempPath);
            if(nlen > 0 && 0 == strncasecmp( pczFilePath, g_czCupsTempPath, strlen(g_czCupsTempPath)) && strlen(g_czCupsSpoolPathDump)>0)
            {
                memset(g_czCupsSpoolPath, 0, sizeof(g_czCupsSpoolPath));
                strncpy(g_czCupsSpoolPath, g_czCupsSpoolPathDump, strlen(g_czCupsSpoolPathDump));
                
                printf("[DLP][%s] KAUTH_FILEOP_CLOSE changed spool path pczName=%s, g_czCupsSpoolPath=%s, g_czCupsSpoolPathDump=%s, g_czCupsTempPath=%s, bDirty=%d\n ", __FUNCTION__, pczProcName, g_czCupsSpoolPath, g_czCupsSpoolPathDump, g_czCupsTempPath, bDirty);
                
                memset(g_czCupsSpoolPathDump, 0, sizeof(g_czCupsSpoolPathDump));
            }
        }
    }
    
    pLog->nAction = ACTION_CLOSE;
    pLog->pVnode = pVnode;
    pLog->pczPath1 = (char*)pczFilePath;
    
    if(IsControlDeviceType( pVnode, pczFilePath ))
    {
        bNoAccess = CtrlCheck_Contents( nPID, pczProcName, pVnode, pczFilePath, pLog, POLICY_COPY );
        printf( "[DLP][%s] IsControlDeviceType=TRUE pczName=%s, pczPath=%s, LogParam.nPolicyType=%d \n ",
               __FUNCTION__, pczProcName, pczFilePath, pLog->nPolicyType);
    }
    
    return bNoAccess;
}


bool SendToPISupervisor(CPIPacket& packet, int& response) {

    bool returnValue = false;
    CPICommunicator::COMM_RESULT_T result;
    CPICommunicator* comm = new CPICommunicator();
    comm->ipString = "127.0.0.1";
    comm->tcpPort = nsPISupervisor::PISUPERVISOR_PORT;
    
    do {
        if( false == comm->initializeTCPClient() ) {
            ERROR_LOG( "Unable to create socket - %s",  strerror(errno));
            break;
        }

        result = comm->connectTCP();
        if(  CPICommunicator::RESULT_OK != result ) {
            ERROR_LOG( "Unable to connect server - code(%d) - (%d)%s", result, errno, strerror(errno));
            break;
        }

        DEBUG_LOG( "connect piagent - ok - tcpState(%d)", comm->tcpState );

        comm->listSend.push_back(packet);

        result = comm->sendTCP();
        if(  CPICommunicator::RESULT_OK != result ) {
            ERROR_LOG( "Unable to send - code(%d) - tcpState(%d) - (%d)%s", result, comm->tcpState, errno, strerror(errno));
            break;
        }

        result = comm->recvTCP();
        if(  CPICommunicator::RESULT_OK != result ) {
            ERROR_LOG( "Unable to recv - code(%d) - (%d)%s", result, errno, strerror(errno));
            break;
        }

        returnValue = true;
    }while(false);

    if( 0 < comm->listRecv.size()) {
        DEBUG_LOG( "recv - msg_count:%d", comm->listRecv.size());
        std::list<CPIPacket>::iterator itr = comm->listRecv.begin();
        for( ; itr != comm->listRecv.end(); ++itr) {
            DEBUG_LOG( "recv - command(%d) - param_count(%d)", itr->getCommand(), itr->getParamCount());
            response = itr->get_int(0);
            returnValue = true;
        }
    }

    if( CPICommunicator::TCPSTATE_NOTCONNECTED != comm->tcpState ) {
        comm->terminateTCPClient();
    }

    delete comm;
    comm = NULL;
    return returnValue;
}

int SendEventDataToPISupervisor( void* pData, size_t nLength)
{
    int result = KERN_FAILURE;
    int response = 0;
    
    if (pData == NULL || nLength == 0 )
        return KERN_INVALID_ARGUMENT;
    
    void *pResult = NULL;
    PCOMMAND_MESSAGE pCmdNew = (PCOMMAND_MESSAGE)pData;
    
    CPIPacket packet;
    packet.setCommand(pCmdNew->Command);
    packet.addBinary(pData, pCmdNew->Size);
    
    if (true == SendToPISupervisor(packet, response))
    {
        PSCANNER_NOTIFICATION pNotify = NULL;
        pNotify = (PSCANNER_NOTIFICATION)pCmdNew->Data;
        if (pNotify != NULL)
        {
            pNotify->nResult = response;
            result = 0;
        }
    }
    
    return result;
}

