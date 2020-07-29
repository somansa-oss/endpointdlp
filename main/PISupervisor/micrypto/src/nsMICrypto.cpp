// nsMICrypto.cpp: implementation of the nsMICrypto class.
//
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "nsMICrypto.h"

namespace nsMICrypto
{
	void BinToHex(unsigned char *buf, int buflen, std::string &ssHex)
	{
		for(int i = 0; i < buflen; i++)
		{
			char szTemp[3]; sprintf(szTemp,"%X%X", buf[i] >> 4, buf[i] & 0x0f);
			ssHex += szTemp;
		}
	}

	void CRC4::Init()
	{
		unsigned char szKey[] = {'S', 'O', 'M', 'A', 'N', 'S', 'A', ' ', 'C', 'S', ' ', 'K', 'E', 'Y'};
		prepare_key(szKey, sizeof(szKey), &key);
	}

	void CRC4::EnDecrypt(unsigned char *buffer_ptr,int buffer_len)
	{
		rc4(buffer_ptr, buffer_len, &key);
	}
}
