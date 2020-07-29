#ifndef _KERNELDATATYPE_H
#define _KERNELDATATYPE_H

#include <stdint.h>

typedef char            CHAR;
typedef unsigned char   UCHAR;
typedef wchar_t         WCHAR;

typedef char*           PCHAR;
typedef unsigned char*  PUCHAR;
typedef wchar_t*        PWCHAR;

typedef uint32_t        ULONG;
typedef uint32_t*       PULONG;
typedef unsigned int    mode_t;

//typedef __uint32_t      ULONG;
//typedef __uint32_t*     PULONG;

typedef  unsigned long  BOOLEAN;

typedef void *          PVOID;

typedef __int64_t      LARGE_INTEGER;

#ifndef boolean_t
#ifdef __X86_64
#define boolean_t unsigned int
#else
#define boolean_t int
#endif
#endif 

#define __in 
#define __out

#ifndef vnode_t
#define vnode_t void*
#endif

#endif /* _KERNELDATATYPE_H */
