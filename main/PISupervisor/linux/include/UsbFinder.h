#ifndef __USB_FINDER_H__
#define __USB_FINDER_H__

#include <iostream>
#include <vector>

#define MAX_NAME 128
#define MAX_PATH 260
typedef unsigned char BYTE;


#define MAX_USB   32
#define MAX_IFACE 12
    
    
#define DEVICE_TYPE_UNKNOWN   0x0000
#define DEVICE_TYPE_USBSTOR   0x0001
#define DEVICE_TYPE_USBMOBILE 0x0002
#define DEVICE_TYPE_RNDIS     0x0004
    
    
class USBIface
{
public:
    char      m_czIfaceName[MAX_NAME];
    uint32_t  m_nIndex;
    uint32_t  m_nClass;
    uint32_t  m_nSubClass;
    uint32_t  m_nProtocol;
};

class USBInfo
{
public:
    USBInfo();
    ~USBInfo();
public:
    void USBInfoInit();
    
public:
    uint32_t  m_nDeviceType;
    uint32_t  m_nVendorID;
    uint32_t  m_nProductID;
    char      m_czSerial[MAX_NAME];
    char      m_czVendor[MAX_NAME];
    char      m_czProduct[MAX_NAME];
    char      m_czVolumePath[MAX_PATH];
    char      m_czMediaID[MAX_PATH];
    
    uint32_t  m_nIfaceCount;
    USBIface  m_Iface[MAX_IFACE];
    
};
    
    
class UsbFinder
{
public:
    UsbFinder();
    ~UsbFinder();
private:
     bool EnumerateUsbDevice();
public:
	bool Reload(std::vector<USBInfo>& list);
    
public:
#ifdef _FIXME_

    static char* CFURLRefToCString(CFURLRef pCFUrl);
    static char* CFTypeRefToCString(CFTypeRef pCFType);
    static char* CFStringRefToCString(CFStringRef pCFString);
    
    static bool UsbDevice_Parsing(char* pczBsdNamePos, io_service_t UsbDevice, UsbFinder* pUsbInfo);
    static void UsbDevice_Callback(void* pRefCon, io_iterator_t Iter);
    static void FetchUsbDeviceInfo( io_service_t usbDevice, UsbFinder* pUsb );
    static char* FetchUsbStringDescriptor( IOUSBDeviceInterface182** ppUsbDevice, BYTE bIndex);
#endif
    
protected:
    void SetUSBInfo(USBInfo& UsbInfo );
    void SetVolume(char* pczVolume);
    void SetVolumePath( char* pczVolumePath );
    
    void SetVendorID(int nVendorID);
    void SetProductID(int nProductID);
    
    void SetVendor(char* pczVendor);
    void SetProduct(char* pczProduct);
    void SetSerial(char* pczSerial);
    void SetInterface( int nNumber, char* pczIfaceName, int nClass, int nSubClass, int nProto );
    void SetInterface( int nPos, USBIface& UsbIface );
    void SetMediaID(USBInfo& UsbTemp);
    uint32_t SetDeviceType(USBInfo& UsbTemp);
    
    void SetCopyUSBInfo(USBInfo& UsbInfo );
    
    bool IsUsbDeviceUsbMobile( USBInfo& UsbTemp );
    bool IsUsbDeviceUsbStor( USBInfo& UsbTemp );
    
    
public:
    void UsbDeviceInit();

#ifdef _FIXME_    
    static void UsbDeviceEnumerate(void* pRefCon, io_iterator_t Iter);
    
    static bool FetchUsbDevice( io_service_t UsbDevice, UsbFinder* pUsbFinder, USBInfo& UsbTemp );
    static bool FetchUsbDeviceBsdName( io_service_t UsbDevice, USBInfo& UsbTemp );
    static bool UsbDeviceSetVolumePath( char* pczBsdNamePos, io_service_t UsbDevice, USBInfo& UsbTemp );
    
    static bool FetchUsbInterface( io_service_t UsbDevice, UsbFinder* pUsbFinder, USBInfo& UsbTemp );
#endif    
    uint32_t GetUsbDeviceCount(void);
    bool GetUsbDevice( int nIndex, char* pczOutBuf, int nMaxOutBuf );
    bool GetUsbDeviceInfo( int nPos, USBInfo* pOutUsbInfo );
    void GetUsbDeviceInfoListXml(std::string& sXmlOut);

public:
    uint32_t m_nCount;
    USBInfo  m_UsbInfo[MAX_USB];
};
#endif 
