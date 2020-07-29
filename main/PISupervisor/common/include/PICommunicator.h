#ifndef	_PICOMMUNICATOR_H
#define _PICOMMUNICATOR_H

#include"LogWriter.h"
#include"PIPacket.h"
#include"PIObject.h"
#include <list>

////////////////////////////////////////////////////////////////////////////////
//CPICommSupport 
class CPICommSupport : public CPIPacketSupport
{
public:
	CPICommSupport(void);
	virtual ~CPICommSupport(void);

public:
	bool savePacket;
	std::string savePacketPath;
public:
	void clear(void);
	bool save(const char* header, int fd, const void* buf, const size_t count);
};

////////////////////////////////////////////////////////////////////////////////
//CPICommunicator 
class CPICommunicator : public CPIObject {
	public:
		CPICommunicator();
		virtual ~CPICommunicator();

	public:
		typedef enum {
			TCPSTATE_NOTCONNECTED	= 0,
			TCPSTATE_CONNECTED		= 1,
			TCPSTATE_SENDING		= 2,
			TCPSTATE_RECVING		= 3,
			TCPSTATE_ONRECV 		= 4
		} COMM_TCPSTATE_T;

		typedef enum {
			STATE_INACTIVE	= 0,
			STATE_ACTIVE	= 1
		} COMM_STATE_T;

		typedef enum {
			RESULT_OK				= 0,
			RESULT_ERROR			= 1,
			RESULT_NOPACKET			= 2,
			RESULT_INVALID_STATE	= 3
		} COMM_RESULT_T;

	public:
		//	Unix Attribute
		long long unixHostID;
		std::string sysName;
		std::string nodeName;
		std::string release;
		std::string version;
		std::string machine;

		//	State
		COMM_STATE_T state;

		// TCP/IP Connection for Client
		struct in_addr ipAddr;
		std::string ipString;
		struct sockaddr_in sockAddr;
		int tcpFd, sockLen;
		unsigned long tcpPort;
		COMM_TCPSTATE_T tcpState;
		unsigned char tcpRecvBuffer[ MAX_LENGTH_TCPBUFFER ];
		unsigned char tcpSendBuffer[ MAX_LENGTH_TCPBUFFER ];
		long remainedRecvBufSize;

		// PacketQueue
		std::list<CPIPacket> listSend;
		std::list<CPIPacket> listRecv;
		pthread_mutex_t mutexSend;
		pthread_mutex_t mutexRecv;

		CPICommSupport support;

	protected:
		virtual bool initialize(void);
		virtual bool finalize(void);
		virtual void clear(void);
	
	public:
		bool initializeTCPClient(void);
		bool terminateTCPClient(void);
		COMM_RESULT_T connectTCP(void);	
		COMM_RESULT_T disconnectTCP(void);
		COMM_RESULT_T sendTCP(void);
		COMM_RESULT_T recvTCP(void);
		COMM_TCPSTATE_T selectTCP(void);
};
#endif
