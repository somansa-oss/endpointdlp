#pragma once

#include <wchar.h>
#include <string>
#include <vector>
//#include <Windows.h>

////////////////////////////////////////////////////////////////////////////////
//CIPCMessageValue
class CIPCMessageValue
{
public:
	CIPCMessageValue(void)
	{
		size = 0;
	}

	~CIPCMessageValue(void)
	{
	}

public:
	typedef std::vector<unsigned char> Value;

public:
	int size;
	CIPCMessageValue::Value value;

public:
	void SetBinary(const unsigned char * pBinary)
	{
		if( NULL == pBinary )
			return;

		value.clear();
		value.reserve(size);
		for(int iIndex = 0; iIndex < size; iIndex++)
		{
			value.push_back(pBinary[iIndex]);
		}
	};

	void SetBinary(const unsigned char * pBinary, const unsigned int cbSize)
	{
		if( NULL == pBinary )
			return;

		size = cbSize;

		this->SetBinary(pBinary);
	};
};

////////////////////////////////////////////////////////////////////////////////:
//CIPCMessage
class CIPCMessage
{
public:
	CIPCMessage(void)
	{
		m_cbTotal = 0;
		m_dwProcessId = 0L;
		m_nCommand = 0;
		m_nParamCount = 0;
		m_pData = NULL;
	};

	~CIPCMessage(void)
	{
		if( NULL != m_pData )
		{
			delete m_pData;
			m_pData = NULL;
		}
	};

private:
	typedef std::vector<CIPCMessageValue> Vector;

private:
	int m_cbTotal;
	DWORD m_dwProcessId;
	int m_nCommand;
	int m_nParamCount;
	Vector m_vectorValue;
	unsigned char * m_pData;

private:
	size_t GetHeaderSize(void) const
	{
		size_t nResult = 0;

		nResult += sizeof(int); // totoal bytes
		nResult += sizeof(DWORD); // pid
		nResult += sizeof(int); // reserved

		return nResult;
	};

	size_t GetValueSize(void) const
	{
		size_t nResult = 0;

		nResult += sizeof(int); // command
		nResult += sizeof(int); // param count
		nResult += sizeof(int) * m_vectorValue.size(); // param length...

		// param
		Vector::const_iterator itr = m_vectorValue.begin();
		while(itr != m_vectorValue.end() )
		{
			nResult += itr->size;

			itr++;
		}

		return nResult;
	};

public:
	void SetCommand(const int nCommand)
	{
		m_nCommand = nCommand;
	};

	int AddBinary(const void * pParam, const size_t cbParam)
	{
		if( NULL == pParam)
		{
			return -1;
		}

		CIPCMessageValue cValue;
		cValue.SetBinary((unsigned char *)pParam, cbParam);
		m_vectorValue.push_back(cValue);

		return m_vectorValue.size()-1;
	};

public:
	int AddINT(const int * pParam)
	{
		return this->AddLONG((const long*) pParam);
	};

	int AddUINT(const unsigned int * pParam)
	{
		return this->AddULONG((const unsigned long *) pParam);
	};

	int AddLONG(const long * pParam)
	{
		return this->AddBinary(pParam, sizeof(long));
	};

	int AddULONG(const unsigned long * pParam)
	{
		return this->AddBinary(pParam, sizeof(unsigned long));
	};

	int AddLONGLONG(const __int64 * pParam)
	{
		return this->AddBinary(pParam, sizeof(__int64));
	};

	int AddULONGLONG(const unsigned __int64 * pParam)
	{
		return this->AddBinary(pParam, sizeof(unsigned __int64));
	};

	int AddCHAR(const char * pszParam, const size_t cbSize)
	{
		return this->CbAddCHAR(pszParam, cbSize); 
	};

	int AddWCHAR(const wchar_t * pwszParam, const size_t cbSize)
	{
		return this->CbAddWCHAR(pwszParam, cbSize); 
	};
	
	int CbAddCHAR(const char * pszParam, const size_t cbSize)
	{
		return this->AddBinary(pszParam, cbSize); 
	};

	int CbAddWCHAR(const wchar_t * pwszParam, const size_t cbSize)
	{
		return this->AddBinary(pwszParam, cbSize); 
	};

	int CchAddCHAR(const char * pszParam, const size_t cchSize)
	{
		const size_t cbSize = cchSize * sizeof(char);
		return this->AddBinary(pszParam, cbSize); 
	};

	int CchAddWCHAR(const wchar_t * pwszParam, const size_t cchSize)
	{
		const size_t cbSize = cchSize * sizeof(wchar_t);
		return this->AddBinary(pwszParam, cbSize); 
	};

public:
	int GetCommand(void) const
	{
		return m_nCommand;
	};

	const unsigned char * GetBinary(const unsigned int nIndex) const
	{
		if( m_vectorValue.size() <= (size_t) nIndex )
			return 0;

		return m_vectorValue[nIndex].value.data();
	};

public:
	int GetINT(const unsigned int nIndex) const
	{
		return (int) GetLONG(nIndex);
	};

	unsigned int GetUINT(const unsigned int nIndex) const
	{
		return (unsigned int) GetULONG(nIndex);
	};

	long GetLONG(const unsigned int nIndex) const
	{
		const unsigned char * pParam = this->GetBinary(nIndex);

		if( NULL == pParam )
			return 0;

		long nResult = 0;
		memcpy(&nResult, pParam, sizeof(long));

		return nResult;
	};

	unsigned long GetULONG(const unsigned int nIndex) const
	{
		const unsigned char * pParam = this->GetBinary(nIndex);

		if( NULL == pParam )
			return 0;

		unsigned long nResult = 0;
		memcpy(&nResult, pParam, sizeof(unsigned long));

		return nResult;
	};
	__int64 GetLONGLONG(const unsigned int nIndex) const
	{
		const unsigned char * pParam = this->GetBinary(nIndex);

		if( NULL == pParam )
			return 0;

		__int64 nResult = 0;
		memcpy(&nResult, pParam, sizeof(__int64));

		return nResult;
	};

	unsigned __int64 GetULONGLONG(const unsigned int nIndex) const
	{
		const unsigned char * pParam = this->GetBinary(nIndex);

		if( NULL == pParam )
			return 0;

		unsigned __int64 nResult = 0;
		memcpy(&nResult, pParam, sizeof(unsigned __int64));

		return nResult;
	};

	char* GetCHAR(const unsigned int nIndex) const
	{
		const unsigned char * pParam = this->GetBinary(nIndex);

		if( NULL == pParam )
			return 0;

		return (char*) pParam;
	};

	wchar_t* GetWCHAR(const unsigned int nIndex) const
	{
		const unsigned char * pParam = this->GetBinary(nIndex);

		if( NULL == pParam )
			return 0;

		return (wchar_t*) pParam;
	};

public:
	int GetSize(void) const
	{
		return m_cbTotal;
	};

	void SetData(unsigned char * pData)
	{
		if( NULL == pData)
			return;

		m_vectorValue.clear();

		unsigned char * pIter = pData;

		// total bytes
		memcpy(&m_cbTotal, pIter, sizeof(int));
		pIter += sizeof(int);

		// pid
		memcpy(&m_dwProcessId, pIter, sizeof(DWORD));
		pIter += sizeof(DWORD);

		// reserved
		pIter += sizeof(int);

		// command
		memcpy(&m_nCommand, pIter, sizeof(int));
		pIter += sizeof(int);

		// param count;
		memcpy(&m_nParamCount, pIter, sizeof(int));
		pIter += sizeof(int);

		// param length
		for(int iIndex = 0; iIndex < m_nParamCount; iIndex++)
		{
			CIPCMessageValue cValue;
			memcpy(&cValue.size, pIter, sizeof(int));
			pIter += sizeof(int);

			m_vectorValue.push_back(cValue);
		}

		// param
		for(int iIndex = 0; iIndex < m_nParamCount; iIndex++)
		{
			CIPCMessageValue & cValue = m_vectorValue[iIndex];
			cValue.SetBinary(pIter);
			pIter += cValue.size;
		}
	};

	unsigned char * GetData(void)
	{
		const int nParamCount = m_vectorValue.size();
		const int cbHeaderSize = GetHeaderSize();
		const int cbValueSize = GetValueSize();
		m_cbTotal = cbHeaderSize + cbValueSize;

		m_pData = new unsigned char[m_cbTotal];
		memset(m_pData, 0x00, m_cbTotal);
		unsigned char * pIter = m_pData;

		// total bytes
		memcpy(pIter, &m_cbTotal, sizeof(int));
		pIter += sizeof(int);

		// pid
		m_dwProcessId = GetCurrentProcessId();
		memcpy(pIter, &m_dwProcessId, sizeof(DWORD));
		pIter += sizeof(DWORD);

		// reserved
		pIter += sizeof(int);

		// command
		memcpy(pIter, &m_nCommand, sizeof(int));
		pIter += sizeof(int);

		// param count;
		memcpy(pIter, &nParamCount, sizeof(int));
		pIter += sizeof(int);

		// param length
		for(int iIndex = 0; iIndex < nParamCount; iIndex++)
		{
			const CIPCMessageValue & cValue = m_vectorValue[iIndex];
			memcpy(pIter, &cValue.size, sizeof(int));
			pIter += sizeof(cValue.size);
		}

		// param
		for(int iIndex = 0; iIndex < nParamCount; iIndex++)
		{
			const CIPCMessageValue & cValue = m_vectorValue[iIndex];
			memcpy(pIter, cValue.value.data(), cValue.size);
			pIter += cValue.size;
		}

		return m_pData;
	};
	
	DWORD GetProcessId(void) const
	{
		return m_dwProcessId;
	}
};