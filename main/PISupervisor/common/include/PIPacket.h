#ifndef	_PIPACKET_H
#define	_PIPACKET_H

#include "include_system.h"
#include "include_commonlib.h"

//#include "Protocol.h"
#include "PIObject.h"
#include <vector>
#include <string>

////////////////////////////////////////////////////////////////////////////////
//CPIPacketSupport
class CPIPacketSupport
{
public:
	CPIPacketSupport(void);
	virtual ~CPIPacketSupport(void);

public:
	bool unsignedType;
	bool bigEndian;

public:
	void clear(void);
	bool isHostBigEndian(void) const;
	unsigned long long hton_int64(const unsigned long long param) const;
	unsigned long long ntoh_int64(const unsigned long long param) const;

	unsigned int hton(const unsigned int param)const;
	unsigned long long hton(const unsigned long long param)const;
	unsigned int ntoh(const unsigned int param)const;
	unsigned long long ntoh(const unsigned long long param)const;
};

////////////////////////////////////////////////////////////////////////////////
//CPIPacketValue
class CPIPacketValue
{
public:
	CPIPacketValue(void);
	virtual ~CPIPacketValue(void);

public:
	typedef std::vector<char> VECTOR_BYTES;
	typedef std::vector<CPIPacketValue> VECTOR;

public:
	int size;
	CPIPacketValue::VECTOR_BYTES value;

public:
	void setBinary(const char * binary);
	void setBinary(const char * binary, const unsigned int size);
};

////////////////////////////////////////////////////////////////////////////////:
//CPIPacket
class CPIPacket : public CPIObject
{
public:
	CPIPacket(void);
	virtual ~CPIPacket(void);

private:
	int totalSize;
	unsigned int processId;
	int command;
	int paramCount;
	CPIPacketValue::VECTOR vectorValue;
	char * data;

public:
	CPIPacketSupport support;

private:
	int getHeaderSize(void) const;
	int getValueSize(void) const;

public:
	void setCommand(const int command);
	int addBinary(const void * param, const unsigned int size);

public:
	int add_int(const int param);
	int add_uint(const unsigned int param);
	int add_longlong(const long long param);
	int add_ulonglong(const unsigned long long param);
	int add_string(const std::string param);

public:
	int getCommand(void) const;
	const char * getBinary(const unsigned int index) const;

public:
	int get_int(const unsigned int index) const;
	unsigned int get_uint(const unsigned int index) const;
	long long get_longlong(const unsigned int index) const;
	unsigned long long get_ulonglong(const unsigned int index) const;
	std::string get_string(const unsigned int index) const;

public:
	int size(void) const;
	int deserialize(char * data);
	char * serialize(void);
	unsigned int getProcessId(void) const;
	int getParamCount(void) const;
	char* getData(void) const;
};
#endif
