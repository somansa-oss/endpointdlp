//
//  DataType.h
//  PISecSmartDrv
//
//  Created by IA1 on 2/22/16.
//  Copyright © 2016 Somansa. All rights reserved.
//

#include <sys/_types.h>
#include <sys/_types/_wchar_t.h>

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

#define __in 
#define __out




#endif /* DataType_h */
