
#ifndef _PIWLAN_CHECK_H_
#define _PIWLAN_CHECK_H_

#include <stdio.h>
#include <vector>
#include <string>

using namespace std;

typedef bool (*fpWLanLibPolicyCheck)();
typedef bool (*fpWLanLibSetPolicySSID)(vector<string> vecPolicySSID );
typedef bool (*fpWLanLibGetBlockSSID)( string& strOutBlockSSID );
typedef bool (*fpWLanLibGetIfaceName)( string& strOutIfaceName );

class CPIWLanCheck
{
public:
    CPIWLanCheck();
    ~CPIWLanCheck();
    
public:
    bool SetWLanSSIDControl( std::vector<std::string> vecWLanPermitList );
    string GetBlockSSID(void);
    string GetIfaceName(void);
    
protected:
    bool LibUninit();
    bool LibInit(const char* pczLibName);
    bool LibWLanLibPolicyCheck();
    bool LibWLanLibSetPolicySSID(vector<string> vecPolicySSID );
    bool LibWLanLibGetBlockSSID();
    bool LibWLanLibGetIfaceName();

private:
    string m_strBlockSSID;
    string m_strIfaceName;
private:
    void* m_pHandle;
    fpWLanLibPolicyCheck   m_pWLanLibPolicyCheck;
    fpWLanLibSetPolicySSID m_pWLanLibSetPolicySSID;
    fpWLanLibGetBlockSSID  m_pWLanLibGetBlockSSID;
    fpWLanLibGetIfaceName  m_pWLanLibGetIfaceName;
};


extern CPIWLanCheck g_SSID;


#endif /* _LIB_CHECK_H_ */
