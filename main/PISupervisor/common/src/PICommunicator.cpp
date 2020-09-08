#ifndef	_PICOMMUNICATOR_CPP
#define _PICOMMUNICATOR_CPP

#ifdef LINUX
#include <string>
#endif

#include"PICommunicator.h"

////////////////////////////////////////////////////////////////////////////////
//CPICommSupport
CPICommSupport::CPICommSupport(void) {
	clear();
}

CPICommSupport::~CPICommSupport(void) {
}

void CPICommSupport::clear(void) {
	savePacket = false;
	savePacketPath = "";
}

bool CPICommSupport::save(const char* header, int fd, const void* buf, const size_t count) {

	if( false == savePacket ) {
		return true;
	}

	if( savePacketPath.empty() ) {
		return false;
	}

	if( (NULL == buf) || (0 == count) ) {
		return false;
	}

	static int seq = 0;
	if( 999 < ++seq ) {
		seq = 0;
	}

	// ----------
	time_t t1;
	struct tm localTime;
	time( &t1 );
	localTime = *localtime_r( &t1, &localTime );
	char pathDateTime[1024] = {0x00,};
	sprintf( pathDateTime, "%s/packet-%04d%02d%02d%02d%02d%02d-%03d-%s-%02d.dat", 
			savePacketPath.c_str(), 
			1900+localTime.tm_year, 1+localTime.tm_mon, localTime.tm_mday, 
			localTime.tm_hour, localTime.tm_min, localTime.tm_sec,
			seq, header, fd);
	// ----------

	int fileFd = -1;
	if((fileFd = open( pathDateTime, O_CREAT|O_RDWR|O_SYNC, S_IRUSR|S_IWUSR ))<0 ) {
		return false;
	}

	bool result = true;
	if( write( fileFd, buf, count) < 0 ) {
		result = false;
	}
	
	close(fileFd);
	fileFd = -1;

	return result;
}

////////////////////////////////////////////////////////////////////////////////
// CPICommunicator
CPICommunicator::CPICommunicator() { 
	clear();
	initialize(); 
}

CPICommunicator::~CPICommunicator() {
	finalize(); 
}

bool CPICommunicator::initialize(void) {
	clear();
	pthread_mutex_init( &mutexSend, 0 ); 
	pthread_mutex_init( &mutexRecv, 0 ); 

	return CPIObject::initialize();
}

bool CPICommunicator::finalize(void) {

	disconnectTCP();

	pthread_mutex_trylock( &mutexSend );
	listSend.clear();
	pthread_mutex_unlock( &mutexSend );
	pthread_mutex_destroy( &mutexSend );
	
	pthread_mutex_trylock( &mutexRecv );
	listRecv.clear();
	pthread_mutex_unlock( &mutexRecv );
	pthread_mutex_destroy( &mutexRecv );
	
	return CPIObject::finalize();
}

void CPICommunicator::clear(void) {
	CPIObject::clear();

	remainedRecvBufSize = 0; 
	state = STATE_INACTIVE;
	tcpState = TCPSTATE_NOTCONNECTED;
	tcpFd = -1;

	support.clear();
}

CPICommunicator::COMM_RESULT_T CPICommunicator::sendTCP(void) {
	//DEBUG_LOG1( "begin");
	
	COMM_RESULT_T result = RESULT_OK;

	size_t listSendCount = listSend.size();
	if( 0 ==  listSendCount ) {
		result = RESULT_NOPACKET;
		//DEBUG_LOG( "fd(%d) - stop - no_packet", tcpFd);
		return result;
	} 

	pthread_mutex_lock( &mutexSend );
	if( 0 < listSend.size()) {
		tcpState= TCPSTATE_SENDING;

		bool isContinue = true;
		std::list<CPIPacket>::iterator itr = listSend.begin();
		while( (itr != listSend.end()) &&  (true == isContinue) ) {
			const void* buf = itr->serialize();
			const size_t count = itr->size(); 
			//DEBUG_LOG( "fd(%d) - sending size(%d)", tcpFd,  count);
			support.save( "sent", tcpFd, buf, count);
			
			long sent = write( tcpFd, buf, count);
			//int errno_save = errno;
			//DEBUG_LOG( "fd(%d) - sent(%d)", tcpFd, count);
			//if( 0 > sent ) {
				//DEBUG_LOG( "write - errno_save(%d:%s)", errno_save, strerror(errno_save));
			//}
			if( 0 < sent ) {
				itr = listSend.erase(itr);
			} else {
				isContinue = false;
				result = RESULT_ERROR;
				tcpState= TCPSTATE_NOTCONNECTED;
			}
		}

		if( TCPSTATE_NOTCONNECTED != tcpState) {
			tcpState= TCPSTATE_CONNECTED;
		}
	}
	pthread_mutex_unlock( &mutexSend );

	//DEBUG_LOG( "fd(%d) - result(%d) - tcp_state(%d)", tcpFd, result, tcpState);
	//DEBUG_LOG1( "end");
	return result;
}

CPICommunicator::COMM_RESULT_T CPICommunicator::recvTCP(void) {
	//DEBUG_LOG1( "begin");
	
	const unsigned long MIN_LENGTH_PIPROTOCOL = sizeof(int);

	long readSize = 0;
	long readSum = 0;
	long parsedSize = 0;
	COMM_RESULT_T result = RESULT_OK;

	while(true) { 
		unsigned char * buf = tcpRecvBuffer + remainedRecvBufSize + readSum;
		size_t count = sizeof(tcpRecvBuffer) - remainedRecvBufSize - readSum;
		readSize = read(tcpFd, buf, count);
		//int errno_save = errno;
		//DEBUG_LOG( "fd(%d) - read_buf(%d) - remained(%d), read_size(%d) - read_sum(%d)", tcpFd, count, remainedRecvBufSize, readSize, readSum);
		if( 0 > readSize ) {
			//DEBUG_LOG( "read - errno(%d:%s)", errno_save, strerror(errno_save));
			ERROR_LOG("fd(%d) - read_size(%d)", tcpFd, readSize);
			break;
		}
		
		if(0 == readSize) {
			//DEBUG_LOG( "fd(%d) - diconnected", tcpFd);
			state = STATE_INACTIVE;
			tcpState = TCPSTATE_NOTCONNECTED;
			break;
		}

		support.save( "recv_raw", tcpFd, buf , readSize);

		readSum += readSize;
		while( true ) {
			if( (readSum - parsedSize) < MIN_LENGTH_PIPROTOCOL ) {
				break; // need to receive more
			}

			int packetTotalBytes = 0; 
			memcpy( (void*)&packetTotalBytes, (void*)(buf + parsedSize), sizeof(int));

			if( true == support.bigEndian ) {
				packetTotalBytes = ntohl(packetTotalBytes);
			}

			//DEBUG_LOG( "fd(%d) - total_size(%d, 0x%02X%02X%02X%02X)", tcpFd, packetTotalBytes, buf[0], buf[1], buf[2], buf[3]);

			if(MIN_LENGTH_PIPROTOCOL >= packetTotalBytes ) {
				ERROR_LOG( "fd(%d) - invalid_total_size(%d, 0x%02X%02X%02X%02X)", 
						tcpFd, packetTotalBytes, buf[0], buf[1], buf[2], buf[3]);

				parsedSize = readSum;
			
				close(tcpFd);
				tcpFd = -1;
				result = RESULT_ERROR;
				tcpState= TCPSTATE_NOTCONNECTED;
				break; // error
			}

			if( (parsedSize + packetTotalBytes ) > readSum ) {
				break; // need to receive more
			} else {
				void* pTemp = buf + parsedSize;
				// parsing... bytes -->  message
				CPIPacket packet;
				packet.support.unsignedType = false;
				packet.support.bigEndian = support.bigEndian;
				parsedSize += packet.deserialize((char*)pTemp);

				support.save( "recv", tcpFd, pTemp, packet.size());

				// add message to listRecv
				pthread_mutex_lock( &mutexRecv );
				listRecv.push_back( packet);
				pthread_mutex_unlock( &mutexRecv );
			}
		}

		remainedRecvBufSize = readSum - parsedSize;
		if( 0 == remainedRecvBufSize ) {
			break;
		}
		else {
			memcpy((void*)tcpRecvBuffer, tcpRecvBuffer + parsedSize, remainedRecvBufSize );
		}
	}
	
	//DEBUG_LOG1( "end");
	return result;
}

bool CPICommunicator::initializeTCPClient(void) {

	if(  0 == ipString.size() ) {
		ERROR_LOG1( "Failed to create stream socket - invalid ip" );
		return false;
	}

	if((tcpFd= socket(AF_INET, SOCK_STREAM, 0))<0) {
		DEBUG_LOG( "Failed to create stream socket - %s", strerror(errno) );
		return false;
	}
	memset(&sockAddr, 0x00, sizeof(sockAddr));
	sockAddr.sin_family= AF_INET;
	sockAddr.sin_addr.s_addr= inet_addr( ipString.c_str() );
	sockAddr.sin_port= htons( tcpPort );
	return true;
}

bool CPICommunicator::terminateTCPClient(void) {
	return disconnectTCP();
}

CPICommunicator::COMM_RESULT_T CPICommunicator::connectTCP(void) {
	switch( connect( tcpFd, (struct sockaddr*)&sockAddr, sizeof(sockAddr))) {
		case -1 :
			ERROR_LOG( "Unable to connect to server: %s", strerror(errno));
			return RESULT_ERROR;
			break;
		default:
			break;
	}
	state = STATE_ACTIVE;
	tcpState = TCPSTATE_CONNECTED;
	return RESULT_OK;
}

CPICommunicator::COMM_RESULT_T CPICommunicator::disconnectTCP(void) {
	if( 0 < tcpFd ) {
		if( close(tcpFd) == -1 ) {
			//ERROR_LOG( "Unable to disconnect from server: %s", strerror(errno));
		}
		tcpFd = -1;
	}
	state = STATE_INACTIVE;
	tcpState= TCPSTATE_NOTCONNECTED;
	
	return RESULT_OK;
}


CPICommunicator::COMM_TCPSTATE_T CPICommunicator::selectTCP(void) {
	//DEBUG_LOG1( "begin");

	if( TCPSTATE_NOTCONNECTED == tcpState ) {
		//DEBUG_LOG( "fd(%d)- stop - tcpState(%d)", tcpFd, tcpState);
		return tcpState;
	}

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	fd_set readfds;
	FD_ZERO( &readfds );
	FD_SET( tcpFd, &readfds );
	long result = select( (tcpFd)+1, &readfds, (fd_set*)0, (fd_set*)0, &tv );
	switch( result ) {
		case 0: 
			//DEBUG_LOG1( "select - timeout");
			tcpState = TCPSTATE_CONNECTED;
			break;
		case -1:
			//DEBUG_LOG( "fd(%d)- error", tcpFd);
			state = STATE_INACTIVE;
			tcpState = TCPSTATE_NOTCONNECTED;
			break;
		default:
			//DEBUG_LOG( "fd(%d)- on_recv", tcpFd);
			tcpState = TCPSTATE_ONRECV;
			break;
	}

	//DEBUG_LOG( "tcp_state(%d)", tcpState);
	//DEBUG_LOG1( "end");
	return tcpState;
}
#endif
