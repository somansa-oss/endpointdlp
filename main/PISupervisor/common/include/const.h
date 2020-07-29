#ifndef	_CONST_H
#define	_CONST_H

extern "C" {

#ifndef	NULL
#define	NULL	0x00
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif 


/* Fixed string */
#define	STRING_STDOUT	"stdout"
#define	STRING_STDERR	"stderr"
#define	STRING_STDIN	"stdin"
#define	STRING_OUTPUT	"output"
#define	STRING_REFFILE	"reference"

#define	CHAR_NV_DELIMITER	'='
#define	CHAR_PAIR_DELIMITER	','
/* Platform specific */
#ifdef	__WIN32
#define	CHAR_DIR_DELIMITER	'\\'
#else
#define	CHAR_DIR_DELIMITER	'/'
#endif

#ifndef INT_MAX
#	define INT_MAX	2147483647
#endif

#ifndef LONG_MAX
#  if __WORDSIZE == 64
#   define LONG_MAX	9223372036854775807L
#  else
#   define LONG_MAX	2147483647L
#  endif
#endif

#ifndef LLONG_MAX
#   define LLONG_MAX	9223372036854775807LL
#endif

};

#endif
