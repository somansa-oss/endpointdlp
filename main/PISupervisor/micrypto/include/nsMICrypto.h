// nsMICrypto.h: interface for the nsMICrypto class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _NSMICRYPTO_H_
#define _NSMICRYPTO_H_

#include <string>
#include <string.h>
#include "rc4.h"

namespace nsMICrypto
{

    void BinToHex(unsigned char *buf, int buflen, std::string &ssHex);

	//for RC4 encryption/descryption
	class CRC4
	{
	public:
		rc4_key key;
		void Init();
		void EnDecrypt(unsigned char *buffer_ptr,int buffer_len);
	};
}

#endif
