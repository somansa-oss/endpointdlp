///
///
#ifndef _PI_SHAREFOLDER_H_
#define _PI_SHAREFOLDER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
#include <vector>
#include <string>
#include <algorithm>
*/
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_PATH 260

#ifdef __cplusplus
extern "C"
{
#endif
    
    typedef struct _SF_RECORD
    {
        char czSFRecord[MAX_PATH];
        char czSFFilePath[MAX_PATH];
        bool bEveryOne;
        struct stat SFStat;
    } SF_RECORD, *PSF_RECORD;
    
    
    class CShareFolder
    {
    public:
        CShareFolder() : m_bBlockAll(false), m_bBlockEveryOne(false), m_bBlockDefault(false), m_nCheckInterval(60) {}
        ~CShareFolder() {}
    public:
        bool getBlockAll() { return m_bBlockAll; }
        bool getBlockEveryOne() { return m_bBlockEveryOne; }
        bool getBlockDefault() { return m_bBlockDefault; }
        int  getCheckInterval() { return m_nCheckInterval; }
        
        void setBlockAll(bool bBlockAll) { m_bBlockAll = bBlockAll; }
        void setBlockEveryOne(bool bBlockEveryOne) { m_bBlockEveryOne = bBlockEveryOne; }
        void setBlockDefault(bool bBlockDefault) { m_bBlockDefault = bBlockDefault; }
        void setCheckInterval(int nCheckInterval) { m_nCheckInterval = nCheckInterval; }
        
        bool IsExistPolicy() { return (m_bBlockAll || m_bBlockEveryOne || m_bBlockDefault); }
        
    private:
        bool m_bBlockAll;
        bool m_bBlockEveryOne;
        bool m_bBlockDefault;
        int  m_nCheckInterval;
    };
    
        
    class CShareFolderCtrl
    {
    public:
        CShareFolderCtrl();
        ~CShareFolderCtrl();
    public:
        int SelectKextMenu();
        bool ShellExecute(std::string strCommand, std::string& strResult);
        bool ShellExecuteFetchSFRecordList(std::string strCommand, std::vector<SF_RECORD>& vecSFList);
        bool ShellExecuteFetchSFFilePath(std::string strCommand, std::string& strOutFilePath);
        
        std::string
        StringReplaceAll( const std::string& strMessage, const std::string&& strPattern, const std::string&& strReplace );
        
        void ShareFolderPrint();
        bool ShareFolderEnum();
        bool ShareFolderDelete(std::string strSFolder);
        
        bool ShareFolderDisableAll();
        bool ShareFolderDisableDefault();
        bool ShareFolderDisableEveryOne();
        
    public:
        CShareFolder& GetSFPolicy() { return m_SFPolicy; }
    protected:
        CShareFolder m_SFPolicy;
        std::vector<SF_RECORD> m_vecSFList;
        
    };
    
    
    
#ifdef __cplusplus
};
#endif

#endif /* _PI_SHAREFOLDER_H_ */
