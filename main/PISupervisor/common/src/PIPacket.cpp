
#ifndef	_PIPACKET_CPP
#define	_PIPACKET_CPP

#ifdef LINUX
#include <string>
#endif

#include "PIPacket.h"
#include <arpa/inet.h>

////////////////////////////////////////////////////////////////////////////////
//CPIPacketSupport
CPIPacketSupport::CPIPacketSupport(void) {
	clear();
}

CPIPacketSupport::~CPIPacketSupport(void) {
}

void CPIPacketSupport::clear(void) {
	unsignedType = true;
	bigEndian = false;
}

bool CPIPacketSupport::isHostBigEndian(void) const{

	typedef enum _ENDIANESS { NATIVE = 0, BIG, LITTLE } ENDIANESS;
	static ENDIANESS hostEndianess = NATIVE;

	if( NATIVE == hostEndianess ) {
		int testInt = 1;
		unsigned char* testByte = (unsigned char*)& testInt;
		hostEndianess = (1 == *testByte) ? LITTLE : BIG;
	}

	return (BIG == hostEndianess) ? true : false;
}

unsigned long long CPIPacketSupport::hton_int64(const unsigned long long param) const {
	return ntoh_int64(param);
}

unsigned long long CPIPacketSupport::ntoh_int64(const unsigned long long param) const {

	if( true == isHostBigEndian() ) {
		return param;
	}

	return (((uint64_t)(ntohl((uint32_t)((param<<32)>>32)))<<32)|ntohl(((uint32_t)(param>>32))));
}

unsigned int CPIPacketSupport::hton(const unsigned int param) const {

	if( false == bigEndian ) {
		return param;
	}

	return ( false == bigEndian ) ? param : htonl((unsigned int) param);
}

unsigned long long CPIPacketSupport::hton(const unsigned long long param) const {

	if( false == bigEndian ) {
		return param;
	}

	return ( false == bigEndian ) ? param : hton_int64((unsigned long long) param);
}

unsigned int CPIPacketSupport::ntoh(const unsigned int param) const {
	return ( false == bigEndian ) ? param : ntohl(param);
}

unsigned long long CPIPacketSupport::ntoh(const unsigned long long param) const {
	return ( false == bigEndian ) ? param : ntoh_int64(param);
}

////////////////////////////////////////////////////////////////////////////////
//CPIPacketValue
CPIPacketValue::CPIPacketValue(void) {
	size = 0;
}

CPIPacketValue::~CPIPacketValue(void) {
}

void CPIPacketValue::setBinary(const char * binary) {
	if( NULL == binary )
		return;

	value.clear();
	value.reserve(size);
	for(int index = 0; index < size; index++)
	{
		value.push_back(binary[index]);
	}
}

void CPIPacketValue::setBinary(const char * binary, const unsigned int size) {
	if( NULL == binary )
		return;

	this->size = size;

	this->setBinary(binary);
}

////////////////////////////////////////////////////////////////////////////////:
//CPIPacket
CPIPacket::CPIPacket(void) {
	totalSize = 0;
	processId = 0L;
	command = 0;
	paramCount = 0;
	data = NULL;
}

CPIPacket::~CPIPacket(void) {
	if( NULL != data )
	{
		delete data;
		data = NULL;
	}
}

int CPIPacket::getHeaderSize(void) const {
	int result = 0;

	result += sizeof(int); // totoal bytes
	result += sizeof(unsigned int); // pid (reserved)
	result += sizeof(int); // reserved

	return result;
}

int CPIPacket::getValueSize(void) const {
	int result = 0;

	result += sizeof(int); // command
	result += sizeof(int); // param count
	result += sizeof(int) * vectorValue.size(); // param length...

	// param
	CPIPacketValue::VECTOR::const_iterator itr = vectorValue.begin();
	for( ; itr != vectorValue.end(); ++itr) {
		result += itr->size;
	}

	return result;
}

void CPIPacket::setCommand(const int command) {
	this->command = command;
}

int CPIPacket::addBinary(const void * param, const unsigned int size) {
	if( NULL == param)
	{
		return -1;
	}

	CPIPacketValue value;
	value.setBinary((char*) param, size);
	vectorValue.push_back(value);

	return (int)(vectorValue.size()-(size_t)1);
}

int CPIPacket::add_int(const int param) {
	int checkedParam = (int) support.hton((const unsigned int)param);
	return this->addBinary(&checkedParam, sizeof(int));
}

int CPIPacket::add_uint(const unsigned int param) {

	if( false == support.unsignedType ) {
		return add_int(util.getInt(param));
	}
	
	unsigned int checkedParam = support.hton(param);
	return this->addBinary(&checkedParam, sizeof(unsigned int));
}

int CPIPacket::add_longlong(const long long param) {
	long long checkedParam = (long long) support.hton((const unsigned long long) param);
	return this->addBinary(&checkedParam, sizeof(long long));
}

int CPIPacket::add_ulonglong(const unsigned long long param) {

	if( false == support.unsignedType ) {
		return add_longlong(util.getLongLong(param));
	}
	
	unsigned long long checkedParam = support.hton(param);
	return this->addBinary(&checkedParam, sizeof(unsigned long long));
}

int CPIPacket::add_string(const std::string param) {
	const size_t size = (param.length()+1)* sizeof(char);
	return this->addBinary(param.c_str(), (const int)size);
}

int CPIPacket::getCommand(void) const {
	return command;
}

const char * CPIPacket::getBinary(const unsigned int index) const {
	if( vectorValue.size() <= index )
		return 0;

	return vectorValue[index].value.data();
}

int CPIPacket::get_int(const unsigned int index) const {
	const char * param = this->getBinary(index);

	if( NULL == param )
		return 0;

	int result = 0;
	memcpy(&result, param, sizeof(int));

	return (int) support.ntoh((const unsigned int) result);
}

unsigned int CPIPacket::get_uint(const unsigned int index) const {
	const char * param = this->getBinary(index);

	if( NULL == param )
		return 0;

	unsigned int result = 0;
	memcpy(&result, param, sizeof(unsigned int));
	
	return support.ntoh(result);
}

long long CPIPacket::get_longlong(const unsigned int index) const {
	const char * param = this->getBinary(index);

	if( NULL == param )
		return 0;

	long long result = 0;
	memcpy(&result, param, sizeof(long long));
	
	return (long long) support.ntoh((const unsigned long long) result);
}

unsigned long long CPIPacket::get_ulonglong(const unsigned int index) const {
	const char * param = this->getBinary(index);

	if( NULL == param )
		return 0;

	unsigned long long result = 0;
	memcpy(&result, param, sizeof(unsigned long long));
	
	return support.ntoh(result);
}

std::string CPIPacket::get_string(const unsigned int index) const {
	const char * param = this->getBinary(index);

	if( NULL == param )
		return 0;

	return std::string((const char*) param);
}

int CPIPacket::size(void) const {
	return totalSize;
}

int CPIPacket::deserialize(char * data) {
	
	if( NULL == data)
		return 0;

	vectorValue.clear();

	char * itr = data;

	// totalSize bytes
	memcpy(&totalSize, itr, sizeof(int));
	totalSize = (int) support.ntoh((const unsigned int) totalSize);
	itr += sizeof(int);

	// pid
	memcpy(&processId, itr, sizeof(unsigned int));
	processId = (int) support.ntoh((const unsigned int) processId);
	itr += sizeof(unsigned int);
	
	// reserved
	itr += sizeof(int);

	// command
	memcpy(&command, itr, sizeof(int));
	command	= (int) support.ntoh((const unsigned int) command);
	itr += sizeof(int);

	// param count;
	memcpy(&paramCount, itr, sizeof(int));
	paramCount = (int) support.ntoh((const unsigned int) paramCount);
	itr += sizeof(int);

	// param length
	for(int index = 0; index < paramCount; index++) {
		CPIPacketValue value;
		memcpy(&value.size, itr, sizeof(int));
		value.size = (int) support.ntoh((const unsigned int) value.size);
		itr += sizeof(int);

		vectorValue.push_back(value);
	}

	// param
	for(int index = 0; index < paramCount; index++) {
		CPIPacketValue & value = vectorValue[index];
		value.setBinary(itr);
		itr += value.size;
	}

	return totalSize;
}

char * CPIPacket::serialize(void) {

	const int paramCount = (int)vectorValue.size();
	const int headerSize = getHeaderSize();
	const int valueSize = getValueSize();
	totalSize = headerSize + valueSize;
	int temp = 0;

	data = new char[totalSize];
	memset(data, 0x00, totalSize);
	char * itr = data;

	// totalSize bytes
	temp = (int) support.hton((const unsigned int) totalSize);
	memcpy(itr, &temp, sizeof(int));
	itr += sizeof(int);

	// pid
	processId = 0; // ::getCurrentProcessId(); // (Win32)
	memcpy(itr, &processId, sizeof(unsigned int));
	itr += sizeof(unsigned int);

	// reserved
	itr += sizeof(int);

	// command
	temp = (int) support.hton((const unsigned int) command);
	memcpy(itr, &temp, sizeof(int));
	itr += sizeof(int);

	// param count;
	temp = (int) support.hton((const unsigned int) paramCount);
	memcpy(itr, &temp, sizeof(int));
	itr += sizeof(int);

	// param length
	for(int index = 0; index < paramCount; index++) {
		const CPIPacketValue & value = vectorValue[index];
		temp = (int) support.hton((const unsigned int) value.size);
		memcpy(itr, &temp, sizeof(int));
		itr += sizeof(value.size);
	}

	// param
	for(int index = 0; index < paramCount; index++) {
		const CPIPacketValue & value = vectorValue[index];
		memcpy(itr, value.value.data(), value.size);
		itr += value.size;
	}

	return data;
}

unsigned int CPIPacket::getProcessId(void) const {
	return processId;
}

int CPIPacket::getParamCount(void) const {
	return paramCount;
}

char* CPIPacket::getData(void) const {
	return this->data;
}
#endif
