#ifndef _KERNELDATATYPE_H
#define _KERNELDATATYPE_H

#ifdef LINUX
#include <linux/types.h>
#include <wchar.h>
#include <stdint.h>
#else
#include <sys/_types.h>
#include <sys/_types/_wchar_t.h>
#include <mach/boolean.h>
#endif

#ifdef LINUX

#ifndef int16_t
typedef __int16_t       int16_t;
typedef __int32_t       int32_t;
typedef int             boolean_t;

typedef __uint32_t      uint32_t;
typedef __uint64_t      uint64_t;

typedef int             kern_return_t;

#ifndef FALSE
#define FALSE           0
#define TRUE            1
#endif

#define false           0
#define true            1

#endif

#endif

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

typedef __int64_t      LARGE_INTEGER;

typedef int             Boolean;

#define __in 
#define __out

#endif /* _KERNELDATATYPE_H */
