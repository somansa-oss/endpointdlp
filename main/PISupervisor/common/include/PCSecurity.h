//
//  PCSecurity.hpp
//  PISecFileShare
//
//  Created by somansa on 2018. 10. 25..
//  Copyright © 2018년 somansa. All rights reserved.
//

#ifndef _PCSECURITY_H_
#define _PCSECURITY_H_

#include <stdio.h>
#include <pthread.h>
#include "ShareFolderCtrl.h"

#ifdef __cplusplus
extern "C"
{
#endif
    
    class CPCSecurity
    {
    public:
        CPCSecurity();
        ~CPCSecurity();
    public:
        bool Load(CShareFolder& cSFolder);
        long LoadPCSecurityInfo(CShareFolder& cSFolder);
        void ApplyShareFolder();
        
        bool PCSecurity_SFThread_Start();
        bool PCSecurity_SFThread_Stop();
        static void* PCSecuritySFolderThreadProc(void* pParam);
        
    public:
        bool getThreadExit() { return m_bThreadExit; }
        void setThreadExit(bool bThreadExit) { m_bThreadExit = bThreadExit; }
        
        pthread_t getSFThread() { return m_pSFThread; }
        void setSFThread(pthread_t pSFThread) { m_pSFThread = pSFThread; }
        
        CShareFolderCtrl& GetSFCtrl() { return m_SFCtrl; }
        CShareFolder& GetSFPolicy() { return m_SFCtrl.GetSFPolicy(); }
        
    private:
        bool m_bThreadExit;
        pthread_t m_pSFThread;
        pthread_mutex_t m_SFMutex;
        CShareFolderCtrl m_SFCtrl;
    };


#ifdef __cplusplus
};
#endif
    
    
#endif /* _PCSECURITY_H_ */
