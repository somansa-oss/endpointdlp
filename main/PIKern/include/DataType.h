//
//  DataType.h
//  PISecSmartDrv
//
//  Created by IA1 on 2/22/16.
//  Copyright © 2016 Somansa. All rights reserved.
//

#ifdef LINUX
#else
#include <sys/_types.h>
#include <sys/_types/_wchar_t.h>
#endif


#ifndef DataType_h
#define DataType_h

typedef char            CHAR;
typedef unsigned char   UCHAR;
typedef wchar_t         WCHAR;
typedef char*           PCHAR;
typedef unsigned char*  PUCHAR;
typedef wchar_t*        PWCHAR;
typedef __uint32_t      ULONG;
typedef __uint32_t*     PULONG;
typedef  unsigned long  BOOLEAN;
typedef void *          PVOID;
typedef __int64_t       LARGE_INTEGER;


enum vtype      {
	/* 0 */
	VNON,
	/* 1 - 5 */
	VREG, VDIR, VBLK, VCHR, VLNK,
	/* 6 - 10 */
	VSOCK, VFIFO, VBAD, VSTR, VCPLX
};


#define PROC_PIDPATHINFO_MAXSIZE    1024 * 4

#define __in 
#define __out




#endif /* DataType_h */
